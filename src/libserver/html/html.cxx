/*-
 * Copyright 2016 Vsevolod Stakhov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#include "util.h"
#include "rspamd.h"
#include "message.h"
#include "html.h"
#include "html_tags.h"
#include "libserver/css/css_value.hxx"

#include "url.h"
#include "contrib/libucl/khash.h"
#include "libmime/images.h"
#include "css/css.h"
#include "libutil/cxx/utf8_util.h"

#include "html_tag_defs.hxx"
#include "html_entities.hxx"
#include "html_tag.hxx"

#include <vector>
#include <frozen/unordered_map.h>
#include <frozen/string.h>

#include <unicode/uversion.h>
#include <unicode/ucnv.h>
#if U_ICU_VERSION_MAJOR_NUM >= 46
#include <unicode/uidna.h>
#endif

namespace rspamd::html {

static const guint max_tags = 8192; /* Ignore tags if this maximum is reached */

static const html_tags_storage html_tags_defs;

auto html_components_map = frozen::make_unordered_map<frozen::string, html_component_type>(
		{
				{"name", html_component_type::RSPAMD_HTML_COMPONENT_NAME},
				{"href", html_component_type::RSPAMD_HTML_COMPONENT_HREF},
				{"src", html_component_type::RSPAMD_HTML_COMPONENT_HREF},
				{"action", html_component_type::RSPAMD_HTML_COMPONENT_HREF},
				{"color", html_component_type::RSPAMD_HTML_COMPONENT_COLOR},
				{"bgcolor", html_component_type::RSPAMD_HTML_COMPONENT_BGCOLOR},
				{"style", html_component_type::RSPAMD_HTML_COMPONENT_STYLE},
				{"class", html_component_type::RSPAMD_HTML_COMPONENT_CLASS},
				{"width", html_component_type::RSPAMD_HTML_COMPONENT_WIDTH},
				{"height", html_component_type::RSPAMD_HTML_COMPONENT_HEIGHT},
				{"size", html_component_type::RSPAMD_HTML_COMPONENT_SIZE},
				{"rel", html_component_type::RSPAMD_HTML_COMPONENT_REL},
				{"alt", html_component_type::RSPAMD_HTML_COMPONENT_ALT},
		});

#define msg_debug_html(...)  rspamd_conditional_debug_fast (NULL, NULL, \
        rspamd_html_log_id, "html", pool->tag.uid, \
        G_STRFUNC, \
        __VA_ARGS__)

INIT_LOG_MODULE(html)

static gboolean
rspamd_html_check_balance(GNode *node, GNode **cur_level)
{
	struct html_tag *arg = (struct html_tag *)node->data, *tmp;
	GNode *cur;

	if (arg->flags & FL_CLOSING) {
		/* First of all check whether this tag is closing tag for parent node */
		cur = node->parent;
		while (cur && cur->data) {
			tmp = (struct html_tag *)cur->data;
			if (tmp->id == arg->id &&
				(tmp->flags & FL_CLOSED) == 0) {
				tmp->flags |= FL_CLOSED;
				/* Destroy current node as we find corresponding parent node */
				g_node_destroy(node);
				/* Change level */
				*cur_level = cur->parent;
				return TRUE;
			}
			cur = cur->parent;
		}
	}
	else {
		return TRUE;
	}

	return FALSE;
}

static gboolean
rspamd_html_process_tag(rspamd_mempool_t *pool,
						struct html_content *hc,
						struct html_tag *tag,
						GNode **cur_level,
						gboolean *balanced)
{
	GNode *nnode;
	struct html_tag *parent;

	if (hc->html_tags == NULL) {
		nnode = g_node_new(NULL);
		*cur_level = nnode;
		hc->html_tags = nnode;
		rspamd_mempool_add_destructor (pool,
				(rspamd_mempool_destruct_t) g_node_destroy,
				nnode);
	}

	if (hc->total_tags > rspamd::html::max_tags) {
		hc->flags |= RSPAMD_HTML_FLAG_TOO_MANY_TAGS;
	}

	if (tag->id == -1) {
		/* Ignore unknown tags */
		hc->total_tags++;
		return FALSE;
	}

	tag->parent = *cur_level;

	if (!(tag->flags & (CM_INLINE | CM_EMPTY))) {
		/* Block tag */
		if (tag->flags & (FL_CLOSING | FL_CLOSED)) {
			if (!*cur_level) {
				msg_debug_html ("bad parent node");
				return FALSE;
			}

			if (hc->total_tags < rspamd::html::max_tags) {
				nnode = g_node_new(tag);
				g_node_append (*cur_level, nnode);

				if (!rspamd_html_check_balance(nnode, cur_level)) {
					msg_debug_html (
							"mark part as unbalanced as it has not pairable closing tags");
					hc->flags |= RSPAMD_HTML_FLAG_UNBALANCED;
					*balanced = FALSE;
				}
				else {
					*balanced = TRUE;
				}

				hc->total_tags++;
			}
		}
		else {
			parent = (struct html_tag *)(*cur_level)->data;

			if (parent) {
				if ((parent->flags & FL_IGNORE)) {
					tag->flags |= FL_IGNORE;
				}

				if (!(tag->flags & FL_CLOSED) &&
					!(parent->flags & FL_BLOCK)) {
					/* We likely have some bad nesting */
					if (parent->id == tag->id) {
						/* Something like <a>bla<a>foo... */
						hc->flags |= RSPAMD_HTML_FLAG_UNBALANCED;
						*balanced = FALSE;
						tag->parent = parent->parent;

						if (hc->total_tags < rspamd::html::max_tags) {
							nnode = g_node_new(tag);
							g_node_append (parent->parent, nnode);
							*cur_level = nnode;
							hc->total_tags++;
						}

						return TRUE;
					}
				}
			}

			if (hc->total_tags < rspamd::html::max_tags) {
				nnode = g_node_new(tag);
				g_node_append (*cur_level, nnode);

				if ((tag->flags & FL_CLOSED) == 0) {
					*cur_level = nnode;
				}

				hc->total_tags++;
			}

			if (tag->flags & (CM_HEAD | CM_UNKNOWN | FL_IGNORE)) {
				tag->flags |= FL_IGNORE;

				return FALSE;
			}

		}
	}
	else {
		/* Inline tag */
		parent = (struct html_tag *)(*cur_level)->data;

		if (parent) {
			if (hc->total_tags < rspamd::html::max_tags) {
				nnode = g_node_new(tag);
				g_node_append (*cur_level, nnode);

				hc->total_tags++;
			}
			if ((parent->flags & (CM_HEAD | CM_UNKNOWN | FL_IGNORE))) {
				tag->flags |= FL_IGNORE;

				return FALSE;
			}
		}
	}

	return TRUE;
}


static auto
find_tag_component_name(rspamd_mempool_t *pool,
					const gchar *begin,
					const gchar *end) -> std::optional<html_component_type>
{
	if (end <= begin) {
		return std::nullopt;
	}

	auto *p = rspamd_mempool_alloc_buffer(pool, end - begin);
	memcpy(p, begin, end - begin);
	auto len = decode_html_entitles_inplace(p, end - begin);
	auto known_component_it = html_components_map.find({p, len});

	if (known_component_it != html_components_map.end()) {
		return known_component_it->second;
	}
	else {
		return std::nullopt;
	}
}

struct tag_content_parser_state {
	int cur_state = 0;
	const char *saved_p = nullptr;
	std::optional<html_component_type> cur_component;

	void reset()
	{
		cur_state = 0;
		saved_p = nullptr;
		cur_component = std::nullopt;
	}
};

static inline void
parse_tag_content(rspamd_mempool_t *pool,
				  struct html_content *hc,
				  struct html_tag *tag,
				  const char *in,
				  struct tag_content_parser_state parser_env)
{
	enum tag_parser_state {
		parse_start = 0,
		parse_name,
		parse_attr_name,
		parse_equal,
		parse_start_dquote,
		parse_dqvalue,
		parse_end_dquote,
		parse_start_squote,
		parse_sqvalue,
		parse_end_squote,
		parse_value,
		spaces_after_name,
		spaces_before_eq,
		spaces_after_eq,
		spaces_after_param,
		ignore_bad_tag
	} state;
	gboolean store = FALSE;

	state = static_cast<enum tag_parser_state>(parser_env.cur_state);

	/*
	 * Stores tag component if it doesn't exist, performing copy of the
	 * value + decoding of the entities
	 * Parser env is set to clear the current html attribute fields (saved_p and
	 * cur_component)
	 */
	auto store_tag_component = [&]() -> void {
		if (parser_env.saved_p != nullptr && parser_env.cur_component &&
			in > parser_env.saved_p) {

			/* We ignore repeated attributes */
			auto found_it = tag->parameters.find(parser_env.cur_component.value());

			if (found_it == tag->parameters.end()) {
				auto sz = (std::size_t)(in - parser_env.saved_p);
				auto *s = rspamd_mempool_alloc_buffer(pool, sz);
				memcpy(s, parser_env.saved_p, sz);
				sz = rspamd_html_decode_entitles_inplace(s, in - parser_env.saved_p);
				tag->parameters.emplace(parser_env.cur_component.value(),
						std::string_view{s, sz});
			}
		}

		parser_env.saved_p = nullptr;
		parser_env.cur_component = std::nullopt;
	};

