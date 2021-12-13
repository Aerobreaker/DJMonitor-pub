#pragma once

#include "all_header.hpp"

#include <wx/gdicmn.h>
#include <exprtk.hpp>
#include <vector>
#include <string>
#include <DateTime.hpp>

/*
To install exprtk:
	vcpkg install exprtk exprtk:x64-windows exprtk:x86-windows-static exprtk:x64-windows-static
*/

long long to_number(std::string inp);
std::vector<std::string> split_string(const std::string &inp, std::string delims, bool incl_null = false);
std::string to_upper(std::string inp);
datetime::seconds parse_duration(std::string inp);
bool cmp_version(std::string tst, std::string min);
std::string bitmap_calc_long(const std::string &inp);
std::vector<std::string> pieces(std::string data, std::string delim);

enum class quadrant {
	ON_TOP = 0, //00000
	IS_TOP = 1, //00001
	IS_BOT = 2, //00010
	IS_LFT = 4, //00100
	IS_RGT = 8, //01000
	UP_LFT = IS_TOP | IS_LFT, //00101
	UP_RGT = IS_TOP | IS_RGT, //01001
	DN_LFT = IS_BOT | IS_LFT, //00110
	DN_RGT = IS_BOT | IS_RGT, //01010
	INVALID = 16 //10000
};

wxPoint get_center(const wxRect &inp);
quadrant compare_rects(const wxRect &first, const wxRect &second);
