#include "window.hpp"

#include "functions.hpp"
#include "sites.hpp"
#include "globals.hpp"
#include "settings.hpp"
#include "resource.h"
#include "extfuncs.hpp"
#include "pieceman.hpp"

#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/fontdlg.h>
#include <wx/colordlg.h>
#include <wx/display.h>
#include <wx/wupdlock.h>
#include <wx/filedlg.h>
#include <vector>
#include <string>

/*META-COMMENT:
* A number of field names, descriptions, and labels have been changed throughout to protect potentially proprietary information
*/

using namespace datetime;
using namespace std;

string CURRENT_ZONE_NAME;
const time_zone *CURRENT_ZONE;
wxCommandEvent menu_evt = wxCommandEvent();

void MainFrame::UpdateDisplay() {
	adj.Set(now);
	curr_date_time->UpdateValue(now.to_string(), control_panel);
	if (force_date != nullptr) {
		adj.SetDate(*force_date);
	}
	if (force_time != nullptr) adj.SetTime(*force_time);
	if (offset != null_duration) adj += offset;
	if (adj == now) {
		if (row_1_sizer->IsShown(adj_date_time_sizer)) {
			row_1_sizer->Hide(adj_date_time_sizer);
			ReSize();
		}
	} else {
		if (!(row_1_sizer->IsShown(adj_date_time_sizer))) row_1_sizer->Show(adj_date_time_sizer);
		if (stale && (time_zone->GetValue() != adj.get_timezone_name())) time_zone->SetValue(adj.get_timezone_name());
		string adjstr = adj.to_string();
		string offstr = adj.get_offset_from(now, m_show_tz_offs->IsChecked());
		if (offstr != "") adjstr += " (" + offstr + ")";
		adj_date_time->UpdateValue(adjstr, control_panel);
		ReSize();
	}
	h_time->UpdateValue(to_string(adj.GetTimeOfDay().to_duration().count()), control_panel);
	julian_month_1->UpdateValue(to_string(adj.get_difference<months>(sq_mj_start).count()), control_panel);
	julian_month_2->UpdateValue(to_string(adj.get_difference<months>(sq_mj_2_start).count()), control_panel);
	julian_date->UpdateValue(to_string(adj.get_difference<days>(sq_julian_date_start).count()), control_panel);
	h_date->UpdateValue(to_string(adj.get_difference<days>(h_julian_date_start).count()), control_panel);
}

void MainFrame::OnTimer(wxTimerEvent &evt) {
	// Don't update the display until all of the values have been updated
	// In release mode, this isn't necessary (but doesn't hurt anything)
	// But in debug mode, string ops are SSSSSLLLLLOOOOOWWWWW.  Without this,
	// there's a noticable delay while the window is wiped and re-filled

	wxWindowUpdateLocker no_updates {this};
	now.Set(system_clock::now());
	UpdateDisplay();
	stale = false;
}

void MainFrame::OnUpdateAdjDateType(wxEvent &evt) {
	wxWindowUpdateLocker no_updates {this};
	string newlst, curval = adj_date->GetValue().ToStdString();
	switch (curval[0]) {
	case 't':
	case 'T':
		if (curval[1] != '+' && curval[1] != '-') break;
	case '+':
	case '-':
		newlst = curval;
	}
	adj_date->SetLast(newlst);
	OnUpdateAdjDate(evt);
}

void MainFrame::OnUpdateAdjDate(wxEvent &evt) {
	wxWindowUpdateLocker no_updates {this};
	string adj_date_text = adj_date->GetValue().ToStdString(), last_date_text = adj_date->GetLast().ToStdString();
	days old_offs = days(0);
	if (last_date_text != "T" && last_date_text != "t") {
		if (toupper(last_date_text[0]) == 'T' && (last_date_text[1] == '+' || last_date_text[1] == '-')) {
			last_date_text = last_date_text.substr(1);
		}
		if (last_date_text[0] == '+' || last_date_text[0] == '-') {
			old_offs = days(to_number(last_date_text));
			offset -= old_offs;
		}
	}
	sys_days *old_force_date = force_date;
	delete force_date;
	force_date = nullptr;
	if (adj_date_text != "" && adj_date_text != "T" && adj_date_text != "t") {
		if (toupper(adj_date_text[0]) == 'T' && (adj_date_text[1] == '+' || adj_date_text[1] == '-')) {
			adj_date_text = adj_date_text.substr(1);
		}
		if (adj_date_text[0] == '+' || adj_date_text[0] == '-') {
			offset += days(to_number(adj_date_text));
		} else {
			int date_type = adj_date_type->GetSelection();
			if (date_type == 0) {
				force_date = new sys_days(smart_parse_date(adj_date_text));
				if (*force_date == null_date) {
					adj_date->RevertValue();
					offset += old_offs;
					force_date = old_force_date;
				}
			} else {
				string num;
				for (const char &it : adj_date_text) {
					switch (it) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					case '*':
					case '/':
					case '-':
					case '+':
						num.push_back(it);
					}
				}
				if (num != "") {
					force_date = new sys_days((date_type == 1 ? sq_julian_date_start : h_julian_date_start) + days(to_number(num)));
				}
			}
		}
	}
	UpdateDisplay();
	if (!evt.IsCommandEvent()) evt.Skip();
	adj_date->SetLast(adj_date->GetValue());
}

void MainFrame::OnUpdateAdjTime(wxEvent &evt) {
	wxWindowUpdateLocker no_updates {this};
	string adj_time_text = adj_time->GetValue().ToStdString(), last_time_text = adj_time->GetLast().ToStdString();
	if (last_time_text != "T" && last_time_text != "t") {
		if (toupper(last_time_text[0]) == 'T' && (last_time_text[1] == '+' || last_time_text[1] == '-')) {
			last_time_text = last_time_text.substr(1);
		}
		if (last_time_text[0] == '+' || last_time_text[0] == '-') {
			offset -= parse_duration(last_time_text);
		}
	}
	delete force_time;
	force_time = nullptr;
	if (adj_time_text != "" && adj_time_text != "T" && adj_time_text != "t") {
		if (toupper(adj_time_text[0]) == 'T' && (adj_time_text[1] == '+' || adj_time_text[1] == '-')) {
			adj_time_text = adj_time_text.substr(1);
		}
		if (adj_time_text[0] == '+' || adj_time_text[0] == '-') {
			offset += parse_duration(adj_time_text);
		} else {
			string check_time;
			for (const char &it : adj_time_text) {
				switch (it) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					check_time.push_back(it);
				}
			}
			if (check_time == adj_time_text && to_number(check_time) >= 60) {
				force_time = new system_duration(smart_parse_time("::" + adj_time_text));
			} else {
				force_time = new system_duration(smart_parse_time(adj_time_text));
			}
		}
	}
	UpdateDisplay();
	if (!evt.IsCommandEvent()) evt.Skip();
	adj_time->SetLast(adj_time->GetValue());
}

