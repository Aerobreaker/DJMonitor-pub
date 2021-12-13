#pragma once

#include "all_header.hpp"
#include "globals.hpp"

#include <wx/frame.h>
#include <wx/font.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/menuitem.h>
#include <DateTime.hpp>

const wxFont DEFAULT_FONT = wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString);
const wxColor DEFAULT_FONT_COLOR = wxColor(0, 0, 0);
const wxColor DEFAULT_BG_COLOR = wxColor(240, 240, 240);
const wxSize DEFAULT_DROPBOX_SIZE = wxSize(50, 23);
const wxSize DEFAULT_TEXTBOX_SIZE = wxSize(50, 14);

/*META-COMMENT:
* The indexing dates have been modified to protect potentially proprietary information.
* Additionally, some fields have been renamed for  the same reason.
* NOTE: Caché $H indexing date has not been modified, as it's the standard set by Intersystems
*/

constexpr datetime::system_duration null_duration = datetime::system_duration(0);
constexpr date::sys_days null_date = date::sys_days(date::days(0));
constexpr date::sys_days sq_julian_date_start = date::August / 31 / 1989;
constexpr date::sys_days h_julian_date_start = date::December / 31 / 1840;
constexpr date::sys_days sq_mj_start = date::August / 1 / 1989;
constexpr date::sys_days sq_mj_2_start = date::October / 10 / 2017;

constexpr unsigned char default_buffer = 8;

class Main_StaticText;
class Main_TextCtrl;
class Invisible_TextCtrl;
class Invisible_StaticBox;
class Invisible_Panel;
class SystemTray_Icon;

class MainFrame : public wxFrame {
private:
	Invisible_Panel *main_panel;
	Invisible_TextCtrl *curr_date_time;
	Invisible_TextCtrl *adj_date_time;
	Main_StaticText *control_panel;
	Invisible_TextCtrl *julian_date;
	Invisible_Panel *tz_panel;
	wxComboBox *time_zone;
	Invisible_Panel *adj_date_panel;
	Main_TextCtrl *adj_date;
	Invisible_TextCtrl *julian_month_1;
	Invisible_TextCtrl *h_time;
	Invisible_Panel *adj_date_type_panel;
	wxChoice *adj_date_type;
	Invisible_TextCtrl *julian_month_2;
	Invisible_TextCtrl *h_date;
	Invisible_Panel *adj_time_panel;
	Main_TextCtrl *adj_time;
	wxBoxSizer *main_panel_sizer;
	wxBoxSizer *main_sizer;
	wxBoxSizer *row_1_sizer;
	wxStaticBoxSizer *curr_date_time_sizer;
	wxStaticBoxSizer *adj_date_time_sizer;
	wxBoxSizer *row_2_sizer;
	wxStaticBoxSizer *julian_date_sizer;
	wxStaticBoxSizer *adj_date_sizer;
	wxBoxSizer *row_3_sizer;
	wxStaticBoxSizer *julian_month_1_sizer;
	wxStaticBoxSizer *h_time_sizer;
	wxStaticBoxSizer *adj_date_type_sizer;
	wxBoxSizer *row_4_sizer;
	wxStaticBoxSizer *julian_month_2_sizer;
	wxStaticBoxSizer *h_date_sizer;
	wxStaticBoxSizer *adj_time_sizer;
	wxStaticBoxSizer *tz_sizer;
	wxBoxSizer *row_5_sizer;
	Invisible_Panel *bitmap_panel;
	wxStaticBoxSizer *bitmap_sizer;
	wxStaticBoxSizer *bit_sizer;
	Main_TextCtrl *bitmap;
	Invisible_TextCtrl *bits;

	wxTimer *clock;
	datetime::DateTime<> adj = datetime::DateTime<>(null_duration);
	datetime::DateTime<> now = datetime::DateTime<>(null_duration);
	date::sys_days *force_date = nullptr;
	datetime::system_duration *force_time = nullptr;
	datetime::system_duration offset = null_duration;
	bool stale = false;
	unsigned char buffer = default_buffer;
	unsigned char white_layer = def_white_layer;

	friend void save_settings(MainFrame *frame);
	friend void load_settings(MainFrame *frame);

	void clear_fields();

public:
	wxMenuItem *m_stay_on_top;
	wxMenuItem *m_show_overrides;
	wxMenuItem *m_save_pos;
	wxMenuItem *m_show_tz_offs;
	wxMenuItem *m_system_tray;
	wxMenuItem *m_custom_font;
	wxMenuItem *m_custom_color;
	wxMenuItem *m_custom_format;
	wxMenuItem *m_show_bitmap;
	wxMenuItem *m_left_click;
	wxMenuItem *m_custom_font_col;
	SystemTray_Icon *tb_icon;

	MainFrame(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = "Julian Date (v" + std::string(version) + ")", const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	void OnTimer(wxTimerEvent &evt);
	void UpdateDisplay();
	void ReSize();
	void OnUpdateAdjDate(wxEvent &evt);
	void OnUpdateAdjDateType(wxEvent &evt);
	void OnUpdateAdjTime(wxEvent &evt);
	void OnUpdateTZ(wxEvent &evt);
	void OnStayOnTop(wxCommandEvent &evt);
	void OnShowOverrides(wxCommandEvent &evt);
	void OnSystemTray(wxCommandEvent &evt);
	void AddLeftClick(wxCommandEvent &evt);
	void OnExit(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnIconize(wxIconizeEvent &evt);
	void OnCustomFont(wxCommandEvent &evt);
	void OnCustomColor(wxCommandEvent &evt);
	void OnCustomFormat(wxCommandEvent &evt);
	void UpdateSiteDict(wxCommandEvent &evt);
	void ShowExtraFunctions(wxCommandEvent &evt);
	void CloseExtraFunctions(wxCommandEvent &evt);
	void OnShowBitmap(wxCommandEvent &evt);
	void CalcBitmap(wxEvent &evt);
	void FlashWindow(wxEvent &evt);
	void OnLoseFocus(wxFocusEvent &evt);
	void OnGetFocus(wxFocusEvent &evt);
	void SetOnTop(bool raise);
	void ShowPieceMan(wxCommandEvent &evt);
	void ClosePieceMan(wxCommandEvent &evt);
	void UpdateStyle();
	void OnCustomFontCol(wxCommandEvent &evt);

	friend int MainApp::OnRun();
	friend ChildFrame;

	~MainFrame();
};