	switch (state) {
	case parse_start:
		if (!g_ascii_isalpha (*in) && !g_ascii_isspace (*in)) {
			hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
			state = ignore_bad_tag;
			tag->id = -1;
			tag->flags |= FL_BROKEN;
		}
		else if (g_ascii_isalpha (*in)) {
			state = parse_name;
			tag->name = std::string_view{in, 0};
		}
		break;

	case parse_name:
		if (g_ascii_isspace (*in) || *in == '>' || *in == '/') {
			const auto *start = tag->name.begin();
			g_assert (in >= start);

			if (*in == '/') {
				tag->flags |= FL_CLOSED;
			}

			tag->name = std::string_view{start, (std::size_t)(in - start)};

			if (tag->name.empty()) {
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				tag->id = -1;
				tag->flags |= FL_BROKEN;
				state = ignore_bad_tag;
			}
			else {
				/*
				 * Copy tag name to the temporary buffer for modifications
				 */
				auto *s = rspamd_mempool_alloc_buffer(pool, tag->name.size() + 1);
				rspamd_strlcpy(s, tag->name.data(), tag->name.size());
				auto nsize = rspamd_html_decode_entitles_inplace(s,
						tag->name.size());
				nsize =  rspamd_str_lc_utf8(s, nsize);
				tag->name = std::string_view{s, nsize};

				const auto *tag_def = rspamd::html::html_tags_defs.by_name(tag->name);

				if (tag_def == nullptr) {
					hc->flags |= RSPAMD_HTML_FLAG_UNKNOWN_ELEMENTS;
					tag->id = -1;
				}
				else {
					tag->id = tag_def->id;
					tag->flags = tag_def->flags;
				}

				state = spaces_after_name;
			}
		}
		break;

	case parse_attr_name:
		if (parser_env.saved_p == nullptr) {
			state = ignore_bad_tag;
		}
		else {
			const auto *attr_name_end = in;

			if (*in == '=') {
				state = parse_equal;
			}
			else if (*in == '"') {
				/* No equal or something sane but we have quote character */
				state = parse_start_dquote;
				attr_name_end = in - 1;

				while (attr_name_end > parser_env.saved_p) {
					if (!g_ascii_isalnum (*attr_name_end)) {
						attr_name_end--;
					}
					else {
						break;
					}
				}

				/* One character forward to obtain length */
				attr_name_end++;
			}
			else if (g_ascii_isspace (*in)) {
				state = spaces_before_eq;
			}
			else if (*in == '/') {
				tag->flags |= FL_CLOSED;
			}
			else if (!g_ascii_isgraph (*in)) {
				state = parse_value;
				attr_name_end = in - 1;

				while (attr_name_end > parser_env.saved_p) {
					if (!g_ascii_isalnum (*attr_name_end)) {
						attr_name_end--;
					}
					else {
						break;
					}
				}

				/* One character forward to obtain length */
				attr_name_end++;
			}
			else {
				return;
			}

			parser_env.cur_component = find_tag_component_name(pool,
					parser_env.saved_p,
					attr_name_end);

			if (!parser_env.cur_component) {
				/* Ignore unknown params */
				parser_env.saved_p = nullptr;
			}
			else if (state == parse_value) {
				parser_env.saved_p = in + 1;
			}
		}

		break;

	case spaces_after_name:
		if (!g_ascii_isspace (*in)) {
			parser_env.saved_p = in;

			if (*in == '/') {
				tag->flags |= FL_CLOSED;
			}
			else if (*in != '>') {
				state = parse_attr_name;
			}
		}
		break;

	case spaces_before_eq:
		if (*in == '=') {
			state = parse_equal;
		}
		else if (!g_ascii_isspace (*in)) {
			/*
			 * HTML defines that crap could still be restored and
			 * calculated somehow... So we have to follow this stupid behaviour
			 */
			/*
			 * TODO: estimate what insane things do email clients in each case
			 */
			if (*in == '>') {
				/*
				 * Attribtute name followed by end of tag
				 * Should be okay (empty attribute). The rest is handled outside
				 * this automata.
				 */

			}
			else if (*in == '"' || *in == '\'') {
				/* Attribute followed by quote... Missing '=' ? Dunno, need to test */
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				tag->flags |= FL_BROKEN;
				state = ignore_bad_tag;
			}
			else {
				/*
				 * Just start another attribute ignoring an empty attributes for
				 * now. We don't use them in fact...
				 */
				state = parse_attr_name;
				parser_env.saved_p = in;
			}
		}
		break;

	case spaces_after_eq:
		if (*in == '"') {
			state = parse_start_dquote;
		}
		else if (*in == '\'') {
			state = parse_start_squote;
		}
		else if (!g_ascii_isspace (*in)) {
			if (parser_env.saved_p != nullptr) {
				/* We need to save this param */
				parser_env.saved_p = in;
			}
			state = parse_value;
		}
		break;

	case parse_equal:
		if (g_ascii_isspace (*in)) {
			state = spaces_after_eq;
		}
		else if (*in == '"') {
			state = parse_start_dquote;
		}
		else if (*in == '\'') {
			state = parse_start_squote;
		}
		else {
			if (parser_env.saved_p != nullptr) {
				/* We need to save this param */
				parser_env.saved_p = in;
			}
			state = parse_value;
		}
		break;

	case parse_start_dquote:
		if (*in == '"') {
			if (parser_env.saved_p != nullptr) {
				/* We have an empty attribute value */
				parser_env.saved_p = nullptr;
			}
			state = spaces_after_param;
		}
		else {
			if (parser_env.saved_p != nullptr) {
				/* We need to save this param */
				parser_env.saved_p = in;
			}
			state = parse_dqvalue;
		}
		break;

	case parse_start_squote:
		if (*in == '\'') {
			if (parser_env.saved_p != nullptr) {
				/* We have an empty attribute value */
				parser_env.saved_p = nullptr;
			}
			state = spaces_after_param;
		}
		else {
			if (parser_env.saved_p != nullptr) {
				/* We need to save this param */
				parser_env.saved_p = in;
			}
			state = parse_sqvalue;
		}
		break;

	case parse_dqvalue:
		if (*in == '"') {
			store = TRUE;
			state = parse_end_dquote;
		}

		if (store) {
			store_tag_component();
		}
		break;

	case parse_sqvalue:
		if (*in == '\'') {
			store = TRUE;
			state = parse_end_squote;
		}
		if (store) {
			store_tag_component();
		}
		break;

	case parse_value:
		if (*in == '/' && *(in + 1) == '>') {
			tag->flags |= FL_CLOSED;
			store = TRUE;
		}
		else if (g_ascii_isspace (*in) || *in == '>' || *in == '"') {
			store = TRUE;
			state = spaces_after_param;
		}

		if (store) {
			store_tag_component();
		}
		break;

	case parse_end_dquote:
	case parse_end_squote:
		if (g_ascii_isspace (*in)) {
			state = spaces_after_param;
		}
		else if (*in == '/' && *(in + 1) == '>') {
			tag->flags |= FL_CLOSED;
		}
		else {
			/* No space, proceed immediately to the attribute name */
			state = parse_attr_name;
			parser_env.saved_p = in;
		}
		break;

	case spaces_after_param:
		if (!g_ascii_isspace (*in)) {
			if (*in == '/' && *(in + 1) == '>') {
				tag->flags |= FL_CLOSED;
			}

			state = parse_attr_name;
			parser_env.saved_p = in;
		}
		break;

	case ignore_bad_tag:
		break;
	}

	parser_env.cur_state = state;
}

}

/* Unconverted C part */

static struct rspamd_url *rspamd_html_process_url(rspamd_mempool_t *pool,
												  const gchar *start, guint len,
												  struct html_tag_component *comp);