void MainFrame::OnUpdateTZ(wxEvent &evt) {
	wxWindowUpdateLocker no_updates {this};
	string new_tz = time_zone->GetValue().ToStdString(), tz = adj.get_timezone_name();
	if (new_tz != tz) {
		new_tz = to_upper(new_tz);
		if (new_tz == "") {
			adj.set_timezone(CURRENT_ZONE);
			time_zone->ChangeValue(CURRENT_ZONE_NAME);
		} else {
			if (site_map->contains(new_tz)) {
				new_tz = site_map->at(new_tz);
				if (new_tz == tz) {
					time_zone->ChangeValue(tz);
					evt.Skip();
					return;
				}
			}
			try {
				adj.set_timezone(new_tz);
				time_zone->SetValue(new_tz);
				stale = true;
			} catch (runtime_error) {
				time_zone->ChangeValue(tz);
			}
		}
		if (force_time != nullptr) {
			// If difference b/w offsets puts force time <0 or >23:59:59, check for force date
			// If force date, adjust by 1.  If not, add +1 or -1 to date adj
			// If difference b/w offsets doesn't put force time out of range, adjust and update

			constexpr seconds endofday = hours(23) + minutes(59) + seconds(59);
			seconds newtime = date::floor<seconds>(*force_time) + adj.GetZonedTime().get_info().offset - get_zone(tz).second->get_info((system_time_point)adj).offset;
			if (newtime < null_duration || newtime > endofday) {
				char adjust = 0;
				if (newtime < null_duration) {
					adjust = -1;
				} else {
					adjust = 1;
				}
				if (force_date == nullptr) {
					string dayoff = adj_date->GetValue().ToStdString();
					if (dayoff != "") {
						if (toupper(dayoff[0] == 'T')) {
							dayoff = dayoff.substr(1);
						}
						long long days = to_number(dayoff) + adjust;
						if (days == 0) {
							adj_date->SetValue("");
						} else if (days > 0) {
							adj_date->SetValue("+" + to_string(days));
						} else {
							adj_date->SetValue(to_string(days));
						}
					} else {
						adj_date->SetValue(adjust == 1 ? "+" + to_string(adjust) : to_string(adjust));
					}
				} else {
					adj_date->SetValue(date::format("%d %b %Y", *force_date + days(adjust)));
				}
				OnUpdateAdjDate(evt);
				newtime -= endofday * adjust + seconds(adjust);
			}
			if (now.format.size() > 1 && now.format.substr(now.format.size() - 2) == "%p") {
				adj_time->SetValue(date::format("%I:%M:%S %p", newtime));
			} else {
				adj_time->SetValue(date::format("%H:%M:%S", newtime));
			}
			OnUpdateAdjTime(evt);
		}
		UpdateDisplay();
	}
	time_zone->SetInsertionPoint(0);
	if (!evt.IsCommandEvent()) evt.Skip();
	Refresh();
}

void MainFrame::SetOnTop(bool raise) {
	long cur_style = GetWindowStyle(), new_style = 0;
	if (m_stay_on_top->IsChecked()) {
		new_style = cur_style | wxSTAY_ON_TOP;
		if (raise) {
			for (const wxFrame *const &frm : app_frames) {
				if (frm->GetWindowStyle() & wxSTAY_ON_TOP) {
					Raise();
					break;
				}
			}
		}
	} else {
		new_style = cur_style & ~wxSTAY_ON_TOP;
	}
	SetWindowStyle(new_style);
}

void MainFrame::OnStayOnTop(wxCommandEvent &evt) {
	SetOnTop(true);
	evt.Skip();
}

void MainFrame::OnGetFocus(wxFocusEvent &evt) {
	for (const wxFrame *const &frm : app_frames) {
		if (frm == this) continue;
		if (frm->GetWindowStyle() & wxSTAY_ON_TOP) {
			SetWindowStyle(GetWindowStyle() | wxSTAY_ON_TOP);
			Raise();
			break;
		}
	}
	evt.Skip();
}

void MainFrame::OnLoseFocus(wxFocusEvent &evt) {
	SetOnTop(false);
	evt.Skip();
}

void MainFrame::OnShowOverrides(wxCommandEvent &evt) {
	if (m_show_overrides->IsChecked()) {
		row_2_sizer->Show(adj_date_panel);
		row_3_sizer->Show(adj_date_type_panel);
		row_4_sizer->Show(adj_time_panel);
	} else {
		row_2_sizer->Hide(adj_date_panel);
		row_3_sizer->Hide(adj_date_type_panel);
		row_4_sizer->Hide(adj_time_panel);
	}
	ReSize();
	evt.Skip();
}

void MainFrame::OnSystemTray(wxCommandEvent &evt) {
	wxMenu *tray_menu = m_system_tray->GetMenu();
	if (m_system_tray->IsChecked()) {
		tb_icon->SetIcon(wxICON(IDI_ICON1), "DJMonitor");
		size_t pos {};
		tray_menu->FindChildItem(ID_SystemTray, &pos);
		tray_menu->Insert(pos + 1, m_left_click);;
	} else {
		tb_icon->RemoveIcon();
		tray_menu->Remove(m_left_click);
	}
	evt.Skip();
}

void MainFrame::AddLeftClick(wxCommandEvent &evt) {
	tb_icon->lclick = m_left_click->IsChecked();
	evt.Skip();
}

