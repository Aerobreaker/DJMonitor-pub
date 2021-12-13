#include "settings.hpp"

#include "window.hpp"
#include "frames.hpp"
#include "globals.hpp"
#include "functions.hpp"

#include <fstream>
#include <sstream>
#include <wx/display.h>
#include <wx/menu.h>

using namespace std;

bool noupdate = false;
bool on_screen(wxRect inp, unsigned char buffer);

// Realistically, all this settings stuff is bad when it comes to adding new settings.
// That said, making a proper plugin structure for settings is just not worth the development effort at this time.
// Mostly because I hadn't considered it as a possibility when initially writing the program and at this time,
// the settings are expected to be fairly (if not completely) static.

void save_settings(MainFrame *frame) {
	if (!noupdate) {
		ofstream fil(settingfile);
		if (fil) {
			bool on_top = frame->m_stay_on_top->IsChecked(), overrides = frame->m_show_overrides->IsChecked();
			bool tz_offs = frame->m_show_tz_offs->IsChecked(), pos = frame->m_save_pos->IsChecked();
			bool tray = frame->m_system_tray->IsChecked(), font = frame->m_custom_font->IsChecked();
			bool bg = frame->m_custom_color->IsChecked(), fmt = frame->m_custom_format->IsChecked();
			bool bmp = frame->m_show_bitmap->IsChecked(), sgl = frame->m_left_click->IsChecked();;
			wxRect framepos = frame->GetRect();
			int date_type = frame->adj_date_type->GetSelection();

			if (current_app->noupdate) {
				fil << "NoUpdate" << endl;
			}
			fil << "version=" << version << endl;
			fil << "StayOnTop=" << on_top << endl;
			fil << "ShowOverrides=" << overrides << endl;
			fil << "ShowTZOffs=" << tz_offs << endl;
			fil << "ShowBitmap=" << bmp << endl;
			fil << "SystemTray=" << tray << endl;
			fil << "SingleClick=" << sgl << endl;
			fil << "SavePosition=" << pos << endl;
			if (pos) fil << "Position=" << framepos.GetLeft() << "," << framepos.GetTop() << "," << framepos.GetWidth() << "x" << framepos.GetHeight() << endl;
			fil << "CustomFont=" << font << endl;
			if (font) {
				fil << "Font=" << frame->GetFont().GetNativeFontInfoDesc() << endl;
				fil << "FontColor=" << frame->GetForegroundColour().GetAsString(wxC2S_CSS_SYNTAX) << endl;
			}
			fil << "CustomColor=" << bg << endl;
			if (bg) fil << "Color=" << frame->GetBackgroundColour().GetAsString(wxC2S_CSS_SYNTAX) << endl;
			fil << "CustomFormat=" << fmt << endl;
			if (fmt) fil << "Format=" << frame->now.format << endl;
			fil << "DateType=" << date_type << endl;
			fil.close();
		}
	}
}

