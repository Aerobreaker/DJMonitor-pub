#include "frames.hpp"

#include "window.hpp"
#include <wx/menu.h>

Main_StaticText::Main_StaticText(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos, const wxSize &size, long style, const wxString &name) : wxStaticText(parent, id, label, pos, size, style, name) {
	SetMinSize(DEFAULT_TEXTBOX_SIZE);
	SetFont(parent->GetFont());
	SetForegroundColour(parent->GetForegroundColour());
	SetBackgroundColour(parent->GetBackgroundColour());
}

Main_TextCtrl::Main_TextCtrl(wxWindow *parent, wxWindowID id, const wxString &value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxTextCtrl(parent, id, value, pos, size, style, validator, name) {
	using std::min;
	SetMinSize(DEFAULT_TEXTBOX_SIZE);
	SetFont(parent->GetFont());
	SetForegroundColour(parent->GetForegroundColour());
	wxColor parent_bg = parent->GetBackgroundColour();
	SetBackgroundColour(wxColor(min(parent_bg.Red() + 15, 255), min(parent_bg.Green() + 15, 255), min(parent_bg.Blue() + 15, 255)));
}

Invisible_TextCtrl::Invisible_TextCtrl(wxWindow *parent, wxWindowID id, const wxString &value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : Main_TextCtrl(parent, id, value, pos, size, style | wxTE_READONLY | wxBORDER_NONE, validator, name) {;
	SetBackgroundColour(parent->GetBackgroundColour());
}

Invisible_StaticBox::Invisible_StaticBox(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos, const wxSize &size, long style, const wxString &name) : wxStaticBox(parent, id, label, pos, size, style, name) {
	SetFont(parent->GetFont());
	SetForegroundColour(parent->GetForegroundColour());
	SetBackgroundColour(parent->GetBackgroundColour());
}

Invisible_Panel::Invisible_Panel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name) : wxPanel(parent, winid, pos, size, style, name) {
	SetFont(parent->GetFont());
	SetForegroundColour(parent->GetForegroundColour());
	SetBackgroundColour(parent->GetBackgroundColour());
}

void Main_TextCtrl::SetValue(const wxString &value) {
	last_value = GetValue();
	wxTextCtrl::SetValue(value);
}

void Main_TextCtrl::RevertValue() {
	wxTextCtrl::SetValue(last_value);
}

void Main_TextCtrl::SetLast(const wxString &value) {
	last_value = value;
}

wxString Main_TextCtrl::GetLast() {
	return last_value;
}

void Invisible_TextCtrl::UpdateValue(const wxString &newval, wxStaticText *placeholder) {
	bool need_placeholder = !placeholder;
	if (need_placeholder) placeholder = new Main_StaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_READONLY | wxBORDER_NONE);
	long start = 0, end = 0;
	bool hassel = HasSelection();
	if (hassel) GetSelection(&start, &end);
	ChangeValue(newval);
	placeholder->SetLabel(newval);
	wxSize plcsize = placeholder->GetBestSize();
	plcsize.SetWidth(plcsize.GetWidth() + 5);
	SetMinSize(plcsize);
	if (hassel) SetSelection(start, end);
	if (need_placeholder) delete placeholder;
}

void SystemTray_Icon::ReplicateRightDown(wxTaskBarIconEvent &evt) {
	wxTaskBarIconEvent newevt(evt);
	newevt.SetEventType(wxEVT_TASKBAR_RIGHT_DOWN);
	evt.Skip();
	if (lclick) ProcessEvent(newevt);
}

void SystemTray_Icon::ReplicateRightUp(wxTaskBarIconEvent &evt) {
	wxTaskBarIconEvent newevt(evt);
	newevt.SetEventType(wxEVT_TASKBAR_RIGHT_UP);
	evt.Skip();
	if (lclick) ProcessEvent(newevt);
}