void MainFrame::OnCustomFont(wxCommandEvent &evt) {
	wxMenu *font_menu = m_custom_font->GetMenu();
	wxFontData data;
	data.SetColour(GetForegroundColour());
	data.SetInitialFont(GetFont());
	wxFontDialog child(this, data);
	if (child.ShowModal() != wxID_OK) {
		data.SetChosenFont(DEFAULT_FONT);
		data.SetColour(DEFAULT_FONT_COLOR);
	} else {
		data = child.GetFontData();
	}
	if (data.GetChosenFont() == DEFAULT_FONT && data.GetColour() == DEFAULT_FONT_COLOR) {
		m_custom_font->Check(false);
		m_custom_font_col->Check(false);
		if (font_menu->FindItem(ID_Custom_FontCol)) font_menu->Remove(m_custom_font_col);
	} else {
		m_custom_font->Check(true);
		if (!font_menu->FindItem(ID_Custom_FontCol)) {
			size_t pos {};
			font_menu->FindChildItem(ID_CustomFont, &pos);
			font_menu->Insert(pos + 1, m_custom_font_col);
		}
	}
	SetFont(data.GetChosenFont());
	SetForegroundColour(data.GetColour());
	UpdateStyle();
	evt.Skip();
}

void MainFrame::OnCustomColor(wxCommandEvent &evt) {
	wxColourData col;
	col.SetColour(GetBackgroundColour());
	wxColourDialog *child = new wxColourDialog(this, &col);
	if (child->ShowModal() != wxID_OK) {
		col.SetColour(DEFAULT_BG_COLOR);
	} else {
		col = child->GetColourData();
	}
	if (col.GetColour() == DEFAULT_BG_COLOR) {
		m_custom_color->Check(false);
	} else {
		m_custom_color->Check(true);
	}
	SetBackgroundColour(col.GetColour());
	UpdateStyle();
	evt.Skip();
}

void MainFrame::OnCustomFontCol(wxCommandEvent &evt) {
	wxColourData col;
	col.SetColour(GetForegroundColour());
	wxColourDialog *child = new wxColourDialog(this, &col);
	if (child->ShowModal() != wxID_OK) {
		col.SetColour(DEFAULT_FONT_COLOR);
	} else {
		col = child->GetColourData();
	}
	if (col.GetColour() == DEFAULT_FONT_COLOR) {
		m_custom_font_col->Check(false);
	} else {
		m_custom_font_col->Check(true);
	}
	SetForegroundColour(col.GetColour());
	UpdateStyle();
	evt.Skip();
}

void MainFrame::OnCustomFormat(wxCommandEvent &evt) {
	wxDialog *child = new wxDialog(this, wxID_ANY, "Custom date/time format");
	child->SetFont(GetFont());
	child->SetForegroundColour(GetForegroundColour());
	child->SetBackgroundColour(GetBackgroundColour());

	wxBoxSizer *child_mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *child_row1_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *child_row2_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *child_row3_sizer = new wxBoxSizer(wxHORIZONTAL);

	Main_StaticText *h12_label = new Main_StaticText(child, wxID_ANY, "Default 12-hour format:");
	Main_StaticText *h24_label = new Main_StaticText(child, wxID_ANY, "Default 24-hour format:");
	Main_StaticText *format_label = new Main_StaticText(child, wxID_ANY, "Date/time format:");

	Invisible_TextCtrl *h12 = new Invisible_TextCtrl(child, wxID_ANY);
	h12->SetFont(GetFont(), false);
	Invisible_TextCtrl *h24 = new Invisible_TextCtrl(child, wxID_ANY);
	h24->SetFont(GetFont(), false);
	Main_TextCtrl *format = new Main_TextCtrl(child, wxID_ANY, now.format, wxDefaultPosition, wxDefaultSize, wxTE_CENTER);

	h12_label->SetMinSize(h12_label->GetSize());
	h24_label->SetMinSize(h24_label->GetSize());
	format_label->SetMinSize(format_label->GetSize());

	h12->UpdateValue(h12_format);
	h24->UpdateValue(h24_format);
	format->SetInsertionPoint(0);

	child_row1_sizer->Add(h12_label, 0, wxLEFT | wxRIGHT | wxEXPAND, default_border);
	child_row1_sizer->Add(h12, 1, wxLEFT | wxRIGHT | wxEXPAND, default_border);

	child_row2_sizer->Add(h24_label, 0, wxLEFT | wxRIGHT | wxEXPAND, default_border);
	child_row2_sizer->Add(h24, 1, wxLEFT | wxRIGHT | wxEXPAND, default_border);

	child_row3_sizer->Add(format_label, 0, wxALL | wxEXPAND, default_border);
	child_row3_sizer->Add(format, 1, wxLEFT | wxRIGHT | wxEXPAND, default_border);

	child_mainSizer->Add(child_row1_sizer, 1, wxEXPAND);
	child_mainSizer->Add(child_row2_sizer, 1, wxEXPAND);
	child_mainSizer->Add(child_row3_sizer, 1, wxEXPAND);
	child_mainSizer->Add(child->CreateStdDialogButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND);
	child->SetSizer(child_mainSizer);
	child->Fit();

	if (child->ShowModal() == wxID_OK) {
		m_custom_format->Check(true);
		now.format = format->GetValue().ToStdString();
		adj.format = now.format;
	} else {
		m_custom_format->Check(false);
		now.format = h12_format;
		adj.format = h12_format;
	}
	delete child;
	UpdateDisplay();
	ReSize();
	evt.Skip();
}

