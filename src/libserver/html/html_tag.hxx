/*-
 * Copyright 2021 Vsevolod Stakhov
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

#ifndef RSPAMD_HTML_TAG_HXX
#define RSPAMD_HTML_TAG_HXX
#pragma once

#include <utility>
#include <string_view>
#include <contrib/robin-hood/robin_hood.h>

namespace rspamd::html {

enum class html_component_type : std::uint8_t {
	RSPAMD_HTML_COMPONENT_NAME = 0,
	RSPAMD_HTML_COMPONENT_HREF,
	RSPAMD_HTML_COMPONENT_COLOR,
	RSPAMD_HTML_COMPONENT_BGCOLOR,
	RSPAMD_HTML_COMPONENT_STYLE,
	RSPAMD_HTML_COMPONENT_CLASS,
	RSPAMD_HTML_COMPONENT_WIDTH,
	RSPAMD_HTML_COMPONENT_HEIGHT,
	RSPAMD_HTML_COMPONENT_SIZE,
	RSPAMD_HTML_COMPONENT_REL,
	RSPAMD_HTML_COMPONENT_ALT,
};

struct html_tag {
	gint id;
	gint flags;
	guint content_length;
	goffset content_offset;

	std::string_view name;
	robin_hood::unordered_flat_map<html_component_type, std::string_view> parameters;

	gpointer extra; /* TODO: convert to variant */
	GNode *parent;
};

}

#endif //RSPAMD_HTML_TAG_HXX