SystemTray_Icon::SystemTray_Icon(MainFrame *parent, wxTaskBarIconType iconType, bool leftclick) : wxTaskBarIcon(iconType) {
	_parent = parent;
	lclick = leftclick;
	Bind(wxEVT_MENU, &SystemTray_Icon::ShowParent, this, ID_RestoreParent);
	Bind(wxEVT_MENU, &SystemTray_Icon::HideParent, this, ID_HideParent);
	Bind(wxEVT_MENU, &SystemTray_Icon::FindParent, this, ID_FindParent);
	Bind(wxEVT_MENU, &SystemTray_Icon::ShowExtra, this, ID_ShowExtras);
	Bind(wxEVT_MENU, &SystemTray_Icon::CloseExtra, this, ID_CloseExtras);
	Bind(wxEVT_MENU, &SystemTray_Icon::ShowPiece, this, ID_ShowPieces);
	Bind(wxEVT_MENU, &SystemTray_Icon::ClosePiece, this, ID_ClosePieces);
	Bind(wxEVT_MENU, &SystemTray_Icon::ExitParent, this, wxID_EXIT);
	Bind(wxEVT_TASKBAR_LEFT_DCLICK, &SystemTray_Icon::ShowParent, this);
	Bind(wxEVT_TASKBAR_LEFT_DOWN, &SystemTray_Icon::ReplicateRightDown, this);
	Bind(wxEVT_TASKBAR_LEFT_UP, &SystemTray_Icon::ReplicateRightUp, this);
}

void SystemTray_Icon::ShowParent(wxEvent &evt) {
	_parent->Show();
	_parent->Iconize(false);
	_parent->Raise();
	_parent->SetFocus();
}

void SystemTray_Icon::HideParent(wxCommandEvent &evt) {
	_parent->Hide();
}

void SystemTray_Icon::ExitParent(wxCommandEvent &evt) {
	_parent->OnExit(menu_evt);
}

void SystemTray_Icon::FindParent(wxCommandEvent &evt) {
	ShowParent(evt);
	_parent->Center();
}

void SystemTray_Icon::ShowExtra(wxCommandEvent &evt) {
	_parent->ShowExtraFunctions(evt);
}

void SystemTray_Icon::CloseExtra(wxCommandEvent &evt) {
	_parent->CloseExtraFunctions(evt);
}

void SystemTray_Icon::ShowPiece(wxCommandEvent &evt) {
	_parent->ShowPieceMan(evt);
}

void SystemTray_Icon::ClosePiece(wxCommandEvent &evt) {
	_parent->ClosePieceMan(evt);
}

wxMenu *SystemTray_Icon::CreatePopupMenu() {
	wxMenu *menu = new wxMenu;
	wxMenuItem *restore = new wxMenuItem(menu, ID_RestoreParent, "&Bring to Front");
	wxMenuItem *hide = new wxMenuItem(menu, ID_HideParent, "&Hide");
	wxMenuItem *show_extra = new wxMenuItem(menu, ID_ShowExtras, "&Show extra functions");
	wxMenuItem *close_extra = new wxMenuItem(menu, ID_CloseExtras, "&Close extra functions");
	wxMenuItem *show_piece = new wxMenuItem(menu, ID_ShowPieces, "Show &piece manager");
	wxMenuItem *close_piece = new wxMenuItem(menu, ID_ClosePieces, "C&lose piece manager");
	wxMenuItem *find = new wxMenuItem(menu, ID_FindParent, "Help!  I'm lost!  &Find me!");
	wxMenuItem *quit = new wxMenuItem(menu, wxID_EXIT);
	menu->Append(show_extra);
	menu->Append(close_extra);
	menu->AppendSeparator();
	menu->Append(show_piece);
	menu->Append(close_piece);
	menu->AppendSeparator();
	menu->Append(restore);
	menu->Append(hide);
	menu->Append(find);
	menu->Append(quit);
	return menu;
}

bool Invisible_TextCtrl::SetFont(const wxFont &font, bool bold) {
	if (bold) {
		if (font.GetWeight() == wxFONTWEIGHT_NORMAL) return Main_TextCtrl::SetFont(font.Bold());
		wxFont new_font {font};
		new_font.SetWeight(wxFONTWEIGHT_MAX);
		return Main_TextCtrl::SetFont(new_font);
	}
	return Main_TextCtrl::SetFont(font);
}

bool Invisible_StaticBox::SetFont(const wxFont &font) {
	wxFont new_font {font};
	new_font.SetPointSize(new_font.GetPointSize() - 2);
	return wxStaticBox::SetFont(new_font);
}