void MainFrame::UpdateSiteDict(wxCommandEvent &evt) {
	wxFileDialog *child = new wxFileDialog(this, "Select updated site code dictionary:", filesystem::path(current_app->updatepath.ToStdString()).parent_path().string(), wxEmptyString, "*.txt", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
	if (child->ShowModal() == wxID_OK) {
		error_code ec;
		if (filesystem::copy_file(child->GetPath().ToStdString(), settingdir + "\\sites.txt", filesystem::copy_options::overwrite_existing, ec)) {
			time_zone->Clear();
			site_map->clear();
			time_zone->Set(get_site_list());
		} else {
			wxMessageDialog dlg = wxMessageDialog(this, "Unable to update site code dictionary!\n \n" + ec.message(), "Unable to update", wxOK | wxICON_ERROR | wxCENTER);
			dlg.ShowModal();
		}
	}
}

void MainFrame::CalcBitmap(wxEvent &evt) {
	string inp = bitmap->GetValue().ToStdString();
	if (!inp.empty()) {
		try {
			bits->SetValue(bitmap_calc_long(bitmap->GetValue().ToStdString()));
		} catch (...) {
			bits->SetValue(wxEmptyString);
		}
	} else {
		bits->SetValue(wxEmptyString);
	}
	evt.Skip();
}

void MainFrame::OnShowBitmap(wxCommandEvent &evt) {
	if (m_show_bitmap->IsChecked()) {
		main_sizer->Show(row_5_sizer);
	} else {
		main_sizer->Hide(row_5_sizer);
	}
	ReSize();
	evt.Skip();
}

MainFrame::MainFrame(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxFrame(parent, id, title, pos, size, style) {
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	SetIcon(wxICON(IDI_ICON1));

	/*Menu accelerators:
	* F - file
	* E - extras
	* H - help
	* S - settings
	*/
	wxMenuBar *main_menu = new wxMenuBar(0);
	/*Menu accelerators:
	* S - stay on top
	* H - show overrides
	* I - tz diff in offs
	* B - show bitmap calc
	* U - update site dict
	*/
	wxMenu *file_menu = new wxMenu();
	m_stay_on_top = new wxMenuItem(file_menu, ID_OnTop, "&Stay on top", wxEmptyString, wxITEM_CHECK);
	m_show_overrides = new wxMenuItem(file_menu, ID_Overrides, "S&how overrides", wxEmptyString, wxITEM_CHECK);
	m_show_tz_offs = new wxMenuItem(file_menu, ID_TZOffs, "&Include timezone difference in offset", wxEmptyString, wxITEM_CHECK);
	m_show_bitmap = new wxMenuItem(file_menu, ID_Show_Bitmap, "Show &bitmap calculator", wxEmptyString, wxITEM_CHECK);
	wxMenuItem *m_update_sites = new wxMenuItem(file_menu, ID_UpdateSiteDict, "&Update site code dictionary");
	wxMenuItem *m_quit = new wxMenuItem(file_menu, wxID_EXIT);
	file_menu->Append(m_stay_on_top);
	file_menu->Append(m_show_overrides);
	file_menu->Append(m_show_bitmap);
	file_menu->Append(m_show_tz_offs);
	file_menu->Append(m_update_sites);
	file_menu->Append(m_quit);
	main_menu->Append(file_menu, "&File");

	/*Menu accelerators:
	* A - save pos
	* M - system tray
	* R - sgl click
	* F - custom font
	* C - custom bg col
	* D - custom date/time fmt
	* O - custom font col
	*/
	wxMenu *setting_menu = new wxMenu();
	m_save_pos = new wxMenuItem(setting_menu, ID_SavePos, "Sa&ve position", wxEmptyString, wxITEM_CHECK);
	m_system_tray = new wxMenuItem(setting_menu, ID_SystemTray, "&Minimize to system tray", wxEmptyString, wxITEM_CHECK);
	m_left_click = new wxMenuItem(setting_menu, ID_LeftClick, "&Register single click", wxEmptyString, wxITEM_CHECK);
	m_custom_font = new wxMenuItem(setting_menu, ID_CustomFont, "Custom &font", wxEmptyString, wxITEM_CHECK);
	m_custom_font_col = new wxMenuItem(setting_menu, ID_Custom_FontCol, "Custom f&ont color", wxEmptyString, wxITEM_CHECK);
	m_custom_color = new wxMenuItem(setting_menu, ID_CustomColor, "Custom &color", wxEmptyString, wxITEM_CHECK);
	m_custom_format = new wxMenuItem(setting_menu, ID_CustomFormat, "Custom &date/time format", wxEmptyString, wxITEM_CHECK);
	setting_menu->Append(m_save_pos);
	setting_menu->Append(m_system_tray);
	setting_menu->Append(m_custom_font);
	setting_menu->Append(m_custom_color);
	setting_menu->Append(m_custom_format);
	main_menu->Append(setting_menu, "&Settings");

	/*Menu accelerators:
	* S - show extra
	* C - close extra
	* H - show piece
	* L - close piece
	*/
	wxMenu *extras_menu = new wxMenu();
	wxMenuItem *show_extras = new wxMenuItem(extras_menu, ID_ShowExtras, "&Show extra functions");
	wxMenuItem *close_extras = new wxMenuItem(extras_menu, ID_CloseExtras, "&Close extra functions");
	wxMenuItem *show_pieces = new wxMenuItem(extras_menu, ID_ShowPieces, "S&how piece manager");
	wxMenuItem *close_pieces = new wxMenuItem(extras_menu, ID_ClosePieces, "C&lose piece manager");
	extras_menu->Append(show_extras);
	extras_menu->Append(close_extras);
	extras_menu->AppendSeparator();
	extras_menu->Append(show_pieces);
	extras_menu->Append(close_pieces);
	main_menu->Append(extras_menu, "&Extra functions");

	/*Menu accelerators:
	* O - overrides submenu
	* S - stay on top
	* H - show overrides
	* B - show bmp calc
	* T - bmp calc
	* I - tz diff in offs
	* V - save pos
	* M - system tray
	* R - sgl click
	* F - custom font
	* C - custom bg col
	* D - custom date/time fmt
	* U - update site code dict
	* N - custom font col
	*/
	wxMenu *help_menu = new wxMenu();
	/*Menu accelerators:
	* D - adj date
	* Y - adj date type
	* T - adj time
	*/
	wxMenu *help_menu_overrides = new wxMenu();
	wxMenuItem *help_menu_overridesItem = new wxMenuItem(help_menu, wxID_ANY, "&Overrides", wxEmptyString, wxITEM_NORMAL, help_menu_overrides);
	wxMenuItem *m_menuItem1 = new wxMenuItem(help_menu, ID_Help_OnTop, "&Stay on top");
	wxMenuItem *m_menuItem2 = new wxMenuItem(help_menu, ID_Help_Overrides, "S&how overrides");
	wxMenuItem *m_menuItem3 = new wxMenuItem(help_menu_overrides, ID_Help_Override_Date, "Adjust &date");
	wxMenuItem *m_menuItem4 = new wxMenuItem(help_menu_overrides, ID_Help_Override_Date_Type, "Date t&ype");
	wxMenuItem *m_menuItem5 = new wxMenuItem(help_menu_overrides, ID_Help_Override_Time, "Adjust &time");
	wxMenuItem *m_menuItem6 = new wxMenuItem(help_menu, ID_Help_Show_Bitmap, "Show &bitmap calculator");
	wxMenuItem *m_menuItem7 = new wxMenuItem(help_menu, ID_Help_Small_Bitmap, "Bi&tmap calculator");
	wxMenuItem *m_menuItem8 = new wxMenuItem(help_menu, ID_Help_TZOffs, "&Include timezone difference in offset");
	wxMenuItem *m_menuItem9 = new wxMenuItem(help_menu, ID_Help_SavePos, "Sa&ve position");
	wxMenuItem *m_menuItem10 = new wxMenuItem(help_menu, ID_Help_SystemTray, "&Minimize to system tray");
	wxMenuItem *m_menuItem11 = new wxMenuItem(help_menu, ID_Help_LeftClick, "&Register single click");
	wxMenuItem *m_menuItem12 = new wxMenuItem(help_menu, ID_Help_CustomFont, "Custom &font");
	wxMenuItem *m_menuItem13 = new wxMenuItem(help_menu, ID_Help_CustomColor, "Custom &color");
	wxMenuItem *m_menuItem14 = new wxMenuItem(help_menu, ID_Help_CustomFormat, "Custom &date/time format");
	wxMenuItem *m_menuItem15 = new wxMenuItem(help_menu, ID_Help_UpdateSiteDict, "&Update site code dictionary");
	wxMenuItem *m_menuItem16 = new wxMenuItem(help_menu, wxID_ABOUT);
	wxMenuItem *m_menuItem17 = new wxMenuItem(help_menu, ID_Help_Custom_FontCol, "Custom fo&nt color");
	help_menu->Append(m_menuItem1);
	help_menu->Append(m_menuItem2);
	help_menu_overrides->Append(m_menuItem3);
	help_menu_overrides->Append(m_menuItem4);
	help_menu_overrides->Append(m_menuItem5);
	help_menu->Append(help_menu_overridesItem);
	help_menu->Append(m_menuItem6);
	help_menu->Append(m_menuItem7);
	help_menu->Append(m_menuItem8);
	help_menu->Append(m_menuItem9);
	help_menu->Append(m_menuItem10);
	help_menu->Append(m_menuItem11);
	help_menu->Append(m_menuItem12);
	help_menu->Append(m_menuItem17);
	help_menu->Append(m_menuItem13);
	help_menu->Append(m_menuItem14);
	help_menu->Append(m_menuItem15);
	help_menu->Append(m_menuItem16);

	main_menu->Append(help_menu, "&Help");
	SetMenuBar(main_menu);

	main_panel_sizer = new wxBoxSizer(wxVERTICAL);
	control_panel = new Main_StaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_READONLY | wxBORDER_NONE);
	main_panel_sizer->Add(control_panel);
	main_panel_sizer->Hide(control_panel);
	main_panel = new Invisible_Panel(this);
	main_sizer = new wxBoxSizer(wxVERTICAL);

	row_1_sizer = new wxBoxSizer(wxVERTICAL);
	curr_date_time_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Current date and time"), wxVERTICAL);
	curr_date_time = new Invisible_TextCtrl(main_panel, wxID_ANY, "Loading time zone information");
	curr_date_time->SetFont(GetFont(), false);
	curr_date_time_sizer->Add(curr_date_time, 1, wxEXPAND, default_border);
	row_1_sizer->Add(curr_date_time_sizer, 1, wxEXPAND, default_border);
	adj_date_time_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Adjusted date and time"), wxVERTICAL);
	adj_date_time = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	adj_date_time->SetFont(GetFont(), false);
	adj_date_time_sizer->Add(adj_date_time, 1, wxEXPAND, default_border);
	row_1_sizer->Add(adj_date_time_sizer, 1, wxEXPAND, default_border);

	main_sizer->Add(row_1_sizer, 0, wxEXPAND, default_border);

	row_2_sizer = new wxBoxSizer(wxHORIZONTAL);
	julian_date_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Julian Date"), wxVERTICAL);
	julian_date = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	julian_date_sizer->Add(julian_date, 1, wxEXPAND, default_border);
	row_2_sizer->Add(julian_date_sizer, 1, wxEXPAND, default_border);
	tz_panel = new Invisible_Panel(main_panel);
	tz_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(tz_panel, wxID_ANY, "Time Zone"), wxVERTICAL);
	time_zone = new wxComboBox(tz_panel, wxID_ANY, "Please wait", wxDefaultPosition, wxDefaultSize, 0, NULL, wxTE_PROCESS_ENTER);
	time_zone->Set(get_site_list());
	time_zone->SetMinSize(DEFAULT_DROPBOX_SIZE);
	tz_sizer->Add(time_zone, 1, wxEXPAND, default_border);
	tz_panel->SetSizer(tz_sizer);
	row_2_sizer->Add(tz_panel, 1, wxEXPAND, default_border);
	adj_date_panel = new Invisible_Panel(main_panel);
	adj_date_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(adj_date_panel, wxID_ANY, "Adjust Date"), wxVERTICAL);
	adj_date = new Main_TextCtrl(adj_date_panel, wxID_ANY);
	adj_date_sizer->Add(adj_date, 1, wxEXPAND, default_border);
	adj_date_panel->SetSizer(adj_date_sizer);
	row_2_sizer->Add(adj_date_panel, 1, wxEXPAND, default_border);

	main_sizer->Add(row_2_sizer, 1, wxEXPAND, default_border);

	row_3_sizer = new wxBoxSizer(wxHORIZONTAL);
	julian_month_1_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Julian Month 1"), wxVERTICAL);
	julian_month_1 = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	julian_month_1_sizer->Add(julian_month_1, 1, wxEXPAND, default_border);
	row_3_sizer->Add(julian_month_1_sizer, 1, wxEXPAND, default_border);
	h_time_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "$H Time"), wxVERTICAL);
	h_time = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	h_time_sizer->Add(h_time, 1, wxEXPAND, default_border);
	row_3_sizer->Add(h_time_sizer, 1, wxEXPAND, default_border);
	adj_date_type_panel = new Invisible_Panel(main_panel);
	adj_date_type_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(adj_date_type_panel, wxID_ANY, "Date Type"), wxVERTICAL);
	vector<wxString> adj_date_typeChoices {"Date","Julian Date","$H Date"};
	adj_date_type = new wxChoice(adj_date_type_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, NULL, 0);
	adj_date_type->Set(adj_date_typeChoices);
	adj_date_type->SetMinSize(DEFAULT_DROPBOX_SIZE);
	adj_date_type_sizer->Add(adj_date_type, 1, wxEXPAND, default_border);
	adj_date_type_panel->SetSizer(adj_date_type_sizer);
	row_3_sizer->Add(adj_date_type_panel, 1, wxEXPAND, default_border);

	main_sizer->Add(row_3_sizer, 1, wxEXPAND, default_border);

	row_4_sizer = new wxBoxSizer(wxHORIZONTAL);
	julian_month_2_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Julian Month 2"), wxVERTICAL);
	julian_month_2 = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	julian_month_2_sizer->Add(julian_month_2, 1, wxEXPAND, default_border);
	row_4_sizer->Add(julian_month_2_sizer, 1, wxEXPAND, default_border);
	h_date_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "$H Date"), wxVERTICAL);
	h_date = new Invisible_TextCtrl(main_panel, wxID_ANY, "Please wait");
	h_date_sizer->Add(h_date, 1, wxEXPAND, default_border);
	row_4_sizer->Add(h_date_sizer, 1, wxEXPAND, default_border);
	adj_time_panel = new Invisible_Panel(main_panel);
	adj_time_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(adj_time_panel, wxID_ANY, "Adjust Time"), wxVERTICAL);
	adj_time = new Main_TextCtrl(adj_time_panel, wxID_ANY);
	adj_time_sizer->Add(adj_time, 1, wxEXPAND, default_border);
	adj_time_panel->SetSizer(adj_time_sizer);
	row_4_sizer->Add(adj_time_panel, 1, wxEXPAND, default_border);

	main_sizer->Add(row_4_sizer, 1, wxEXPAND, default_border);

	row_5_sizer = new wxBoxSizer(wxHORIZONTAL);
	bitmap_panel = new Invisible_Panel(main_panel);
	bitmap_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(bitmap_panel, wxID_ANY, "Bitmap"), wxVERTICAL);
	bitmap = new Main_TextCtrl(bitmap_panel, wxID_ANY);
	bitmap_sizer->Add(bitmap, 1, wxEXPAND, default_border);
	bitmap_panel->SetSizer(bitmap_sizer);
	row_5_sizer->Add(bitmap_panel, 1, wxEXPAND, default_border);
	bit_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(main_panel, wxID_ANY, "Bits"), wxVERTICAL);
	bits = new Invisible_TextCtrl(main_panel, wxID_ANY, "");
	bit_sizer->Add(bits, 1, wxEXPAND, default_border);
	row_5_sizer->Add(bit_sizer, 1, wxEXPAND, default_border);

	main_sizer->Add(row_5_sizer, 1, wxEXPAND, default_border);

	main_panel->SetSizer(main_sizer);
	main_panel_sizer->Add(main_panel, 1, wxEXPAND, default_border);
	SetSizer(main_panel_sizer);

	tb_icon = new SystemTray_Icon(this);

	Bind(wxEVT_TIMER, &MainFrame::OnTimer, this);

	Bind(wxEVT_MENU, &MainFrame::ShowExtraFunctions, this, ID_ShowExtras);
	Bind(wxEVT_MENU, &MainFrame::CloseExtraFunctions, this, ID_CloseExtras);
	Bind(wxEVT_MENU, &MainFrame::ShowPieceMan, this, ID_ShowPieces);
	Bind(wxEVT_MENU, &MainFrame::ClosePieceMan, this, ID_ClosePieces);

	Bind(wxEVT_MENU, &MainFrame::OnStayOnTop, this, ID_OnTop);
	Bind(wxEVT_MENU, &MainFrame::OnShowOverrides, this, ID_Overrides);
	Bind(wxEVT_MENU, &MainFrame::OnShowBitmap, this, ID_Show_Bitmap);
	Bind(wxEVT_MENU, &MainFrame::OnSystemTray, this, ID_SystemTray);
	Bind(wxEVT_MENU, &MainFrame::AddLeftClick, this, ID_LeftClick);
	Bind(wxEVT_MENU, &MainFrame::OnCustomFont, this, ID_CustomFont);
	Bind(wxEVT_MENU, &MainFrame::OnCustomFormat, this, ID_CustomFormat);
	Bind(wxEVT_MENU, &MainFrame::OnCustomColor, this, ID_CustomColor);
	Bind(wxEVT_MENU, &MainFrame::UpdateSiteDict, this, ID_UpdateSiteDict);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnCustomFontCol, this, ID_Custom_FontCol);

	adj_date->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUpdateAdjDate, this);
	adj_date->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnUpdateAdjDate, this);
	adj_date_type->Bind(wxEVT_CHOICE, &MainFrame::OnUpdateAdjDateType, this);

	adj_time->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUpdateAdjTime, this);
	adj_time->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnUpdateAdjTime, this);

	time_zone->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUpdateTZ, this);
	time_zone->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnUpdateTZ, this);
	time_zone->Bind(wxEVT_COMBOBOX, &MainFrame::OnUpdateTZ, this);

	bitmap->Bind(wxEVT_KILL_FOCUS, &MainFrame::CalcBitmap, this);
	bitmap->Bind(wxEVT_TEXT_ENTER, &MainFrame::CalcBitmap, this);

	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	Bind(wxEVT_ICONIZE, &MainFrame::OnIconize, this);

	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to make this window visible even when it's not focused", "Stay on top", wxICON_INFORMATION); }, ID_Help_OnTop);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to show the date and time overrides.\n \nSelect a specific override for more information about that particular option", "Show overrides", wxICON_INFORMATION); }, ID_Help_Overrides);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter +N or -N to adjust today's date by N days\nEnter a date of the selected type to show data for that date.", "Adjust Date", wxICON_INFORMATION); }, ID_Help_Override_Date);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Use this menu to select what kind of date is entered in the \"Adjust\" box:\n \n  Date takes human readable format (ex. 12/31)\n  Julian Date takes a Julian Date (ex. 15430)\n  $H Date takes a date in $H format (ex. 64737).", "Date Type", wxICON_INFORMATION); }, ID_Help_Override_Date_Type);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter +S or -S to adjust the current time by S seconds\nEnter +/- M:S to adjust the time by M minutes and S seconds\nEnter +/- H:M:S to adjust the time by H hours, M minues, and S seconds\nEnter a time (ex. 15:20:14 or 3:20:14 PM) to show data for that time.", "Adjust Time", wxICON_INFORMATION); }, ID_Help_Override_Time);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to show the small bitmap calculator", "Show Bitmap Calculator", wxICON_INFORMATION); }, ID_Help_Show_Bitmap);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter a nmber by itself to calculate the bits which are set (ex. 2560 returns bits 10 and 12)\n \nInclude one or more commas (\",\") to instead calculate a bitmap from it's constituent bits (ex. 10, 12 returns 2560)", "Bitmap Calculator", wxICON_INFORMATION); }, ID_Help_Small_Bitmap);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to save the window size and position when closing", "Save position", wxICON_INFORMATION); }, ID_Help_SavePos);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to include the time zone differences when displaying time offset from the current time", "Include Timezone Difference", wxICON_INFORMATION); }, ID_Help_TZOffs);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to keep the window alive in the system tray when minimizing or closing via the \"X\" icon", "Minimize to System Tray", wxICON_INFORMATION); }, ID_Help_SystemTray);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option if you'd like the system tray icon to respond to a single click the same way it does right click", "Register Single Click", wxICON_INFORMATION); }, ID_Help_LeftClick);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to modify the font style, size, and/or color used", "Custom font", wxICON_INFORMATION); }, ID_Help_CustomFont);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to modify the background color of the window", "Custom color", wxICON_INFORMATION); }, ID_Help_CustomColor);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to change the formatting of the date and time\n \nFormat must be provided in strftime style\nSome examples are provided; if something else is desired, any strftime compatible format will be accepted", "Custom format", wxICON_INFORMATION); }, ID_Help_CustomFormat);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Use this function to update the site code time zone dictionary used by the program", "Update site code dictionary", wxICON_INFORMATION); }, ID_Help_UpdateSiteDict);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Version " + string(version) + string(version_date) + "\nCreated by Erik A.Robertson\n \nThis application was created using C++ (standard 20) and wxWidgets version 3.1.3", "About", wxICON_INFORMATION); }, wxID_ABOUT);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select this option to modify the font color if the colors available in the custom font dialog aren't sufficient", "Custom font color", wxICON_INFORMATION); }, ID_Help_Custom_FontCol);

	wxWindowList queue {};
	queue.push_back(this);
	while (!queue.IsEmpty()) {
		const wxWindowList::value_type &node = queue.front();
		wxWindowList node_children = node->GetChildren();
		if (!node_children.IsEmpty()) {
			queue.splice(queue.end(), node_children);
		}
		node->Bind(wxEVT_SET_FOCUS, &MainFrame::OnGetFocus, this);
		node->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnLoseFocus, this);
		queue.pop_front();
	}

	clock = new wxTimer(this, wxID_ANY);
	CURRENT_ZONE = current_zone();
	CURRENT_ZONE_NAME = CURRENT_ZONE->name();
	time_zone->SetValue(CURRENT_ZONE_NAME);
	load_settings(this);
	UpdateStyle();
	clock->Start(1000);
	app_frames.insert(this);
}

