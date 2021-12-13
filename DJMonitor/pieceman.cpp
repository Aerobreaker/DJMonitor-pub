#include "pieceman.hpp"

#include "functions.hpp"
#include "window.hpp"

#include <wx/menu.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>
#include <string>

using namespace std;

void PieceMan_Frame::UpdateData() {
	search->Clear();
	pc->Set({"All"});
	pc->SetSelection(0);

	if (dat == "") {
		output->SetValue("");
		return;
	}

	pcs = pieces(dat, del);
	string delimstr {""};
	nstr.clear();
	pstr.clear();
	for (size_t i = 0; i < pcs.size(); ) {
		string numstr = to_string(i+1);
		long long dif = pcs[i].length() - numstr.length();
		size_t prelen = floor(static_cast<double>(abs(dif)) / 2), postlen = ceil(static_cast<double>(abs(dif)) / 2);
		if (dif >= 0) {
			pstr.append(delimstr).append(pcs[i]);
		} else {
			pstr.append(delimstr).append(prelen, ' ').append(pcs[i]).append(postlen, ' ');
		}
		if (dif > 0) {
			nstr.append(delimstr).append(prelen, ' ').append(numstr).append(postlen, ' ');
		} else {
			nstr.append(delimstr).append(numstr);
		}
		if (!i) delimstr = ' ' + del + ' ';
		pc->Append({wxString::FromDouble(++i)});
	}

	UpdateDisplay();
}

void PieceMan_Frame::UpdatePiece() {
	pnum = pc->GetSelection();
	if (pnum) {
		output->SetValue(pcs[--pnum]);
		if (pcs[pnum++] == "") output->SetValue("\"\"");
	} else {
		output->SetValue(disp);
	}
	Layout();
	Refresh();
}

void PieceMan_Frame::UpdateDisplay() {
	disp.clear();
	output->SetValue(pstr);

	int cnt = output->GetNumberOfLines();
	vector<int> lines(cnt);
	for (int i = 0; i < cnt; i++) {
		lines[i] = output->GetLineLength(i);
	}
	size_t st = 0;
	for (int i = 0; i < cnt; i++) {
		int len = lines[i];
		output->SetValue(nstr.substr(st, len));
		if (output->GetNumberOfLines() != 1) {
			len = output->GetLineLength(0);
			output->SetValue(pstr.substr(st + len));
			if ((output->GetNumberOfLines() + i) > cnt) {
				cnt = i + output->GetNumberOfLines();
				lines.push_back(0);
			}
			for (int j = i + 1; j < cnt; j++) {
				lines[j] = output->GetLineLength(j - i - 1);
			}
		}
		disp.append(pstr.substr(st, len)).append(1, '\n').append(nstr.substr(st, len)).append(2, '\n');
		st += len;
	}
	if (st < pstr.length()) disp.append(pstr.substr(st)).append(1, '\n').append(nstr.substr(st));
	while (!disp.empty() && disp.back() == '\n') disp.pop_back();

	UpdatePiece();
}

void PieceMan_Frame::OnUpdateData(wxEvent &evt) {
	evt.Skip();
	string str = data->GetValue().ToStdString();
	if (str == dat) {
		return;
	}
	dat = data->GetValue().ToStdString();
	UpdateData();
}

void PieceMan_Frame::OnUpdateDelim(wxEvent &evt) {
	evt.Skip();
	string str = move(delim->GetValue().ToStdString());
	if (str == del) return;
	if (str == "") {
		delim->RevertValue();
		return;
	}
	del = move(str);
	UpdateData();
}

void PieceMan_Frame::OnUpdateSearch(wxEvent &evt) {
	evt.Skip();
	string str = search->GetValue().ToStdString();
	if (str == "") return;

	for (size_t i = 0; i < pcs.size(); i++) {
		if (pcs[i].find(str) != string::npos) {
			pc->SetSelection(i + 1);
			UpdatePiece();
			return;
		}
	}
}

void PieceMan_Frame::OnUpdatePiece(wxCommandEvent &evt) {
	evt.Skip();
	int sel = pc->GetSelection();
	if (sel == pnum) return;
	UpdatePiece();
}

void PieceMan_Frame::OnTimer(wxTimerEvent &evt) {
	clock->Stop();
	UpdateDisplay();
}

void PieceMan_Frame::OnResize(wxSizeEvent &evt) {
	// Use a timer event to handle updating the display after a resize
	// Attempting to do it during the resize event causes all sorts of bad stuff (window getting yanked out from under the cursor, bad word wraps, etc.)
	// A 0.1 second delay should be barely noticable
	clock->Start(100);
	evt.Skip();
}

