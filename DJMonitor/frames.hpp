#pragma once

#include "all_header.hpp"

#include <unordered_set>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/taskbar.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>

class Main_StaticText : public wxStaticText {
public:
	Main_StaticText(wxWindow *parent, wxWindowID id, const wxString &label = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = wxStaticTextNameStr);
};

class Main_TextCtrl : public wxTextCtrl {
private:
	wxString last_value = "";
public:
	Main_TextCtrl(wxWindow *parent, wxWindowID id, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxTE_PROCESS_ENTER, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxTextCtrlNameStr);
	void SetValue(const wxString &value);
	void RevertValue();
	void SetLast(const wxString &value);
	wxString GetLast();
};

class Invisible_TextCtrl : public Main_TextCtrl {
public:
	Invisible_TextCtrl(wxWindow *parent, wxWindowID id, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxTE_CENTER, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxTextCtrlNameStr);
	virtual bool AcceptsFocus() const { return false; }
	virtual bool SetFont(const wxFont &font, bool bold = true);
	void UpdateValue(const wxString &newval, wxStaticText *placeholder = nullptr);
};

class Invisible_StaticBox : public wxStaticBox {
public:
	Invisible_StaticBox(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxString &name = wxStaticBoxNameStr);
	virtual bool SetFont(const wxFont &font);
};

class Invisible_Panel : public wxPanel {
public:
	Invisible_Panel(wxWindow *parent, wxWindowID winid = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, const wxString &name = wxPanelNameStr);
};

class SystemTray_Icon : public wxTaskBarIcon {
private:
	MainFrame *_parent;
public:
	SystemTray_Icon(MainFrame *parent, wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE, bool leftclick = false);
	bool lclick = false;
	wxMenu *CreatePopupMenu();
	void ShowParent(wxEvent &evt);
	void HideParent(wxCommandEvent &evt);
	void ExitParent(wxCommandEvent &evt);
	void FindParent(wxCommandEvent &evt);
	void ShowExtra(wxCommandEvent &evt);
	void CloseExtra(wxCommandEvent &evt);
	void ShowPiece(wxCommandEvent &evt);
	void ClosePiece(wxCommandEvent &evt);
	void ReplicateRightDown(wxTaskBarIconEvent &evt);
	void ReplicateRightUp(wxTaskBarIconEvent &evt);
};

class ChildFrame : public wxFrame {
protected:
	unsigned char white_layer = def_white_layer;

public:
	ChildFrame(MainFrame *parent, wxWindowID id = wxID_ANY, const wxString &title = "", const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	wxMenuItem *m_stay_on_top;

	virtual void OnClose(wxCommandEvent &evt);
	virtual void OnGetFocus(wxFocusEvent &evt);
	virtual void OnLoseFocus(wxFocusEvent &evt);
	virtual void OnStayOnTop(wxCommandEvent &evt);
	virtual void EndCreation();
	virtual void SetOnTop(bool raise);

	virtual ~ChildFrame();
};