void MainFrame::ReSize() {
	Layout();
	wxWindowUpdateLocker no_updates {this};
	wxSize minsize = GetMinSize();
	SetMinSize(wxDefaultSize);
	wxSize bstsize = GetBestSize();
	wxSize cursize = GetSize();
	SetMinSize(bstsize);
	int minwid = minsize.GetWidth(), minhgt = minsize.GetHeight(), bstwid = bstsize.GetWidth(), bsthgt = bstsize.GetHeight();
	int curwid = cursize.GetWidth(), curhgt = cursize.GetHeight(), newwid = 0, newhgt = 0;
	if (bstwid < minwid) {
		newwid = curwid <= minwid ? bstwid : curwid;
	} else {
		newwid = curwid < bstwid ? bstwid : curwid;
	}
	if (bsthgt < minhgt) {
		newhgt = curhgt <= minhgt ? bsthgt : curhgt;
	} else {
		newhgt = curhgt < bsthgt ? bsthgt : curhgt;
	}
	wxSize newsize {newwid, newhgt};
	if (newsize != cursize) {
		SetSize(newsize);
		Layout();
		wxDisplay disp {(unsigned)wxDisplay::GetFromWindow(this)};
		wxRect thisrect = GetRect();
		wxRect disprect = disp.GetClientArea();
		disprect.x -= buffer;
		disprect.y -= buffer;
		disprect.width += buffer * 2;
		disprect.height += buffer * 2;
		wxPoint curpos = GetPosition();
		if (!disprect.Contains(thisrect)) {
			int newx = curpos.x, newy = curpos.y, curbot = thisrect.GetBottom(), currgt = thisrect.GetRight();
			int disbot = disprect.GetBottom(), disrgt = disprect.GetRight();
			if (currgt > disrgt) {
				newx -= currgt - disrgt;
			}
			if (curbot > disbot) {
				newy -= curbot - disbot;
			}
			if (newx < disprect.x) {
				newx = disprect.x;
			}
			if (newy < disprect.y) {
				newy = disprect.y;
			}
			SetPosition(wxPoint(newx, newy));
		} else {
			int xadj = 0, yadj = 0;
			switch (compare_rects(thisrect, disprect)) {
			case quadrant::IS_TOP:
				xadj = (curwid - newwid) / 2;
				break;
			case quadrant::UP_RGT:
				xadj = curwid - newwid;
				break;
			case quadrant::IS_LFT:
				yadj = (curhgt - newhgt) / 2;
				break;
			case quadrant::ON_TOP:
				xadj = (curwid - newwid) / 2;
				yadj = (curhgt - newhgt) / 2;
				break;
			case quadrant::IS_RGT:
				yadj = (curhgt - newhgt) / 2;
				break;
			case quadrant::DN_LFT:
				yadj = curhgt - newhgt;
				break;
			case quadrant::IS_BOT:
				xadj = (curwid - newwid) / 2;
				break;
			case quadrant::DN_RGT:
				xadj = curwid - newwid;
				yadj = curhgt - newhgt;
				break;
			case quadrant::INVALID:
				wxMessageBox("Something has gone very wrong.  Code resz_cmp_rect651", "Error", wxOK | wxCENTER | wxICON_ERROR);
			}
			curpos.x += xadj;
			curpos.y += yadj;
			SetPosition(curpos);
		}
	}
}