struct rspamd_url *
rspamd_html_process_url(rspamd_mempool_t *pool, const gchar *start, guint len,
						struct html_tag_component *comp) {
	struct rspamd_url *url;
	guint saved_flags = 0;
	gchar *decoded;
	gint rc;
	gsize decoded_len;
	const gchar *p, *s, *prefix = "http://";
	gchar *d;
	guint i;
	gsize dlen;
	gboolean has_bad_chars = FALSE, no_prefix = FALSE;
	static const gchar hexdigests[] = "0123456789abcdef";

	p = start;

	/* Strip spaces from the url */
	/* Head spaces */
	while (p < start + len && g_ascii_isspace (*p)) {
		p++;
		start++;
		len--;
	}

	if (comp) {
		comp->start = (guchar *)p;
		comp->len = len;
	}

	/* Trailing spaces */
	p = start + len - 1;

	while (p >= start && g_ascii_isspace (*p)) {
		p--;
		len--;

		if (comp) {
			comp->len--;
		}
	}

	s = start;
	dlen = 0;

	for (i = 0; i < len; i++) {
		if (G_UNLIKELY (((guint) s[i]) < 0x80 && !g_ascii_isgraph(s[i]))) {
			dlen += 3;
		}
		else {
			dlen++;
		}
	}

	if (rspamd_substring_search(start, len, "://", 3) == -1) {
		if (len >= sizeof("mailto:") &&
			(memcmp(start, "mailto:", sizeof("mailto:") - 1) == 0 ||
			 memcmp(start, "tel:", sizeof("tel:") - 1) == 0 ||
			 memcmp(start, "callto:", sizeof("callto:") - 1) == 0)) {
			/* Exclusion, has valid but 'strange' prefix */
		}
		else {
			for (i = 0; i < len; i++) {
				if (!((s[i] & 0x80) || g_ascii_isalnum (s[i]))) {
					if (i == 0 && len > 2 && s[i] == '/' && s[i + 1] == '/') {
						prefix = "http:";
						dlen += sizeof("http:") - 1;
						no_prefix = TRUE;
					}
					else if (s[i] == '@') {
						/* Likely email prefix */
						prefix = "mailto://";
						dlen += sizeof("mailto://") - 1;
						no_prefix = TRUE;
					}
					else if (s[i] == ':' && i != 0) {
						/* Special case */
						no_prefix = FALSE;
					}
					else {
						if (i == 0) {
							/* No valid data */
							return NULL;
						}
						else {
							no_prefix = TRUE;
							dlen += strlen(prefix);
						}
					}

					break;
				}
			}
		}
	}

	decoded = (char *)rspamd_mempool_alloc (pool, dlen + 1);
	d = decoded;

	if (no_prefix) {
		gsize plen = strlen(prefix);
		memcpy(d, prefix, plen);
		d += plen;
	}

	/*
	 * We also need to remove all internal newlines, spaces
	 * and encode unsafe characters
	 */
	for (i = 0; i < len; i++) {
		if (G_UNLIKELY (g_ascii_isspace(s[i]))) {
			continue;
		}
		else if (G_UNLIKELY (((guint) s[i]) < 0x80 && !g_ascii_isgraph(s[i]))) {
			/* URL encode */
			*d++ = '%';
			*d++ = hexdigests[(s[i] >> 4) & 0xf];
			*d++ = hexdigests[s[i] & 0xf];
			has_bad_chars = TRUE;
		}
		else {
			*d++ = s[i];
		}
	}

	*d = '\0';
	dlen = d - decoded;

	url = rspamd_mempool_alloc0_type(pool, struct rspamd_url);

	rspamd_url_normalise_propagate_flags (pool, decoded, &dlen, saved_flags);

	rc = rspamd_url_parse(url, decoded, dlen, pool, RSPAMD_URL_PARSE_HREF);

	/* Filter some completely damaged urls */
	if (rc == URI_ERRNO_OK && url->hostlen > 0 &&
		!((url->protocol & PROTOCOL_UNKNOWN))) {
		url->flags |= saved_flags;

		if (has_bad_chars) {
			url->flags |= RSPAMD_URL_FLAG_OBSCURED;
		}

		if (no_prefix) {
			url->flags |= RSPAMD_URL_FLAG_SCHEMALESS;

			if (url->tldlen == 0 || (url->flags & RSPAMD_URL_FLAG_NO_TLD)) {
				/* Ignore urls with both no schema and no tld */
				return NULL;
			}
		}

		decoded = url->string;
		decoded_len = url->urllen;

		if (comp) {
			comp->start = (guchar *)decoded;
			comp->len = decoded_len;
		}
		/* Spaces in href usually mean an attempt to obfuscate URL */
		/* See https://github.com/vstakhov/rspamd/issues/593 */
#if 0
		if (has_spaces) {
			url->flags |= RSPAMD_URL_FLAG_OBSCURED;
		}
#endif

		return url;
	}

	return NULL;
}

static struct rspamd_url *
rspamd_html_process_url_tag(rspamd_mempool_t *pool, struct html_tag *tag,
							struct html_content *hc) {
	struct html_tag_component *comp;
	GList *cur;
	struct rspamd_url *url;
	const gchar *start;
	gsize len;

	cur = tag->params->head;

	while (cur) {
		comp = (struct html_tag_component *)cur->data;

		if (comp->type == RSPAMD_HTML_COMPONENT_HREF && comp->len > 0) {
			start = (char *)comp->start;
			len = comp->len;

			/* Check base url */
			if (hc && hc->base_url && comp->len > 2) {
				/*
				 * Relative url cannot start from the following:
				 * schema://
				 * data:
				 * slash
				 */
				gchar *buf;
				gsize orig_len;

				if (rspamd_substring_search(start, len, "://", 3) == -1) {

					if (len >= sizeof("data:") &&
						g_ascii_strncasecmp(start, "data:", sizeof("data:") - 1) == 0) {
						/* Image data url, never insert as url */
						return NULL;
					}

					/* Assume relative url */

					gboolean need_slash = FALSE;

					orig_len = len;
					len += hc->base_url->urllen;

					if (hc->base_url->datalen == 0) {
						need_slash = TRUE;
						len++;
					}

					buf = (char *)rspamd_mempool_alloc (pool, len + 1);
					rspamd_snprintf(buf, len + 1, "%*s%s%*s",
							hc->base_url->urllen, hc->base_url->string,
							need_slash ? "/" : "",
							(gint) orig_len, start);
					start = buf;
				}
				else if (start[0] == '/' && start[1] != '/') {
					/* Relative to the hostname */
					orig_len = len;
					len += hc->base_url->hostlen + hc->base_url->protocollen +
						   3 /* for :// */;
					buf = (char *)rspamd_mempool_alloc (pool, len + 1);
					rspamd_snprintf(buf, len + 1, "%*s://%*s/%*s",
							hc->base_url->protocollen, hc->base_url->string,
							hc->base_url->hostlen, rspamd_url_host_unsafe (hc->base_url),
							(gint) orig_len, start);
					start = buf;
				}
			}

			url = rspamd_html_process_url(pool, start, len, comp);

			if (url && tag->extra == NULL) {
				tag->extra = url;
			}

			return url;
		}

		cur = g_list_next (cur);
	}

	return NULL;
}

struct rspamd_html_url_query_cbd {
	rspamd_mempool_t *pool;
	khash_t (rspamd_url_hash) *url_set;
	struct rspamd_url *url;
	GPtrArray *part_urls;
};

static gboolean
rspamd_html_url_query_callback(struct rspamd_url *url, gsize start_offset,
							   gsize end_offset, gpointer ud) {
	struct rspamd_html_url_query_cbd *cbd =
			(struct rspamd_html_url_query_cbd *) ud;
	rspamd_mempool_t *pool;

	pool = cbd->pool;

	if (url->protocol == PROTOCOL_MAILTO) {
		if (url->userlen == 0) {
			return FALSE;
		}
	}

	msg_debug_html ("found url %s in query of url"
					" %*s", url->string,
			cbd->url->querylen, rspamd_url_query_unsafe(cbd->url));

	url->flags |= RSPAMD_URL_FLAG_QUERY;

	if (rspamd_url_set_add_or_increase(cbd->url_set, url, false)
		&& cbd->part_urls) {
		g_ptr_array_add(cbd->part_urls, url);
	}

	return TRUE;
}

static void
rspamd_process_html_url(rspamd_mempool_t *pool, struct rspamd_url *url,
						khash_t (rspamd_url_hash) *url_set,
						GPtrArray *part_urls) {
	if (url->querylen > 0) {
		struct rspamd_html_url_query_cbd qcbd;

		qcbd.pool = pool;
		qcbd.url_set = url_set;
		qcbd.url = url;
		qcbd.part_urls = part_urls;

		rspamd_url_find_multiple(pool,
				rspamd_url_query_unsafe (url), url->querylen,
				RSPAMD_URL_FIND_ALL, NULL,
				rspamd_html_url_query_callback, &qcbd);
	}

	if (part_urls) {
		g_ptr_array_add(part_urls, url);
	}
}

