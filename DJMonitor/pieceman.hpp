#pragma once

#include "all_header.hpp"
#include "frames.hpp"

#include <wx/choice.h>
#include <wx/timer.h>

class PieceMan_Frame : public ChildFrame {
private:
	int pnum = 0;
	/*META-COMMENT:
	* Default delimiter has been modified to protect potentially proprietary information.
	*/

	std::string del {"_"};
	std::string dat {""};
	std::string pstr {""};
	std::string nstr {""};
	std::string disp {""};
	std::vector<std::string> pcs {};
	wxTimer *clock;

public:
	PieceMan_Frame(MainFrame *parent, wxWindowID id = wxID_ANY, const wxString &title = "Piece Manager", const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(-1, -1), long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

	Main_TextCtrl *data;
	Main_TextCtrl *delim;
	Main_TextCtrl *search;
	wxChoice *pc;
	Invisible_TextCtrl *output;

	void OnUpdateData(wxEvent &evt);
	void OnUpdateDelim(wxEvent &evt);
	void OnUpdateSearch(wxEvent &evt);
	void OnUpdatePiece(wxCommandEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	void UpdateData();
	void UpdatePiece();
	void UpdateDisplay();

	~PieceMan_Frame();
};