void MainFrame::ShowExtraFunctions(wxCommandEvent &evt) {
	ExtraFunctions_Frame *ext_child = nullptr;
	for (wxFrame *const &child : app_frames) {
		ext_child = dynamic_cast<ExtraFunctions_Frame *>(child);
		if (ext_child) break;
	}
	if (ext_child) {
		ext_child->Show();
		ext_child->Iconize(false);
		ext_child->Raise();
		ext_child->SetFocus();
	} else {
		ext_child = new ExtraFunctions_Frame(this);
		ext_child->Show();
	}
}

void MainFrame::CloseExtraFunctions(wxCommandEvent &evt) {
	ExtraFunctions_Frame *ext_child = nullptr;
	for (wxFrame *const &child : app_frames) {
		ext_child = dynamic_cast<ExtraFunctions_Frame *>(child);
		if (ext_child) break;
	}
	if (ext_child) {
		ext_child->Close(true);
	}
}

void MainFrame::ShowPieceMan(wxCommandEvent &evt) {
	PieceMan_Frame *piece_child = nullptr;
	for (wxFrame *const &child : app_frames) {
		piece_child = dynamic_cast<PieceMan_Frame *>(child);
		if (piece_child) break;
	}
	if (piece_child) {
		piece_child->Show();
		piece_child->Iconize(false);
		piece_child->Raise();
		piece_child->SetFocus();
	} else {
		piece_child = new PieceMan_Frame(this);
		piece_child->Show();
	}
}