PieceMan_Frame::PieceMan_Frame(MainFrame *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : ChildFrame(parent, id, title, pos, size, style) {
	wxMenuBar *menu = GetMenuBar();
	wxMenu *help_menu = new wxMenu();
	wxMenuItem *m_menuItem1 = new wxMenuItem(help_menu, ID_Help_Piece_Data, "&Data");
	wxMenuItem *m_menuItem2 = new wxMenuItem(help_menu, ID_Help_Piece_Delim, "D&elimiter");
	wxMenuItem *m_menuItem3 = new wxMenuItem(help_menu, ID_Help_Piece_Search, "&Search for");
	wxMenuItem *m_menuItem4 = new wxMenuItem(help_menu, ID_Help_Piece_Piece, "&Piece number");
	help_menu->Append(m_menuItem1);
	help_menu->Append(m_menuItem2);
	help_menu->Append(m_menuItem3);
	help_menu->Append(m_menuItem4);
	menu->Append(help_menu, "&Help");

	wxSizer *main_sizer = GetSizer();
	Invisible_Panel *input_panel = new Invisible_Panel(this);
	wxBoxSizer *input_sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *row1_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *data_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(input_panel, wxID_ANY, "Data"), wxVERTICAL);
	data = new Main_TextCtrl(input_panel, wxID_ANY);
	data_sizer->Add(data, 1, wxEXPAND, default_border);
	row1_sizer->Add(data_sizer, 4, wxEXPAND, default_border);
	wxStaticBoxSizer *delim_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(input_panel, wxID_ANY, "Delimiter"), wxVERTICAL);
	delim = new Main_TextCtrl(input_panel, wxID_ANY, del);
	delim->SetLast(del);
	delim_sizer->Add(delim, 1, wxEXPAND, default_border);
	row1_sizer->Add(delim_sizer, 1, wxEXPAND, default_border);
	input_sizer->Add(row1_sizer, 1, wxEXPAND);

	wxBoxSizer *row2_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer *search_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(input_panel, wxID_ANY, "Search for"), wxVERTICAL); 
	search = new Main_TextCtrl(input_panel, wxID_ANY);
	search_sizer->Add(search, 1, wxEXPAND, default_border);
	row2_sizer->Add(search_sizer, 4, wxEXPAND, default_border);
	wxString all[1] {"All"};
	wxStaticBoxSizer *piece_sizer = new wxStaticBoxSizer(new Invisible_StaticBox(input_panel, wxID_ANY, "Piece number"), wxVERTICAL);
	pc = new wxChoice(input_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, all);
	pc->SetSelection(0);
	piece_sizer->Add(pc, 1, wxEXPAND, default_border);
	row2_sizer->Add(piece_sizer, 1, wxEXPAND, default_border);
	input_sizer->Add(row2_sizer, 1, wxEXPAND);

	input_panel->SetSizer(input_sizer);
	main_sizer->Add(input_panel, 1, wxEXPAND);

	wxStaticLine *separator = new wxStaticLine(this, wxID_ANY);
	main_sizer->Add(separator, 0, wxEXPAND);

	wxBoxSizer *output_sizer = new wxBoxSizer(wxVERTICAL);

	output = new Invisible_TextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxALIGN_CENTRE_HORIZONTAL);
	output_sizer->Add(output, 1, wxALL | wxEXPAND, default_border);
	main_sizer->Add(output_sizer, 1, wxEXPAND);

	clock = new wxTimer(this, wxID_ANY);

	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter the data to be pieced out in this field", "Data", wxICON_INFORMATION); }, ID_Help_Piece_Data);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter the delimiter to be used for piecing out the data in this field", "Delimiter", wxICON_INFORMATION); }, ID_Help_Piece_Delim);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Enter a string here to display the first piece that contains the string", "Search for", wxICON_INFORMATION); }, ID_Help_Piece_Search);
	Bind(wxEVT_MENU, [](wxCommandEvent &evt) {wxMessageBox("Select \"All\" to display all pieces\n\n-or-\n\nSelect a piece number to display only that piece", "Piece", wxICON_INFORMATION); }, ID_Help_Piece_Piece);

	data->Bind(wxEVT_KILL_FOCUS, &PieceMan_Frame::OnUpdateData, this);
	data->Bind(wxEVT_TEXT_ENTER, &PieceMan_Frame::OnUpdateData, this);
	delim->Bind(wxEVT_KILL_FOCUS, &PieceMan_Frame::OnUpdateDelim, this);
	delim->Bind(wxEVT_TEXT_ENTER, &PieceMan_Frame::OnUpdateDelim, this);
	search->Bind(wxEVT_KILL_FOCUS, &PieceMan_Frame::OnUpdateSearch, this);
	search->Bind(wxEVT_TEXT_ENTER, &PieceMan_Frame::OnUpdateSearch, this);
	pc->Bind(wxEVT_CHOICE, &PieceMan_Frame::OnUpdatePiece, this);

	Bind(wxEVT_SIZE, &PieceMan_Frame::OnResize, this);
	Bind(wxEVT_TIMER, &PieceMan_Frame::OnTimer, this);

	EndCreation();

	output->SetFont(DEFAULT_FONT);
	Layout();
	Refresh();
}

PieceMan_Frame::~PieceMan_Frame() {
	clock->Stop();
}
