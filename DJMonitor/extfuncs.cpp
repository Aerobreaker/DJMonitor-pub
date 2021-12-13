#include "extfuncs.hpp"

#include "functions.hpp"
#include "window.hpp"

#include <wx/menu.h>
#include <wx/button.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

bool is_numeric(string inp, bool include_fractions = true);
string join(vector<string> inp, char delim = ' ', bool prim = false);
string join(vector<string>::iterator start, vector<string>::iterator end, char delim = ' ', bool prim = false);
vector<string> tokenize_tag(string inp);
bool rem_braces(vector<string> &inp);

string fix_tag(string inp, size_t buflen = 80) {
	size_t plchold = 0;
	string outp = "";
	outp.reserve(inp.length());
	for (const char &it : inp) {
		switch (it) {
		case '\n':
			while (plchold++ < buflen) outp.push_back(' ');
			plchold = 0;
			break;
		default:
			outp.push_back(it);
			++plchold;
		}
	}
	return outp;
}

string expand_tag(string inp) {
	/*Explanation:
	* MUMPS / Intersystems Cache Objectscript can be a little different from most languages.  The way it's parsed is as follows:
	*     Case_Insensitive_Command {space} [Parameter] {space_or_line_break} Case_Insensitive_Command {space} [Parameter] {space_or_line_break}...
	* Notably, every command is followed by a space.  Then, even if there's no parameter, there's a second space before the next command.
	* Each command in the brace_cmds set (and only these commands) can be followed by a code block encapsulated within {} characters.
	* The {} encapsulating the code block, however is optional.  If omitted, everything following that command's parameter is considered part
	* of it's code block.  Note that additional lines are never assumed to be part of a code block (but can be specified as such by encapsulating
	* said code block between {} characters) and that code blocks can be empty.
	* 
	* There are some exceptions, however:
	*     1. Each command in the part_block set can only exist between pairs of {} blocks.
	*         a. You can't have ELSEIF, for example, unless it immediately follows a }.
	*         b. Each of them, unlike the other commands which utilize code blocks, must have a block contained within {} characters.
	*     2. Each command within the no_param_brace set, and only these commands, does not take a parameter, but instead must be followed by a
	*        code block contained within {} characters.
	*         a. Note that, unlike the brace_cmds set, the {} characters surrounding the code block for these commands are not optional.
	* 
	* The goal of this function is to take a body of MUMPS/Objectscript code that's been written all in one line and split it out into
	* a more readable body of code with proper line breaks and indentation.  This means esentially taking every other space and changing
	* it into a line break, adding {} characters to encapsulate all code blocks, adding indentation to code blocks as appropriate, and then
	* removing any and all trailing spaces on each line.
	*/

	unordered_set<string> brace_cmds = {"I", "IF", "F", "FOR", "ELSEIF", "WHILE"};
	unordered_set<string> part_block = {"ELSE", "CATCH", "ELSEIF"};
	unordered_set<string> no_param_brace = {"TRY", "ELSE", "CATCH"};
	string outp;
	outp.reserve(inp.length()); // We'll almost certainly need more than inp.length() characters (because of indent characters) but this is a better starting point than 0
	vector<string> tokens = tokenize_tag(inp);
	int indentlev = 1;
	vector<int> bracelevels {};
	for (vector<string>::iterator it = tokens.begin(); it != tokens.end(); ) {
		string cmd = *(it++);
		string chkcmd = to_upper(cmd);
		while (cmd == "}") {
			while (!bracelevels.empty() && indentlev > bracelevels.back()) {
				outp.append(--indentlev, '\t');
				outp.push_back('}');
				outp.push_back('\n');
			}
			if (!bracelevels.empty()) {
				bracelevels.pop_back();
			}
			if (it == tokens.end()) break;
			cmd = *(it++);
			if (cmd == "" && it != tokens.end()) cmd = *(it++);
			chkcmd = to_upper(cmd);
		}
		while (cmd == "" && it != tokens.end()) {
			cmd = *(it++);
			chkcmd = to_upper(cmd);
		}
		if (cmd == "") break;
		outp.append(indentlev, '\t');
		if (part_block.contains(chkcmd)) {
			while (outp.back() != '}') outp.pop_back();
			outp.push_back(' ');
		}
		outp.append(cmd);
		if (it == tokens.end()) break;
		string exec = *(it++);
		if (exec == "}") {
			--it;
			exec = "";
		} else {
			outp.push_back(' ');
		}
		if (part_block.contains(chkcmd) && no_param_brace.contains(chkcmd)) {
			if (exec == "") {
				if (it != tokens.end() && *it == "{") ++it;
			} else if (exec != "{") {
				--it;
			}
			outp.push_back('{');
			bracelevels.push_back(indentlev++);
		} else {
			outp.append(exec);
			if (brace_cmds.contains(chkcmd) || no_param_brace.contains(chkcmd)) {
				if (it != tokens.end() && *it == "") ++it;
				if (it != tokens.end() && *it == "{") {
					bracelevels.push_back(indentlev);
					++it;
				}
				if (!no_param_brace.contains(cmd)) outp.push_back(' ');
				outp.push_back('{');
				++indentlev;
			} else if (exec == "" && it != tokens.end() && *it != "}") outp.pop_back();
		}
		outp.push_back('\n');
	}
	while (indentlev > 1) {
		outp.insert(outp.end(), --indentlev, '\t');
		outp.push_back('}');
		outp.push_back('\n');
	}
	if (outp.back() == '\n') outp.pop_back();
	return outp;
}