void ChildFrame::OnGetFocus(wxFocusEvent &evt) {
	for (const wxFrame *const &frm : app_frames) {
		if (frm->GetWindowStyle() & wxSTAY_ON_TOP) {
			SetWindowStyle(GetWindowStyle() | wxSTAY_ON_TOP);
			Raise();
			break;
		}
	}
	evt.Skip();
}

void ChildFrame::SetOnTop(bool raise) {
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

void ChildFrame::OnLoseFocus(wxFocusEvent &evt) {
	SetOnTop(false);
	evt.Skip();
}

void ChildFrame::OnStayOnTop(wxCommandEvent &evt) {
	// Notably, if removing stay-on-top from a frame while another has stay-on-top, the current frame will drop behind the other
	// In theory, a quick SetFocus() here would fix that.  In practice, it works once.  Possibly sleeping 50ms and then raising
	// a focus event or calling SetFocus() might work?
	SetOnTop(true);
	
	evt.Skip();
}

void ChildFrame::OnClose(wxCommandEvent &evt) {
	Close(true);
}

void ChildFrame::EndCreation() {
	wxFont font = GetFont();
	wxColor fgcol = GetForegroundColour();
	wxColor bgcol = GetBackgroundColour();
	wxColor text_bg_col(std::min(bgcol.Red() + white_layer, 255), std::min(bgcol.Green() + white_layer, 255), std::min(bgcol.Blue() + white_layer, 255));

	wxWindowList queue = this->GetChildren();
	while (!queue.IsEmpty()) {
		const wxWindowList::value_type &node = queue.front();
		wxWindowList node_children = node->GetChildren();
		if (!node_children.IsEmpty()) queue.splice(queue.end(), node_children);
		node->Bind(wxEVT_SET_FOCUS, &ChildFrame::OnGetFocus, this);
		node->Bind(wxEVT_KILL_FOCUS, &ChildFrame::OnLoseFocus, this);
		node->SetFont(font);
		node->SetForegroundColour(fgcol);
		// If the node is a wxTextCtrl but not an Invisible_TextCtrl, set the text background color instead of standard
		if ((dynamic_cast<wxTextCtrl *>(node)) && !(dynamic_cast<Invisible_TextCtrl *>(node))) {
			node->SetBackgroundColour(text_bg_col);
		} else {
			node->SetBackgroundColour(bgcol);
		}
		queue.pop_front();
	}

	Fit();
	CenterOnParent();
}

ChildFrame::ChildFrame(MainFrame *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxFrame(parent, id, title, pos, size, style) {
	SetSizeHints(450, 250);
	SetIcon(parent->GetIcon());

	wxMenuBar *main_menu = new wxMenuBar(0);
	wxMenu *file_menu = new wxMenu();
	m_stay_on_top = new wxMenuItem(file_menu, ID_Child_OnTop, "&Stay on top", wxEmptyString, wxITEM_CHECK);
	wxMenuItem *m_quit = new wxMenuItem(file_menu, wxID_EXIT);
	file_menu->Append(m_stay_on_top);
	file_menu->Append(m_quit);
	main_menu->Append(file_menu, "&File");
	SetMenuBar(main_menu);

	if (parent->GetWindowStyle() & wxSTAY_ON_TOP) {
		m_stay_on_top->Check();
		SetWindowStyle(GetWindowStyle() | wxSTAY_ON_TOP);
	}

	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	SetSizer(main_sizer);

	Bind(wxEVT_MENU, &ChildFrame::OnStayOnTop, this, ID_Child_OnTop);
	Bind(wxEVT_MENU, &ChildFrame::OnClose, this, wxID_EXIT);

	SetFont(parent->GetFont());
	SetForegroundColour(parent->GetForegroundColour());
	SetBackgroundColour(parent->GetBackgroundColour());
	Bind(wxEVT_SET_FOCUS, &ChildFrame::OnGetFocus, this);
	Bind(wxEVT_KILL_FOCUS, &ChildFrame::OnLoseFocus, this);

	app_frames.insert(this);
}

ChildFrame::~ChildFrame() {
	app_frames.erase(this);
}