static void
rspamd_html_process_data_image(rspamd_mempool_t *pool,
							   struct html_image *img,
							   struct html_tag_component *src) {
	/*
	 * Here, we do very basic processing of the data:
	 * detect if we have something like: `data:image/xxx;base64,yyyzzz==`
	 * We only parse base64 encoded data.
	 * We ignore content type so far
	 */
	struct rspamd_image *parsed_image;
	const gchar *semicolon_pos = NULL, *end = (gchar *)src->start + src->len;

	semicolon_pos = (gchar *)src->start;

	while ((semicolon_pos = (gchar *)memchr(semicolon_pos, ';', end - semicolon_pos)) != NULL) {
		if (end - semicolon_pos > sizeof("base64,")) {
			if (memcmp(semicolon_pos + 1, "base64,", sizeof("base64,") - 1) == 0) {
				const gchar *data_pos = semicolon_pos + sizeof("base64,");
				gchar *decoded;
				gsize encoded_len = end - data_pos, decoded_len;
				rspamd_ftok_t inp;

				decoded_len = (encoded_len / 4 * 3) + 12;
				decoded = (gchar *)rspamd_mempool_alloc (pool, decoded_len);
				rspamd_cryptobox_base64_decode(data_pos, encoded_len,
						reinterpret_cast<guchar *>(decoded), &decoded_len);
				inp.begin = decoded;
				inp.len = decoded_len;

				parsed_image = rspamd_maybe_process_image(pool, &inp);

				if (parsed_image) {
					msg_debug_html ("detected %s image of size %ud x %ud in data url",
							rspamd_image_type_str(parsed_image->type),
							parsed_image->width, parsed_image->height);
					img->embedded_image = parsed_image;
				}
			}

			break;
		}
		else {
			/* Nothing useful */
			return;
		}

		semicolon_pos++;
	}
}

static void
rspamd_html_process_img_tag(rspamd_mempool_t *pool, struct html_tag *tag,
							struct html_content *hc, khash_t (rspamd_url_hash) *url_set,
							GPtrArray *part_urls,
							GByteArray *dest) {
	struct html_tag_component *comp;
	struct html_image *img;
	rspamd_ftok_t fstr;
	const guchar *p;
	GList *cur;
	gulong val;
	gboolean seen_width = FALSE, seen_height = FALSE;
	goffset pos;

	cur = tag->params->head;
	img = rspamd_mempool_alloc0_type (pool, struct html_image);
	img->tag = tag;
	tag->flags |= FL_IMAGE;

	while (cur) {
		comp = static_cast<html_tag_component *>(cur->data);

		if (comp->type == RSPAMD_HTML_COMPONENT_HREF && comp->len > 0) {
			fstr.begin = (gchar *) comp->start;
			fstr.len = comp->len;
			img->src = rspamd_mempool_ftokdup (pool, &fstr);

			if (comp->len > sizeof("cid:") - 1 && memcmp(comp->start,
					"cid:", sizeof("cid:") - 1) == 0) {
				/* We have an embedded image */
				img->flags |= RSPAMD_HTML_FLAG_IMAGE_EMBEDDED;
			}
			else {
				if (comp->len > sizeof("data:") - 1 && memcmp(comp->start,
						"data:", sizeof("data:") - 1) == 0) {
					/* We have an embedded image in HTML tag */
					img->flags |=
							(RSPAMD_HTML_FLAG_IMAGE_EMBEDDED | RSPAMD_HTML_FLAG_IMAGE_DATA);
					rspamd_html_process_data_image(pool, img, comp);
					hc->flags |= RSPAMD_HTML_FLAG_HAS_DATA_URLS;
				}
				else {
					img->flags |= RSPAMD_HTML_FLAG_IMAGE_EXTERNAL;
					if (img->src) {

						img->url = rspamd_html_process_url(pool,
								img->src, fstr.len, NULL);

						if (img->url) {
							struct rspamd_url *existing;

							img->url->flags |= RSPAMD_URL_FLAG_IMAGE;
							existing = rspamd_url_set_add_or_return(url_set, img->url);

							if (existing != img->url) {
								/*
								 * We have some other URL that could be
								 * found, e.g. from another part. However,
								 * we still want to set an image flag on it
								 */
								existing->flags |= img->url->flags;
								existing->count++;
							}
							else if (part_urls) {
								/* New url */
								g_ptr_array_add(part_urls, img->url);
							}
						}
					}
				}
			}
		}
		else if (comp->type == RSPAMD_HTML_COMPONENT_HEIGHT) {
			rspamd_strtoul(reinterpret_cast<const gchar *>(comp->start), comp->len, &val);
			img->height = val;
			seen_height = TRUE;
		}
		else if (comp->type == RSPAMD_HTML_COMPONENT_WIDTH) {
			rspamd_strtoul(reinterpret_cast<const gchar *>(comp->start), comp->len, &val);
			img->width = val;
			seen_width = TRUE;
		}
		else if (comp->type == RSPAMD_HTML_COMPONENT_STYLE) {
			/* Try to search for height= or width= in style tag */
			if (!seen_height && comp->len > 0) {
				pos = rspamd_substring_search_caseless(reinterpret_cast<const gchar *>(comp->start),
						comp->len,
						"height", sizeof("height") - 1);

				if (pos != -1) {
					p = comp->start + pos + sizeof("height") - 1;

					while (p < comp->start + comp->len) {
						if (g_ascii_isdigit (*p)) {
							rspamd_strtoul(reinterpret_cast<const gchar *>(p),
									comp->len - (p - comp->start), &val);
							img->height = val;
							break;
						}
						else if (!g_ascii_isspace (*p) && *p != '=' && *p != ':') {
							/* Fallback */
							break;
						}
						p++;
					}
				}
			}

			if (!seen_width && comp->len > 0) {
				pos = rspamd_substring_search_caseless(reinterpret_cast<const gchar *>(comp->start),
						comp->len,
						"width", sizeof("width") - 1);

				if (pos != -1) {
					p = comp->start + pos + sizeof("width") - 1;

					while (p < comp->start + comp->len) {
						if (g_ascii_isdigit (*p)) {
							rspamd_strtoul(reinterpret_cast<const gchar *>(p),
									comp->len - (p - comp->start), &val);
							img->width = val;
							break;
						}
						else if (!g_ascii_isspace (*p) && *p != '=' && *p != ':') {
							/* Fallback */
							break;
						}
						p++;
					}
				}
			}
		}
		else if (comp->type == RSPAMD_HTML_COMPONENT_ALT && comp->len > 0 && dest != NULL) {
			if (dest->len > 0 && !g_ascii_isspace (dest->data[dest->len - 1])) {
				/* Add a space */
				g_byte_array_append(dest, reinterpret_cast<const guint8 *>(" "), 1);
			}

			g_byte_array_append(dest, comp->start, comp->len);

			if (!g_ascii_isspace (dest->data[dest->len - 1])) {
				/* Add a space */
				g_byte_array_append(dest, reinterpret_cast<const guint8 *>(" "), 1);
			}
		}

		cur = g_list_next (cur);
	}

	if (hc->images == NULL) {
		hc->images = g_ptr_array_sized_new(4);
		rspamd_mempool_notify_alloc (pool, 4 * sizeof(gpointer) + sizeof(GPtrArray));
		rspamd_mempool_add_destructor (pool, rspamd_ptr_array_free_hard,
				hc->images);
	}

	if (img->embedded_image) {
		if (!seen_height) {
			img->height = img->embedded_image->height;
		}
		if (!seen_width) {
			img->width = img->embedded_image->width;
		}
	}

	g_ptr_array_add(hc->images, img);
	tag->extra = img;
}

static void
rspamd_html_process_link_tag(rspamd_mempool_t *pool, struct html_tag *tag,
							 struct html_content *hc, khash_t (rspamd_url_hash) *url_set,
							 GPtrArray *part_urls) {
	struct html_tag_component *comp;
	GList *cur;

	cur = tag->params->head;

	while (cur) {
		comp = static_cast<html_tag_component *>(cur->data);

		if (comp->type == RSPAMD_HTML_COMPONENT_REL && comp->len > 0) {
			if (comp->len == sizeof("icon") - 1 &&
				rspamd_lc_cmp(reinterpret_cast<const gchar *>(comp->start), "icon", sizeof("icon") - 1) == 0) {

				rspamd_html_process_img_tag(pool, tag, hc, url_set, part_urls, NULL);
			}
		}

		cur = g_list_next (cur);
	}
}

