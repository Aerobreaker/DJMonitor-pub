#pragma once

#include "all_header.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/app.h>

const extern std::string appdatadir;
const extern std::string settingdir;
const extern std::string settingfile;
const extern std::string tz_dir;

extern std::unordered_map<std::string, std::string> *site_map;
extern wxCommandEvent menu_evt;

class MainApp : public wxApp {
private:
	wxLogNull *logNo = nullptr;
	wxMessageOutput *msgout;
	bool hidden = false;
public:
	MainFrame *frame = nullptr;
	wxString updatepath {""};
	long buffer = -1;
	long white_layer = -1;
	bool noupdate = false;
	virtual bool OnInit();
	virtual int OnRun();
	virtual int OnExit();
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
	virtual bool OnCmdLineError(wxCmdLineParser &parser);
	int FilterEvent(wxEvent &evt) {
		// special event handling goes here
		return wxApp::FilterEvent(evt);
	};
};
extern MainApp *current_app;

constexpr char default_border = 5;

extern std::unordered_set<wxFrame *> app_frames;