void load_settings(MainFrame *frame) {
	ifstream fil(settingfile);
	bool ontop = false, over = false, tz = false, savepos = false, tray = false, font = false, bg = false, fmt = false;
	bool noupd = false, bmp = false, sgl = false;
	int date_type = 1;
	string format = datetime::h12_format;
	wxRect pos {wxDefaultPosition, wxDefaultSize};
	wxRect default_rect {wxDefaultPosition, wxDefaultSize};
	wxFont newfont = DEFAULT_FONT;
	wxColor font_color = DEFAULT_FONT_COLOR;
	wxColor bg_color = DEFAULT_BG_COLOR;
	if (fil) {
		string token, vers = "0", minvers {version};
		while (!fil.eof()) {
			token.clear();
			while (!fil.eof() && fil.peek() != '=') {
				token.push_back(fil.get());
				if (fil.peek() == '\r' || fil.peek() == '\n') {
					token.clear();
					while (!fil.eof() && (fil.peek() == '\r' || fil.peek() == '\n')) fil.get();
				}
			}
			if (fil.peek() == '=') fil.get();
			if (token == "version") {
				vers.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') vers.push_back(fil.get());
			} else if (token == "StayOnTop") {
				fil >> ontop;
			} else if (token == "ShowOverrides") {
				fil >> over;
			} else if (token == "ShowTZOffs") {
				fil >> tz;
			} else if (token == "SystemTray") {
				fil >> tray;
			} else if (token == "SingleClick") {
				fil >> sgl;
			} else if (token == "SavePosition") {
				fil >> savepos;
			} else if (token == "ShowBitmap") {
				fil >> bmp;
			} else if (token == "Position") {
				token.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') token.push_back(fil.get());
				if (token != "") {
					vector<string> positions = split_string(token, ",x");
					pos.SetLeft(to_number(positions[0]));
					pos.SetTop(to_number(positions[1]));
					pos.SetWidth(to_number(positions[2]));
					pos.SetHeight(to_number(positions[3]));
				}
			} else if (token == "CustomFont") {
				fil >> font;
			} else if (token == "Font") {
				token.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') token.push_back(fil.get());
				if (token != "") newfont = wxFont(token);
			} else if (token == "FontColor") {
				token.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') token.push_back(fil.get());
				if (token != "") font_color.Set(token);
			} else if (token == "CustomColor") {
				fil >> bg;
			} else if (token == "Color") {
				token.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') token.push_back(fil.get());
				if (token != "") bg_color.Set(token);
			} else if (token == "CustomFormat") {
				fil >> fmt;
			} else if (token == "Format") {
				format.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') format.push_back(fil.get());
			} else if (token == "min_version") {
				minvers.clear();
				while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') minvers.push_back(fil.get());
			} else if (token == "DateType") {
				fil >> date_type;
			}
			while (!fil.eof() && fil.peek() != '\r' && fil.peek() != '\n') fil.get();
			while (!fil.eof() && (fil.peek() == '\r' || fil.peek() == '\n')) fil.get();
		}
		fil.close();
		if (!cmp_version(vers, min_version) || !cmp_version(version, minvers)) {
			// If version in file < min version, or current version < min version in file,
			// restore default settings
			// NOTE: The latter condition is to prevent a new settings file from breaking a legacy version of the program
			ontop = false, over = false, tz = false, savepos = false, tray = false, font = false, bg = false, fmt = false;
			noupd = false, bmp = false, sgl = false;
			date_type = 1;
			format = datetime::h12_format;
			pos = wxRect(wxDefaultPosition, wxDefaultSize);
			newfont = DEFAULT_FONT;
			font_color = DEFAULT_FONT_COLOR;
			bg_color = DEFAULT_BG_COLOR;
		}
		if (!cmp_version(version, vers)) {
			// If current version is less than the one in the settings file, don't overwrite settings
			noupdate = true;
		}
	}
	if (!savepos) pos = wxRect(wxDefaultPosition, wxDefaultSize);
	if (savepos && (pos.GetWidth() < 0 || pos.GetHeight() < 0)) pos.SetSize(wxDefaultSize);
	if (savepos && !on_screen(pos, current_app->buffer)) pos = wxRect(wxDefaultPosition, wxDefaultSize);
	frame->m_stay_on_top->Check(ontop);
	frame->OnStayOnTop(menu_evt);
	frame->m_show_overrides->Check(over);
	frame->OnShowOverrides(menu_evt);
	frame->m_show_bitmap->Check(bmp);
	frame->OnShowBitmap(menu_evt);
	frame->m_show_tz_offs->Check(tz);
	frame->m_system_tray->Check(tray);
	frame->OnSystemTray(menu_evt);
	frame->m_left_click->Check(sgl);
	frame->AddLeftClick(menu_evt);
	frame->m_save_pos->Check(savepos);
	if (savepos && pos != default_rect) {
		frame->SetPosition(pos.GetPosition());
		frame->SetSize(pos.GetSize());
	} else {
		frame->Fit();
		frame->Center(wxBOTH);
	}
	frame->m_custom_font->Check(font);
	if (font) {
		size_t pos {};
		wxMenu *font_menu = frame->m_custom_font->GetMenu();
		font_menu->FindChildItem(ID_CustomFont, &pos);
		font_menu->Insert(pos + 1, frame->m_custom_font_col);
		if (font_color != DEFAULT_FONT_COLOR) frame->m_custom_font_col->Check(true);
	} else {
		newfont = DEFAULT_FONT;
		font_color = DEFAULT_FONT_COLOR;
	}
	frame->SetFont(newfont);
	frame->SetForegroundColour(font_color);
	frame->m_custom_color->Check(bg);
	if (!bg) bg_color = DEFAULT_BG_COLOR;
	frame->SetBackgroundColour(bg_color);
	frame->m_custom_format->Check(fmt);
	if (!fmt) format = datetime::h12_format;
	frame->now.format = format;
	frame->adj.format = format;
	frame->adj_date_type->SetSelection(date_type);
	frame->UpdateStyle();
}

bool on_screen(wxRect inp, unsigned char buffer) {
	unsigned dispcnt = wxDisplay::GetCount();
	wxRect disparea;
	for (unsigned i = 0; i < dispcnt; i++) {
		wxRect curr_disp_area = wxDisplay(i).GetClientArea();
		curr_disp_area.x -= buffer;
		curr_disp_area.y -= buffer;
		curr_disp_area.width += buffer * 2;
		curr_disp_area.height += buffer * 2;
		disparea.Union(curr_disp_area);
	}
	return disparea.Contains(inp);
}