static void
rspamd_html_process_color(const gchar *line, guint len, struct html_color *cl)
{
	const gchar *p = line, *end = line + len;
	char hexbuf[7];

	memset(cl, 0, sizeof(*cl));

	if (*p == '#') {
		/* HEX color */
		p++;
		rspamd_strlcpy(hexbuf, p, MIN ((gint) sizeof(hexbuf), end - p + 1));
		cl->d.val = strtoul(hexbuf, NULL, 16);
		cl->d.comp.alpha = 255;
		cl->valid = TRUE;
	}
	else if (len > 4 && rspamd_lc_cmp(p, "rgb", 3) == 0) {
		/* We have something like rgba(x,x,x,x) or rgb(x,x,x) */
		enum {
			obrace,
			num1,
			num2,
			num3,
			num4,
			skip_spaces
		} state = skip_spaces, next_state = obrace;
		gulong r = 0, g = 0, b = 0, opacity = 255;
		const gchar *c;
		gboolean valid = FALSE;

		p += 3;

		if (*p == 'a') {
			p++;
		}

		c = p;

		while (p < end) {
			switch (state) {
			case obrace:
				if (*p == '(') {
					p++;
					state = skip_spaces;
					next_state = num1;
				}
				else if (g_ascii_isspace (*p)) {
					state = skip_spaces;
					next_state = obrace;
				}
				else {
					goto stop;
				}
				break;
			case num1:
				if (*p == ',') {
					if (!rspamd_strtoul(c, p - c, &r)) {
						goto stop;
					}

					p++;
					state = skip_spaces;
					next_state = num2;
				}
				else if (!g_ascii_isdigit (*p)) {
					goto stop;
				}
				else {
					p++;
				}
				break;
			case num2:
				if (*p == ',') {
					if (!rspamd_strtoul(c, p - c, &g)) {
						goto stop;
					}

					p++;
					state = skip_spaces;
					next_state = num3;
				}
				else if (!g_ascii_isdigit (*p)) {
					goto stop;
				}
				else {
					p++;
				}
				break;
			case num3:
				if (*p == ',') {
					if (!rspamd_strtoul(c, p - c, &b)) {
						goto stop;
					}

					valid = TRUE;
					p++;
					state = skip_spaces;
					next_state = num4;
				}
				else if (*p == ')') {
					if (!rspamd_strtoul(c, p - c, &b)) {
						goto stop;
					}

					valid = TRUE;
					goto stop;
				}
				else if (!g_ascii_isdigit (*p)) {
					goto stop;
				}
				else {
					p++;
				}
				break;
			case num4:
				if (*p == ',') {
					if (!rspamd_strtoul(c, p - c, &opacity)) {
						goto stop;
					}

					valid = TRUE;
					goto stop;
				}
				else if (*p == ')') {
					if (!rspamd_strtoul(c, p - c, &opacity)) {
						goto stop;
					}

					valid = TRUE;
					goto stop;
				}
				else if (!g_ascii_isdigit (*p)) {
					goto stop;
				}
				else {
					p++;
				}
				break;
			case skip_spaces:
				if (!g_ascii_isspace (*p)) {
					c = p;
					state = next_state;
				}
				else {
					p++;
				}
				break;
			}
		}

stop:

		if (valid) {
			cl->d.comp.r = r;
			cl->d.comp.g = g;
			cl->d.comp.b = b;
			cl->d.comp.alpha = opacity;
			cl->valid = TRUE;
		}
	}
	else {
		auto maybe_color_value =
				rspamd::css::css_value::maybe_color_from_string({line, len});

		if (maybe_color_value.has_value()) {
			auto color = maybe_color_value->to_color().value();
			cl->d.val = color.to_number();
			cl->d.comp.alpha = 255; /* Non transparent */
		}
	}
}

/*
 * Target is used for in and out if this function returns TRUE
 */
static gboolean
rspamd_html_process_css_size(const gchar *suffix, gsize len,
							 gdouble *tgt) {
	gdouble sz = *tgt;
	gboolean ret = FALSE;

	if (len >= 2) {
		if (memcmp(suffix, "px", 2) == 0) {
			sz = (guint) sz; /* Round to number */
			ret = TRUE;
		}
		else if (memcmp(suffix, "em", 2) == 0) {
			/* EM is 16 px, so multiply and round */
			sz = (guint) (sz * 16.0);
			ret = TRUE;
		}
		else if (len >= 3 && memcmp(suffix, "rem", 3) == 0) {
			/* equal to EM in our case */
			sz = (guint) (sz * 16.0);
			ret = TRUE;
		}
		else if (memcmp(suffix, "ex", 2) == 0) {
			/*
			 * Represents the x-height of the element's font.
			 * On fonts with the "x" letter, this is generally the height
			 * of lowercase letters in the font; 1ex = 0.5em in many fonts.
			 */
			sz = (guint) (sz * 8.0);
			ret = TRUE;
		}
		else if (memcmp(suffix, "vw", 2) == 0) {
			/*
			 * Vewport width in percentages:
			 * we assume 1% of viewport width as 8px
			 */
			sz = (guint) (sz * 8.0);
			ret = TRUE;
		}
		else if (memcmp(suffix, "vh", 2) == 0) {
			/*
			 * Vewport height in percentages
			 * we assume 1% of viewport width as 6px
			 */
			sz = (guint) (sz * 6.0);
			ret = TRUE;
		}
		else if (len >= 4 && memcmp(suffix, "vmax", 4) == 0) {
			/*
			 * Vewport width in percentages
			 * we assume 1% of viewport width as 6px
			 */
			sz = (guint) (sz * 8.0);
			ret = TRUE;
		}
		else if (len >= 4 && memcmp(suffix, "vmin", 4) == 0) {
			/*
			 * Vewport height in percentages
			 * we assume 1% of viewport width as 6px
			 */
			sz = (guint) (sz * 6.0);
			ret = TRUE;
		}
		else if (memcmp(suffix, "pt", 2) == 0) {
			sz = (guint) (sz * 96.0 / 72.0); /* One point. 1pt = 1/72nd of 1in */
			ret = TRUE;
		}
		else if (memcmp(suffix, "cm", 2) == 0) {
			sz = (guint) (sz * 96.0 / 2.54); /* 96px/2.54 */
			ret = TRUE;
		}
		else if (memcmp(suffix, "mm", 2) == 0) {
			sz = (guint) (sz * 9.6 / 2.54); /* 9.6px/2.54 */
			ret = TRUE;
		}
		else if (memcmp(suffix, "in", 2) == 0) {
			sz = (guint) (sz * 96.0); /* 96px */
			ret = TRUE;
		}
		else if (memcmp(suffix, "pc", 2) == 0) {
			sz = (guint) (sz * 96.0 / 6.0); /* 1pc = 12pt = 1/6th of 1in. */
			ret = TRUE;
		}
	}
	else if (suffix[0] == '%') {
		/* Percentages from 16 px */
		sz = (guint) (sz / 100.0 * 16.0);
		ret = TRUE;
	}

	if (ret) {
		*tgt = sz;
	}

	return ret;
}

static void
rspamd_html_process_font_size(const gchar *line, guint len, guint *fs,
							  gboolean is_css) {
	const gchar *p = line, *end = line + len;
	gchar *err = NULL, numbuf[64];
	gdouble sz = 0;
	gboolean failsafe = FALSE;

	while (p < end && g_ascii_isspace (*p)) {
		p++;
		len--;
	}

	if (g_ascii_isdigit (*p)) {
		rspamd_strlcpy(numbuf, p, MIN (sizeof(numbuf), len + 1));
		sz = strtod(numbuf, &err);

		/* Now check leftover */
		if (sz < 0) {
			sz = 0;
		}
	}
	else {
		/* Ignore the rest */
		failsafe = TRUE;
		sz = is_css ? 16 : 1;
		/* TODO: add textual fonts descriptions */
	}

	if (err && *err != '\0') {
		const gchar *e = err;
		gsize slen;

		/* Skip spaces */
		while (*e && g_ascii_isspace (*e)) {
			e++;
		}

		/* Lowercase */
		slen = strlen(e);
		rspamd_str_lc((gchar *) e, slen);

		if (!rspamd_html_process_css_size(e, slen, &sz)) {
			failsafe = TRUE;
		}
	}
	else {
		/* Failsafe naked number */
		failsafe = TRUE;
	}

	if (failsafe) {
		if (is_css) {
			/*
			 * In css mode we usually ignore sizes, but let's treat
			 * small sizes specially
			 */
			if (sz < 1) {
				sz = 0;
			}
			else {
				sz = 16; /* Ignore */
			}
		}
		else {
			/* In non-css mode we have to check legacy size */
			sz = sz >= 1 ? sz * 16 : 16;
		}
	}

	if (sz > 32) {
		sz = 32;
	}

	*fs = sz;
}