string collapse_tag(string inp) {
	/*Explanation
	* See expand_tag for an explanation of how MUMPS/Objectscript is parsed.  Important to this function, however, is that the commands
	* in the no_param_cmds set, and only those commands, can be on a line by themselves (with no additional text follwoing them).  Other
	* commands either require a parameter or a code block, which for this function can be assumed to be encapsulated within {} characters
	* or to not be empty.
	* 
	* The reason this is important is because it means that, with exception of the no_param_cmds, we can essentially replace the line
	* breaks with spaces and everything will work.  For the no_param_cmds, though, we need to insert an extra space if there's not one
	* on the line already in order for it to work.
	* 
	* The goal of this function is to take a body of code that's been written with proper code blocks and, optionally, indentation and
	* collapse it down to one line so that it can be pasted into a terminal session.  In the interest of maintaining the terminal session
	* history as short as possible, this does also mean that we can exploit the fact that code blocks are assumed to contain everything
	* following the branching command on the same line, and remove any {} characters encapsulating a code block that terminates another
	* code block.  We also want to remove trailing spaces at the end of a code block.  For example (note the mix of tabs and spaces):
	*	N
	*	I TST {
	*		F I=START:STEP:END {
	*		    D STUFF
	*		    I ANOTHER_TST {
	*		        Q
	*		    }
	*		}
	*	}
	*	D MORE_STUFF
	* Can be collapsed to:
	*     N  I TST {F I=START:STEP:END D STUFF I ANOTHER_TST Q} D MORE_STUFF
	*/
	
	unordered_set<string> no_param_cmds {"N", "NEW", "Q", "QUIT"};
	vector<string> tokens {""};
	bool newline_mode = true, in_quotes = false, seen_space = false;
	for (const auto &it : inp) {
		switch (it) {
		case '\n':
			if (!in_quotes) {
				if (!seen_space && !tokens.back().empty() && no_param_cmds.contains(to_upper(tokens.back()))) tokens.back().push_back(' ');
				newline_mode = true;
				seen_space = false;
				tokens.push_back("");
			}
		case ' ':
			if (!newline_mode && !in_quotes) seen_space = true;
		case '\t':
			if (!newline_mode || in_quotes) tokens.back().push_back(it);
			break;
		case '"':
			in_quotes = !in_quotes;
		default:
			tokens.back().push_back(it);
			newline_mode = false;
		}
	}
	if (!rem_braces(tokens)) return "Brace mismatch detected!\nUnable to continue...";
	string outp;
	outp.reserve(inp.length());
	for (string &it : tokens) {
		if (it.empty()) continue;
		if (it.back() != '{') it.push_back(' ');
		if (it[0] == '}') {
			while (!outp.empty() && outp.back() == ' ') {
				outp.pop_back();
			}
		}
		outp.insert(outp.end(), it.begin(), it.end());
	}
	while (!outp.empty() && outp.back() == ' ') {
		outp.pop_back();
	}
	return outp;
}