void MainFrame::ClosePieceMan(wxCommandEvent &evt) {
	PieceMan_Frame *piece_child = nullptr;
	for (wxFrame *const &child : app_frames) {
		piece_child = dynamic_cast<PieceMan_Frame *>(child);
		if (piece_child) break;
	}
	if (piece_child) {
		piece_child->Close(true);
	}
}

void MainFrame::OnExit(wxCommandEvent &evt) {
	delete force_date, force_time;
	force_date = nullptr;
	force_time = nullptr;
	offset = null_duration;
	adj.set_timezone(now.get_timezone());
	row_1_sizer->Show(adj_date_time_sizer);
	ReSize();
	UpdateDisplay();
	save_settings(this);
	Close(true);
}

void MainFrame::clear_fields() {
	wxWindowUpdateLocker no_updates {this};
	adj_date->SetValue("");
	OnUpdateAdjDate(menu_evt);
	adj_time->SetValue("");
	OnUpdateAdjTime(menu_evt);
	time_zone->SetValue(CURRENT_ZONE_NAME);
	OnUpdateTZ(menu_evt);
	bitmap->SetValue("");
	CalcBitmap(menu_evt);
}

void MainFrame::OnClose(wxCloseEvent &evt) {
	// For some reason, attempting to run OnExit during a shutdown event causes
	// errors.  I'm guessing because it's trying to open a file handle during shutdown
	// What I'd like to do is open the file handle when the program opens, preferably
	// writing settings into it dynamically so they're up-to-date even if it crashes
	// But I haven't had time to figure that out.
	if (evt.CanVeto()) {
		if (m_system_tray->IsChecked()) {
			clear_fields();
			Hide();
			evt.Veto();
			return;
		} else {
			OnExit(menu_evt);
		}
	}
	evt.Skip();
}

