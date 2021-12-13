#pragma once

#include "all_header.hpp"
#include "frames.hpp"

class ExtraFunctions_Frame : public ChildFrame {
public:
	ExtraFunctions_Frame(MainFrame *parent, wxWindowID id = wxID_ANY, const wxString &title = "Extra Functions", const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	void DoFixTag(wxCommandEvent &evt);
	void DoExpandTag(wxCommandEvent &evt);
	void DoCollapseTag(wxCommandEvent &evt);
	void DoBuildLoop(wxCommandEvent &evt);
	void DoBitmapCalc(wxCommandEvent &evt);

	Main_TextCtrl *input;
	Invisible_TextCtrl *output;
};