string build_loop(string inp) {
	enum class state {
		ST,
		SU,
		QT,
		ED
	};
	vector<string> subs;
	vector<pair<string, size_t>> vars;
	string base = inp[0] == '^' ? "" : "^";
	state stat = state::ST;
	size_t lst = 0, inplen = inp.length();
	for (size_t i = 0; i < inplen; ++i) {
		char chr = inp[i];
		switch (stat) {
		case state::ST:
			if (chr == '(') {
				base.append(move(inp.substr(lst, i)));
				lst = i + 1;
				stat = state::SU;
			}
			break;
		case state::QT:
			if (chr == '"') {
				stat = state::SU;
			}
			break;
		case state::SU:
			if (chr == '"') {
				stat = state::QT;
			} else if (chr == ',' || chr == ')') {
				string sub = inp.substr(lst, i - lst);
				subs.push_back(sub);
				if (!is_numeric(sub) && (sub[0] != '"' || sub.back() != '"')) vars.push_back({sub, subs.size() - 1});
				lst = i + 1;
				if (chr == ')') {
					stat = state::ED;
				}
			}
		}
	}
	if (stat != state::ED) {
		string sub;
		switch (stat) {
		case state::QT:
			inp += '"';
		case state::SU:
			sub = inp.substr(lst);
			subs.push_back(sub);
			if (!is_numeric(sub) && (sub[0] != '"' || sub.back() != '"')) vars.push_back({sub, subs.size() - 1});
		}
	}
	if (vars.empty()) {
		return base + "(" + join(subs, ',') + ")";
	}
	if (vars.size() == 1) {
		string var;
		size_t ind;
		tie<string, size_t>(var, ind) = vars[0];
		return "S " + var + "=\"\" F  S " + var + "=$O(" + base + "(" + join(subs.begin(), subs.begin() + ind, ',', true) + var + ")) Q:" + var + "=\"\"  ";
	}
	string var, outp;
	size_t ind;
	tie<string, size_t>(var, ind) = vars[0];
	outp = "S " + var + "=\"\" F  S " + var + "=$O(" + base + "(" + join(subs.begin(), subs.begin() + ind, ',', true) + var + "))," + vars[1].first + "=\"\" Q:" + var + "=\"\"  ";
	for (vector<pair<string, size_t>>::iterator it = vars.begin() + 1; it != vars.end(); ) {
		tie<string, size_t>(var, ind) = *(it++);
		if (it == vars.end()) {
			outp += "F  S " + var + "=$O(" + base + "(" + join(subs.begin(), subs.begin() + ind, ',', true) + var + ")) Q:" + var + "=\"\"  ";
		} else {
			string nxt = (*it).first;
			outp += "F  S " + var + "=$O(" + base + "(" + join(subs.begin(), subs.begin() + ind, ',', true) + var + "))," + nxt + "=\"\" Q:" + var + "=\"\"  ";
		}
	}
	return outp;
}


// These next few functions could probably be independent, but I want them to all work the same way (essentially) and this is the easiest way to do that

void perform_update(ExtraFunctions_Frame *frm, char flg) {
	string err, outp, inp = frm->input->GetValue().ToStdString();
	if (inp == "") {
		frm->output->SetValue("");
		return;
	}
	try {
		switch (flg) {
		case 0:
			err = "fix tag";
			outp = fix_tag(inp);
			break;
		case 1:
			err = "expand tag";
			outp = expand_tag(inp);
			break;
		case 2:
			err = "collapse tag";
			outp = collapse_tag(inp);
			break;
		case 3:
			err = "build loop";
			outp = build_loop(inp);
			break;
		case 4:
			err = "calculate bitmap";
			outp = bitmap_calc_long(inp);
		}
		frm->output->SetValue(outp);
		frm->output->SetFocus();
		frm->output->SelectAll();
	} catch (...) {
		frm->output->SetValue("Unable to " + err + "!");
	}
}

void ExtraFunctions_Frame::DoFixTag(wxCommandEvent &evt) {
	perform_update(this, 0);
	evt.Skip();
}

void ExtraFunctions_Frame::DoExpandTag(wxCommandEvent &evt) {
	perform_update(this, 1);
	evt.Skip();
}

void ExtraFunctions_Frame::DoCollapseTag(wxCommandEvent &evt) {
	perform_update(this, 2);
	evt.Skip();
}

void ExtraFunctions_Frame::DoBuildLoop(wxCommandEvent &evt) {
	perform_update(this, 3);
	evt.Skip();
}

void ExtraFunctions_Frame::DoBitmapCalc(wxCommandEvent &evt) {
	perform_update(this, 4);
	evt.Skip();
}