static void
rspamd_html_process_style(rspamd_mempool_t *pool, struct html_block *bl,
						  struct html_content *hc, const gchar *style, guint len) {
	const gchar *p, *c, *end, *key = NULL;
	enum {
		read_key,
		read_colon,
		read_value,
		skip_spaces,
	} state = skip_spaces, next_state = read_key;
	guint klen = 0;
	gdouble opacity = 1.0;

	p = style;
	c = p;
	end = p + len;

	while (p <= end) {
		switch (state) {
		case read_key:
			if (p == end || *p == ':') {
				key = c;
				klen = p - c;
				state = skip_spaces;
				next_state = read_value;
			}
			else if (g_ascii_isspace (*p)) {
				key = c;
				klen = p - c;
				state = skip_spaces;
				next_state = read_colon;
			}

			p++;
			break;

		case read_colon:
			if (p == end || *p == ':') {
				state = skip_spaces;
				next_state = read_value;
			}

			p++;
			break;

		case read_value:
			if (p == end || *p == ';') {
				if (key && klen && p - c > 0) {
					if ((klen == 5 && g_ascii_strncasecmp(key, "color", 5) == 0)
						|| (klen == 10 && g_ascii_strncasecmp(key, "font-color", 10) == 0)) {

						rspamd_html_process_color(c, p - c, &bl->font_color);
						msg_debug_html ("got color: %xd", bl->font_color.d.val);
					}
					else if ((klen == 16 && g_ascii_strncasecmp(key,
							"background-color", 16) == 0) ||
							 (klen == 10 && g_ascii_strncasecmp(key,
									 "background", 10) == 0)) {

						rspamd_html_process_color(c, p - c, &bl->background_color);
						msg_debug_html ("got bgcolor: %xd", bl->background_color.d.val);
					}
					else if (klen == 7 && g_ascii_strncasecmp(key, "display", 7) == 0) {
						if (p - c >= 4 && rspamd_substring_search_caseless(c, p - c,
								"none", 4) != -1) {
							bl->visible = FALSE;
							msg_debug_html ("tag is not visible");
						}
					}
					else if (klen == 9 &&
							 g_ascii_strncasecmp(key, "font-size", 9) == 0) {
						rspamd_html_process_font_size(c, p - c,
								&bl->font_size, TRUE);
						msg_debug_html ("got font size: %ud", bl->font_size);
					}
					else if (klen == 7 &&
							 g_ascii_strncasecmp(key, "opacity", 7) == 0) {
						gchar numbuf[64];

						rspamd_strlcpy(numbuf, c,
								MIN (sizeof(numbuf), p - c + 1));
						opacity = strtod(numbuf, NULL);

						if (opacity > 1) {
							opacity = 1;
						}
						else if (opacity < 0) {
							opacity = 0;
						}

						bl->font_color.d.comp.alpha = (guint8) (opacity * 255.0);
					}
					else if (klen == 10 &&
							 g_ascii_strncasecmp(key, "visibility", 10) == 0) {
						if (p - c >= 6 && rspamd_substring_search_caseless(c,
								p - c,
								"hidden", 6) != -1) {
							bl->visible = FALSE;
							msg_debug_html ("tag is not visible");
						}
					}
				}

				key = NULL;
				klen = 0;
				state = skip_spaces;
				next_state = read_key;
			}

			p++;
			break;

		case skip_spaces:
			if (p < end && !g_ascii_isspace (*p)) {
				c = p;
				state = next_state;
			}
			else {
				p++;
			}

			break;
		}
	}
}

static void
rspamd_html_process_block_tag(rspamd_mempool_t *pool, struct html_tag *tag,
							  struct html_content *hc) {
	struct html_tag_component *comp;
	struct html_block *bl;
	rspamd_ftok_t fstr;
	GList *cur;

	cur = tag->params->head;
	bl = rspamd_mempool_alloc0_type (pool, struct html_block);
	bl->tag = tag;
	bl->visible = TRUE;
	bl->font_size = (guint) -1;
	bl->font_color.d.comp.alpha = 255;

	while (cur) {
		comp = static_cast<html_tag_component *>(cur->data);

		if (comp->len > 0) {
			switch (comp->type) {
			case RSPAMD_HTML_COMPONENT_COLOR:
				fstr.begin = (gchar *) comp->start;
				fstr.len = comp->len;
				rspamd_html_process_color(reinterpret_cast<const gchar *>(comp->start), comp->len,
						&bl->font_color);
				msg_debug_html ("tag %*s; got color: %xd",
						tag->name.len, tag->name.start, bl->font_color.d.val);
				break;
			case RSPAMD_HTML_COMPONENT_BGCOLOR:
				fstr.begin = (gchar *) comp->start;
				fstr.len = comp->len;
				rspamd_html_process_color(reinterpret_cast<const gchar *>(comp->start), comp->len,
						&bl->background_color);
				msg_debug_html ("tag %*s; got color: %xd",
						tag->name.len, tag->name.start, bl->font_color.d.val);

				if (tag->id == Tag_BODY) {
					/* Set global background color */
					memcpy(&hc->bgcolor, &bl->background_color,
							sizeof(hc->bgcolor));
				}
				break;
			case RSPAMD_HTML_COMPONENT_STYLE:
				bl->style.len = comp->len;
				bl->style.start = comp->start;
				msg_debug_html ("tag: %*s; got style: %*s",
						tag->name.len, tag->name.start,
						(gint) bl->style.len, bl->style.start);
				rspamd_html_process_style(pool, bl, hc, reinterpret_cast<const gchar *>(comp->start), comp->len);
				break;
			case RSPAMD_HTML_COMPONENT_CLASS:
				fstr.begin = (gchar *) comp->start;
				fstr.len = comp->len;
				bl->html_class = rspamd_mempool_ftokdup (pool, &fstr);
				msg_debug_html ("tag: %*s; got class: %s",
						tag->name.len, tag->name.start, bl->html_class);
				break;
			case RSPAMD_HTML_COMPONENT_SIZE:
				/* Not supported by html5 */
				/* FIXME maybe support it */
				bl->font_size = 16;
				msg_debug_html ("tag %*s; got size: %*s",
						tag->name.len, tag->name.start,
						(gint) comp->len, comp->start);
				break;
			default:
				/* NYI */
				break;
			}
		}

		cur = g_list_next (cur);
	}

	if (hc->blocks == NULL) {
		hc->blocks = g_ptr_array_sized_new(64);
		rspamd_mempool_notify_alloc (pool, 64 * sizeof(gpointer) + sizeof(GPtrArray));
		rspamd_mempool_add_destructor (pool, rspamd_ptr_array_free_hard,
				hc->blocks);
	}

	g_ptr_array_add(hc->blocks, bl);
	tag->extra = bl;
}

static void
rspamd_html_check_displayed_url(rspamd_mempool_t *pool,
								GList **exceptions,
								khash_t (rspamd_url_hash) *url_set,
								GByteArray *dest,
								gint href_offset,
								struct rspamd_url *url) {
	struct rspamd_url *displayed_url = NULL;
	struct rspamd_url *turl;
	gboolean url_found = FALSE;
	struct rspamd_process_exception *ex;
	guint saved_flags = 0;
	gsize dlen;

	if (href_offset < 0) {
		/* No dispalyed url, just some text within <a> tag */
		return;
	}

	url->visible_part = (gchar *)rspamd_mempool_alloc (pool, dest->len - href_offset + 1);
	rspamd_strlcpy(url->visible_part,
			reinterpret_cast<const gchar *>(dest->data + href_offset),
			dest->len - href_offset + 1);
	dlen = dest->len - href_offset;

	/* Strip unicode spaces from the start and the end */
	url->visible_part = rspamd_string_unicode_trim_inplace(url->visible_part,
			&dlen);
	rspamd_html_url_is_phished(pool, url,
			reinterpret_cast<const guchar *>(url->visible_part),
			dlen,
			&url_found, &displayed_url);

	if (url_found) {
		url->flags |= saved_flags | RSPAMD_URL_FLAG_DISPLAY_URL;
	}

	if (exceptions && url_found) {
		ex = rspamd_mempool_alloc_type (pool,struct rspamd_process_exception);
		ex->pos = href_offset;
		ex->len = dest->len - href_offset;
		ex->type = RSPAMD_EXCEPTION_URL;
		ex->ptr = url;

		*exceptions = g_list_prepend(*exceptions,
				ex);
	}

	if (displayed_url && url_set) {
		turl = rspamd_url_set_add_or_return(url_set,
				displayed_url);

		if (turl != NULL) {
			/* Here, we assume the following:
			 * if we have a URL in the text part which
			 * is the same as displayed URL in the
			 * HTML part, we assume that it is also
			 * hint only.
			 */
			if (turl->flags &
				RSPAMD_URL_FLAG_FROM_TEXT) {
				turl->flags |= RSPAMD_URL_FLAG_HTML_DISPLAYED;
				turl->flags &= ~RSPAMD_URL_FLAG_FROM_TEXT;
			}

			turl->count++;
		}
		else {
			/* Already inserted by `rspamd_url_set_add_or_return` */
		}
	}

	rspamd_normalise_unicode_inplace(url->visible_part, &dlen);
}

static gboolean
rspamd_html_propagate_lengths(GNode *node, gpointer _unused) {
	GNode *child;
	struct html_tag *tag = static_cast<html_tag *>(node->data), *cld_tag;

	if (tag) {
		child = node->children;

		/* Summarize content length from children */
		while (child) {
			cld_tag = static_cast<html_tag *>(child->data);
			tag->content_length += cld_tag->content_length;
			child = child->next;
		}
	}

	return FALSE;
}