void MainFrame::OnIconize(wxIconizeEvent &evt) {
	if (IsIconized() && m_system_tray->IsChecked()) {
		// Deiconize after iconizing here because we're about to hide
		// and this allows the display to be updated when the fields are cleared
		Iconize(false);
		clear_fields();
		Hide();
		return;
	}
	evt.Skip();
}

void MainFrame::FlashWindow(wxEvent &evt = menu_evt) {
	if (!HasFocus()) RequestUserAttention();
	evt.Skip();
}

MainFrame::~MainFrame() {
	clock->Stop();
	tb_icon->Destroy();
	delete main_panel, control_panel, clock;
	app_frames.erase(this);
}

void MainFrame::UpdateStyle() {
	wxFont current_font = GetFont();
	wxColor current_font_color = GetForegroundColour();
	wxColor current_bg_color = GetBackgroundColour();
	wxColor text_bg_col(min(current_bg_color.Red() + white_layer, 255), min(current_bg_color.Green() + white_layer, 255), min(current_bg_color.Blue() + white_layer, 255));
	wxWindowList queue {};
	queue.push_back(this);
	while (!queue.IsEmpty()) {
		const wxWindowList::value_type &node = queue.front();
		wxWindowList node_children = node->GetChildren();
		if (!node_children.IsEmpty()) {
			queue.splice(queue.end(), node_children);
		}
		node->SetFont(current_font);
		node->SetForegroundColour(current_font_color);
		// If the node is a wxTextCtrl but not an Invisible_TextCtrl, use the text background color instead of window bg color
		if ((dynamic_cast<wxTextCtrl *>(node)) && !(dynamic_cast<Invisible_TextCtrl *>(node))) {
			node->SetBackgroundColour(text_bg_col);
		} else {
			node->SetBackgroundColour(current_bg_color);
		}
		queue.pop_front();
	}
	Refresh();
	UpdateDisplay();
	ReSize();
}