ExtraFunctions_Frame::ExtraFunctions_Frame(MainFrame *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : ChildFrame(parent, id, title, pos, size, style) {
	wxMenuBar* menu = GetMenuBar();
	wxMenu* help_menu = new wxMenu();
	wxMenuItem* m_menuItem1 = new wxMenuItem(help_menu, ID_Help_FixTag, "&Fix tag");
	wxMenuItem* m_menuItem2 = new wxMenuItem(help_menu, ID_Help_ExpandTag, "&Expand tag");
	wxMenuItem* m_menuItem3 = new wxMenuItem(help_menu, ID_Help_CollapseTag, "&Collapse tag");
	wxMenuItem* m_menuItem4 = new wxMenuItem(help_menu, ID_Help_BuildLoop, "&Build loop");
	wxMenuItem* m_menuItem5 = new wxMenuItem(help_menu, ID_Help_Bitmap, "C&alculate bitmaps");
	help_menu->Append(m_menuItem1);
	help_menu->Append(m_menuItem2);
	help_menu->Append(m_menuItem3);
	help_menu->Append(m_menuItem4);
	help_menu->Append(m_menuItem5);
	menu->Append(help_menu, "&Help");
	Bind(wxEVT_MENU, [](wxCommandEvent& evt) {wxMessageBox("Use this function to correct a tag (or any string) which has been inappropriately wrapped by SmarTerm's screen wrapping\n \nThe function assumes 80 characters line length, and will remove the extraneous line breaks from the input, filling in spaces as needed", "Fix tag", wxICON_INFORMATION); }, ID_Help_FixTag);
	Bind(wxEVT_MENU, [](wxCommandEvent& evt) {wxMessageBox("Use this function to expand a tag from a single line to multi-line studio-style format", "Expand tag", wxICON_INFORMATION); }, ID_Help_ExpandTag);
	Bind(wxEVT_MENU, [](wxCommandEvent& evt) {wxMessageBox("Use this function to collapse a tag from multi-line studio-style format to a single line", "Collapse tag", wxICON_INFORMATION); }, ID_Help_CollapseTag);
	Bind(wxEVT_MENU, [](wxCommandEvent& evt) {wxMessageBox("Use this function to take a global reference and create a set of nested for loops to iterate to the provided global level\n \nFor example, and input of:\n  GBL(1,VAR,\"Q\",0,VARTWO\nwill produce the following output:\n  S VAR=\"\" F  S VAR=$O(^GBL(1,VAR),VARTWO=\"\" Q:VAR=\"\"  F  S VARTWO=$O(^GBL(1,VAR,\"Q\",0,VARTWO)) Q:VARTWO=\"\"  ", "Build loop", wxICON_INFORMATION); }, ID_Help_BuildLoop);
	Bind(wxEVT_MENU, [](wxCommandEvent& evt) {wxMessageBox("Use this function to determine which bits are set in a bitmap", "Calculate bitmaps", wxICON_INFORMATION); }, ID_Help_Bitmap);

	wxBoxSizer *extframe_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *io_sizer = new wxBoxSizer(wxVERTICAL);

	input = new Main_TextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_BESTWRAP);
	output = new Invisible_TextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_BESTWRAP);
	wxStaticLine *separator = new wxStaticLine(this, wxID_ANY);
	io_sizer->Add(input, 1, wxALL | wxEXPAND, default_border);
	io_sizer->Add(separator, 0, wxEXPAND);
	io_sizer->Add(output, 1, wxALL | wxEXPAND, default_border);
	extframe_sizer->Add(io_sizer, 1, wxEXPAND, default_border);

	wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);

	// These buttons have accelerators but they don't work.  I haven't been able to figure out why though.

	wxButton *fix_tag = new wxButton(this, wxID_ANY, "&Fix tag");
	wxButton *expand_tag = new wxButton(this, wxID_ANY, "&Expand tag");
	wxButton *collapse_tag = new wxButton(this, wxID_ANY, "&Collapse tag");
	wxButton *build_loop = new wxButton(this, wxID_ANY, "&Build loop");
	wxButton *bitmap_calc = new wxButton(this, wxID_ANY, "C&alculate Bitmaps");
	wxButton *close = new wxButton(this, wxID_EXIT);
	button_sizer->Add(fix_tag, 1, wxALL | wxEXPAND, default_border);
	button_sizer->Add(expand_tag, 1, wxALL | wxEXPAND, default_border);
	button_sizer->Add(collapse_tag, 1, wxALL | wxEXPAND, default_border);
	button_sizer->Add(build_loop, 1, wxALL | wxEXPAND, default_border);
	button_sizer->Add(bitmap_calc, 1, wxALL | wxEXPAND, default_border);
	// Add an empty space between Close and the rest of the buttons
	button_sizer->Add(0, 0, 1, wxEXPAND, default_border);
	button_sizer->Add(close, 1, wxALL | wxEXPAND, default_border);
	extframe_sizer->Add(button_sizer, 0, wxEXPAND, default_border);

	SetSizer(extframe_sizer);

	fix_tag->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::DoFixTag, this);
	expand_tag->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::DoExpandTag, this);
	collapse_tag->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::DoCollapseTag, this);
	build_loop->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::DoBuildLoop, this);
	bitmap_calc->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::DoBitmapCalc, this);
	close->Bind(wxEVT_BUTTON, &ExtraFunctions_Frame::OnClose, this);

	EndCreation();
}