static void
rspamd_html_propagate_style(struct html_content *hc,
							struct html_tag *tag,
							struct html_block *bl,
							GQueue *blocks) {
	struct html_block *bl_parent;
	gboolean push_block = FALSE;


	/* Propagate from the parent if needed */
	bl_parent = static_cast<html_block *>(g_queue_peek_tail(blocks));

	if (bl_parent) {
		if (!bl->background_color.valid) {
			/* Try to propagate background color from parent nodes */
			if (bl_parent->background_color.valid) {
				memcpy(&bl->background_color, &bl_parent->background_color,
						sizeof(bl->background_color));
			}
		}
		else {
			push_block = TRUE;
		}

		if (!bl->font_color.valid) {
			/* Try to propagate background color from parent nodes */
			if (bl_parent->font_color.valid) {
				memcpy(&bl->font_color, &bl_parent->font_color,
						sizeof(bl->font_color));
			}
		}
		else {
			push_block = TRUE;
		}

		/* Propagate font size */
		if (bl->font_size == (guint) -1) {
			if (bl_parent->font_size != (guint) -1) {
				bl->font_size = bl_parent->font_size;
			}
		}
		else {
			push_block = TRUE;
		}
	}

	/* Set bgcolor to the html bgcolor and font color to black as a last resort */
	if (!bl->font_color.valid) {
		/* Don't touch opacity as it can be set separately */
		bl->font_color.d.comp.r = 0;
		bl->font_color.d.comp.g = 0;
		bl->font_color.d.comp.b = 0;
		bl->font_color.valid = TRUE;
	}
	else {
		push_block = TRUE;
	}

	if (!bl->background_color.valid) {
		memcpy(&bl->background_color, &hc->bgcolor, sizeof(hc->bgcolor));
	}
	else {
		push_block = TRUE;
	}

	if (bl->font_size == (guint) -1) {
		bl->font_size = 16; /* Default for browsers */
	}
	else {
		push_block = TRUE;
	}

	if (push_block && !(tag->flags & FL_CLOSED)) {
		g_queue_push_tail(blocks, bl);
	}
}