bool is_numeric(string inp, bool include_fractions) {
	bool seen_dot = !include_fractions;
	for (const char &it : inp) {
		switch (it) {
		case '.':
			if (seen_dot) return false;
			seen_dot = true; [[fallthrough]];
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
			break;
		default:
			return false;
		}
	}
	return true;
}

string join(vector<string>::iterator start, vector<string>::iterator end, char delim, bool prim) {
	string outp, lst;
	if (!prim) lst = *(end--);
	for (vector<string>::iterator it = start; it != end; ++it) {
		outp.append(*it);
		outp.push_back(delim);
	}
	outp.append(lst);
	return outp;
}

string join(vector<string> inp, char delim, bool prim) {
	string outp, lst;
	if (!prim) {
		lst = inp.back();
		inp.pop_back();
	}
	for (const string &it : inp) {
		outp.append(it);
		outp.push_back(delim);
	}
	outp.append(lst);
	return outp;
}

vector<string>tokenize_tag(string inp) {
	vector<string> outp {""};
	bool in_quotes = false;
	for (const char &it : inp) {
		switch (it) {
		case '{':
		case '}':
			if (!in_quotes) outp.push_back(string(1, it)); [[fallthrough]];
		case ' ':
			if (!in_quotes) {
				outp.push_back("");
			} else {
				outp.back().push_back(it);
			}
			break;
		case '"':
			in_quotes = !in_quotes; [[fallthrough]];
		default:
			outp.back().push_back(it);
		}
	}
	return outp;
}

bool pop_brace(vector<string> &inp, size_t &i, vector<size_t> &braces, unordered_set<size_t> &req_braces) {
	/*
	* inp        = a multi-line tag that's been cleaned up a little and split into individual strings per line
	* i          = the line number containing the closing brace under consideration for popping
	* braces     = a vector of line numbers which have been found to contain open braces
	* req_braces = a set of braces which have been found to be required to maintain functionality
	*/
	// If braces.empty(), there are more closing than opening braces - return false
	if (braces.empty()) return false;
	// if something else is on the line or the brace is required don't pop the characters
	// in either case, pop from the braces vector
	size_t j = braces.back(); // The line number for the last opening brace
	if (j == i) {
		// if an ending brace on the same line as an open brace, the braces won't be popped
		// update the previous opening brace to this one
		j = braces[braces.size() - 2];
		braces[braces.size() - 2] = i;
	}
	if (!req_braces.contains(j)) {
		if (inp[i] == "}") {
			// If it's the last line, the } isn't needed
			if (i >= (inp.size() - 1)) {
				inp[i].pop_back();
				inp[j].pop_back();
				if (inp[j].back() == ' ') inp[j].pop_back();
			} else if (inp[i + 1].front() == '}') { // If it's not the last line but the next line character is }, the } isn't needed
				inp[i].pop_back();
				inp[j].pop_back();
				if (inp[j].back() == ' ') inp[j].pop_back();
			}
		}
	}
	braces.pop_back();
	return true;
}

bool rem_braces(vector<string> &inp) {
	vector<size_t> braces {};
	unordered_set<string> brace_cmds {"ELSE","CATCH","ELSEIF","WHILE","TRY"};
	unordered_set<size_t> req_braces {};
	for (size_t i = 0; i < inp.size(); i++) {
		if (inp[i].back() == '{') {
			// here check for brace commands and mark as required
			string cmd = inp[i];
			
			// this next bit is just standard pop } logic; there's something else on the line so the braces shouldn't be popped
			// let pop_brace handle that though

			// These next two lines isolate just the command preceding the { (not any parameters)
			if (inp[i].front() == '}') cmd = cmd.substr(cmd[1] == ' ' ? 2 : 1);
			cmd = cmd.substr(0, cmd.find(' '));

			cmd = to_upper(cmd);
			if (brace_cmds.contains(cmd)) req_braces.insert(i);
			braces.push_back(i);
		}
		if (inp[i].front() == '}') {
			// pop_brace takes care of ensuring we don't accidentally pop braces that are needed
			if (!pop_brace(inp, i, braces, req_braces)) return false;
		}
	}
	// If opening brace vector isn't empty, there's more opening than closing braces - return false
	if (!braces.empty()) return false;
	return true;
}