GByteArray*
rspamd_html_process_part_full (rspamd_mempool_t *pool,
							   struct html_content *hc,
							   GByteArray *in,
							   GList **exceptions,
							   khash_t (rspamd_url_hash) *url_set,
							   GPtrArray *part_urls,
							   bool allow_css)
{
	const guchar *p, *c, *end, *savep = NULL;
	guchar t;
	gboolean closing = FALSE, need_decode = FALSE, save_space = FALSE,
			balanced;
	GByteArray *dest;
	guint obrace = 0, ebrace = 0;
	GNode *cur_level = NULL;
	gint substate = 0, len, href_offset = -1;
	struct html_tag *cur_tag = NULL, *content_tag = NULL;
	struct rspamd_url *url = NULL;
	GQueue *styles_blocks;

	enum {
		parse_start = 0,
		tag_begin,
		sgml_tag,
		xml_tag,
		compound_tag,
		comment_tag,
		comment_content,
		sgml_content,
		tag_content,
		tag_end,
		xml_tag_end,
		content_ignore,
		content_write,
		content_style,
		content_ignore_sp
	} state = parse_start;

	g_assert (in != NULL);
	g_assert (hc != NULL);
	g_assert (pool != NULL);

	hc->tags_seen = (guchar *)rspamd_mempool_alloc0 (pool, NBYTES (N_TAGS));

	/* Set white background color by default */
	hc->bgcolor.d.comp.alpha = 0;
	hc->bgcolor.d.comp.r = 255;
	hc->bgcolor.d.comp.g = 255;
	hc->bgcolor.d.comp.b = 255;
	hc->bgcolor.valid = TRUE;

	dest = g_byte_array_sized_new (in->len / 3 * 2);
	styles_blocks = g_queue_new ();

	p = in->data;
	c = p;
	end = p + in->len;

	while (p < end) {
		t = *p;

		switch (state) {
		case parse_start:
			if (t == '<') {
				state = tag_begin;
			}
			else {
				/* We have no starting tag, so assume that it's content */
				hc->flags |= RSPAMD_HTML_FLAG_BAD_START;
				state = content_write;
			}

			break;
		case tag_begin:
			switch (t) {
			case '<':
				p ++;
				closing = FALSE;
				break;
			case '!':
				state = sgml_tag;
				p ++;
				break;
			case '?':
				state = xml_tag;
				hc->flags |= RSPAMD_HTML_FLAG_XML;
				p ++;
				break;
			case '/':
				closing = TRUE;
				p ++;
				break;
			case '>':
				/* Empty tag */
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				state = tag_end;
				continue;
			default:
				state = tag_content;
				substate = 0;
				savep = NULL;
				cur_tag = rspamd_mempool_alloc0_type (pool, struct html_tag);
				cur_tag->params = g_queue_new ();
				rspamd_mempool_add_destructor (pool,
						(rspamd_mempool_destruct_t)g_queue_free, cur_tag->params);
				break;
			}

			break;

		case sgml_tag:
			switch (t) {
			case '[':
				state = compound_tag;
				obrace = 1;
				ebrace = 0;
				p ++;
				break;
			case '-':
				state = comment_tag;
				p ++;
				break;
			default:
				state = sgml_content;
				break;
			}

			break;

		case xml_tag:
			if (t == '?') {
				state = xml_tag_end;
			}
			else if (t == '>') {
				/* Misformed xml tag */
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				state = tag_end;
				continue;
			}
			/* We efficiently ignore xml tags */
			p ++;
			break;

		case xml_tag_end:
			if (t == '>') {
				state = tag_end;
				continue;
			}
			else {
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				p ++;
			}
			break;

		case compound_tag:
			if (t == '[') {
				obrace ++;
			}
			else if (t == ']') {
				ebrace ++;
			}
			else if (t == '>' && obrace == ebrace) {
				state = tag_end;
				continue;
			}
			p ++;
			break;

		case comment_tag:
			if (t != '-')  {
				hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
				state = tag_end;
			}
			else {
				p++;
				ebrace = 0;
				/*
				 * https://www.w3.org/TR/2012/WD-html5-20120329/syntax.html#syntax-comments
				 *  ... the text must not start with a single
				 *  U+003E GREATER-THAN SIGN character (>),
				 *  nor start with a "-" (U+002D) character followed by
				 *  a U+003E GREATER-THAN SIGN (>) character,
				 *  nor contain two consecutive U+002D HYPHEN-MINUS
				 *  characters (--), nor end with a "-" (U+002D) character.
				 */
				if (p[0] == '-' && p + 1 < end && p[1] == '>') {
					hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
					p ++;
					state = tag_end;
				}
				else if (*p == '>') {
					hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
					state = tag_end;
				}
				else {
					state = comment_content;
				}
			}
			break;

		case comment_content:
			if (t == '-') {
				ebrace ++;
			}
			else if (t == '>' && ebrace >= 2) {
				state = tag_end;
				continue;
			}
			else {
				ebrace = 0;
			}

			p ++;
			break;

		case content_ignore:
			if (t != '<') {
				p ++;
			}
			else {
				state = tag_begin;
			}
			break;

		case content_write:

			if (t != '<') {
				if (t == '&') {
					need_decode = TRUE;
				}
				else if (g_ascii_isspace (t)) {
					save_space = TRUE;

					if (p > c) {
						if (need_decode) {
							goffset old_offset = dest->len;

							if (content_tag) {
								if (content_tag->content_length == 0) {
									content_tag->content_offset = old_offset;
								}
							}

							g_byte_array_append (dest, c, (p - c));

							len = rspamd_html_decode_entitles_inplace (
									reinterpret_cast<gchar *>(dest->data + old_offset),
									p - c);
							dest->len = dest->len + len - (p - c);

							if (content_tag) {
								content_tag->content_length += len;
							}
						}
						else {
							len = p - c;

							if (content_tag) {
								if (content_tag->content_length == 0) {
									content_tag->content_offset = dest->len;
								}

								content_tag->content_length += len;
							}

							g_byte_array_append (dest, c, len);
						}
					}

					c = p;
					state = content_ignore_sp;
				}
				else {
					if (save_space) {
						/* Append one space if needed */
						if (dest->len > 0 &&
								!g_ascii_isspace (dest->data[dest->len - 1])) {
							g_byte_array_append (dest, reinterpret_cast<const guint8 *>(" "), 1);
							if (content_tag) {
								if (content_tag->content_length == 0) {
									/*
									 * Special case
									 * we have a space at the beginning but
									 * we have no set content_offset
									 * so we need to do it here
									 */
									content_tag->content_offset = dest->len;
								}
								else {
									content_tag->content_length++;
								}
							}
						}
						save_space = FALSE;
					}
				}
			}
			else {
				if (c != p) {

					if (need_decode) {
						goffset old_offset = dest->len;

						if (content_tag) {
							if (content_tag->content_length == 0) {
								content_tag->content_offset = dest->len;
							}
						}

						g_byte_array_append (dest, c, (p - c));
						len = rspamd_html_decode_entitles_inplace (
								reinterpret_cast<gchar *>(dest->data + old_offset),
								p - c);
						dest->len = dest->len + len - (p - c);

						if (content_tag) {
							content_tag->content_length += len;
						}
					}
					else {
						len = p - c;

						if (content_tag) {
							if (content_tag->content_length == 0) {
								content_tag->content_offset = dest->len;
							}

							content_tag->content_length += len;
						}

						g_byte_array_append (dest, c, len);
					}
				}

				content_tag = NULL;

				state = tag_begin;
				continue;
			}

			p ++;
			break;

		case content_style: {

			/*
			 * We just search for the first </s substring and then pass
			 * the content to the parser (if needed)
			 */
			goffset end_style = rspamd_substring_search (reinterpret_cast<const gchar *>(p), end - p,
					"</", 2);
			if (end_style == -1 || g_ascii_tolower (p[end_style + 2]) != 's') {
				/* Invalid style */
				state = content_ignore;
			}
			else {

				if (allow_css) {
					GError *err = NULL;
					hc->css_style = rspamd_css_parse_style (pool, p, end_style, hc->css_style,
							&err);

					if (err) {
						msg_info_pool ("cannot parse css: %e", err);
						g_error_free (err);
					}
				}

				p += end_style;
				state = tag_begin;
			}
			break;
		}

		case content_ignore_sp:
			if (!g_ascii_isspace (t)) {
				c = p;
				state = content_write;
				continue;
			}

			p ++;
			break;

		case sgml_content:
			/* TODO: parse DOCTYPE here */
			if (t == '>') {
				state = tag_end;
				/* We don't know a lot about sgml tags, ignore them */
				cur_tag = NULL;
				continue;
			}
			p ++;
			break;

		case tag_content:
			rspamd_html_parse_tag_content (pool, hc, cur_tag,
					p, &substate, &savep);
			if (t == '>') {
				if (closing) {
					cur_tag->flags |= FL_CLOSING;

					if (cur_tag->flags & FL_CLOSED) {
						/* Bad mix of closed and closing */
						hc->flags |= RSPAMD_HTML_FLAG_BAD_ELEMENTS;
					}

					closing = FALSE;
				}

				state = tag_end;
				continue;
			}
			p ++;
			break;

		case tag_end:
			substate = 0;
			savep = NULL;

			if (cur_tag != NULL) {
				balanced = TRUE;

				if (rspamd_html_process_tag (pool, hc, cur_tag, &cur_level,
						&balanced)) {
					state = content_write;
					need_decode = FALSE;
				}
				else {
					if (cur_tag->id == Tag_STYLE) {
						state = content_style;
					}
					else {
						state = content_ignore;
					}
				}

				if (cur_tag->id != -1 && cur_tag->id < N_TAGS) {
					if (cur_tag->flags & CM_UNIQUE) {
						if (isset (hc->tags_seen, cur_tag->id)) {
							/* Duplicate tag has been found */
							hc->flags |= RSPAMD_HTML_FLAG_DUPLICATE_ELEMENTS;
						}
					}
					setbit (hc->tags_seen, cur_tag->id);
				}

				if (!(cur_tag->flags & (FL_CLOSED|FL_CLOSING))) {
					content_tag = cur_tag;
				}

				/* Handle newlines */
				if (cur_tag->id == Tag_BR || cur_tag->id == Tag_HR) {
					if (dest->len > 0 && dest->data[dest->len - 1] != '\n') {
						g_byte_array_append (dest, reinterpret_cast<const guint8 *>("\r\n"), 2);

						if (content_tag) {
							if (content_tag->content_length == 0) {
								/*
								 * Special case
								 * we have a \r\n at the beginning but
								 * we have no set content_offset
								 * so we need to do it here
								 */
								content_tag->content_offset = dest->len;
							}
							else {
								content_tag->content_length += 2;
							}
						}
					}
					save_space = FALSE;
				}

				if ((cur_tag->id == Tag_P ||
						cur_tag->id == Tag_TR ||
						cur_tag->id == Tag_DIV)) {
					if (dest->len > 0 && dest->data[dest->len - 1] != '\n') {
						g_byte_array_append (dest, reinterpret_cast<const guint8 *>("\r\n"), 2);

						if (content_tag) {
							if (content_tag->content_length == 0) {
								/*
								 * Special case
								 * we have a \r\n at the beginning but
								 * we have no set content_offset
								 * so we need to get it here
								 */
								content_tag->content_offset = dest->len;
							}
							else {
								content_tag->content_length += 2;
							}
						}
					}
					save_space = FALSE;
				}

				/* XXX: uncomment when styles parsing is not so broken */
				if (cur_tag->flags & FL_HREF /* && !(cur_tag->flags & FL_IGNORE) */) {
					if (!(cur_tag->flags & (FL_CLOSING))) {
						url = rspamd_html_process_url_tag (pool, cur_tag, hc);

						if (url != NULL) {

							if (url_set != NULL) {
								struct rspamd_url *maybe_existing =
										rspamd_url_set_add_or_return (url_set, url);
								if (maybe_existing == url) {
									rspamd_process_html_url (pool, url, url_set,
											part_urls);
								}
								else {
									url = maybe_existing;
									/* Increase count to avoid odd checks failure */
									url->count ++;
								}
							}

							href_offset = dest->len;
						}
					}

					if (cur_tag->id == Tag_A) {
						if (!balanced && cur_level && cur_level->prev) {
							struct html_tag *prev_tag;
							struct rspamd_url *prev_url;

							prev_tag = static_cast<html_tag *>(cur_level->prev->data);

							if (prev_tag->id == Tag_A &&
									!(prev_tag->flags & (FL_CLOSING)) &&
									prev_tag->extra) {
								prev_url = static_cast<rspamd_url *>(prev_tag->extra);

								rspamd_html_check_displayed_url (pool,
										exceptions, url_set,
										dest, href_offset,
										prev_url);
							}
						}

						if (cur_tag->flags & (FL_CLOSING)) {

							/* Insert exception */
							if (url != NULL && (gint) dest->len > href_offset) {
								rspamd_html_check_displayed_url (pool,
										exceptions, url_set,
										dest, href_offset,
										url);

							}

							href_offset = -1;
							url = NULL;
						}
					}
				}
				else if (cur_tag->id == Tag_BASE && !(cur_tag->flags & (FL_CLOSING))) {
					/*
					 * Base is allowed only within head tag but HTML is retarded
					 */
					if (hc->base_url == NULL) {
						url = rspamd_html_process_url_tag (pool, cur_tag, hc);

						if (url != NULL) {
							msg_debug_html ("got valid base tag");
							hc->base_url = url;
							cur_tag->extra = url;
							cur_tag->flags |= FL_HREF;
						}
						else {
							msg_debug_html ("got invalid base tag!");
						}
					}
				}

				if (cur_tag->id == Tag_IMG && !(cur_tag->flags & FL_CLOSING)) {
					rspamd_html_process_img_tag (pool, cur_tag, hc, url_set,
							part_urls, dest);
				}
				else if (cur_tag->id == Tag_LINK && !(cur_tag->flags & FL_CLOSING)) {
					rspamd_html_process_link_tag (pool, cur_tag, hc, url_set,
							part_urls);
				}
				else if (cur_tag->flags & FL_BLOCK) {
					struct html_block *bl;

					if (cur_tag->flags & FL_CLOSING) {
						/* Just remove block element from the queue if any */
						if (styles_blocks->length > 0) {
							g_queue_pop_tail (styles_blocks);
						}
					}
					else {
						rspamd_html_process_block_tag (pool, cur_tag, hc);
						bl = static_cast<html_block *>(cur_tag->extra);

						if (bl) {
							rspamd_html_propagate_style (hc, cur_tag,
									bl, styles_blocks);

							/* Check visibility */
							if (bl->font_size < 3 ||
								bl->font_color.d.comp.alpha < 10) {

								bl->visible = FALSE;
								msg_debug_html ("tag is not visible: font size: "
												"%d, alpha: %d",
										(int)bl->font_size,
										(int)bl->font_color.d.comp.alpha);
							}

							if (!bl->visible) {
								state = content_ignore;
							}
						}
					}
				}
			}
			else {
				state = content_write;
			}


			p++;
			c = p;
			cur_tag = NULL;
			break;
		}
	}

	if (hc->html_tags) {
		g_node_traverse (hc->html_tags, G_POST_ORDER, G_TRAVERSE_ALL, -1,
				rspamd_html_propagate_lengths, NULL);
	}

	g_queue_free (styles_blocks);
	hc->parsed = dest;

	return dest;
}

GByteArray*
rspamd_html_process_part (rspamd_mempool_t *pool,
		struct html_content *hc,
		GByteArray *in)
{
	return rspamd_html_process_part_full (pool, hc, in, NULL,
			NULL, NULL, FALSE);
}

guint
rspamd_html_decode_entitles_inplace (gchar *s, gsize len)
{
	return rspamd::html::decode_html_entitles_inplace(s, len);
}

gint
rspamd_html_tag_by_name(const gchar *name)
{
	const auto *td = rspamd::html::html_tags_defs.by_name(name);

	if (td != nullptr) {
		return td->id;
	}

	return -1;
}

gboolean
rspamd_html_tag_seen(struct html_content *hc, const gchar *tagname)
{
	gint id;

	g_assert (hc != NULL);
	g_assert (hc->tags_seen != NULL);

	id = rspamd_html_tag_by_name(tagname);

	if (id != -1) {
		return isset(hc->tags_seen, id);
	}

	return FALSE;
}

const gchar *
rspamd_html_tag_by_id(gint id)
{
	const auto *td = rspamd::html::html_tags_defs.by_id(id);

	if (td != nullptr) {
		return td->name.c_str();
	}

	return nullptr;
}