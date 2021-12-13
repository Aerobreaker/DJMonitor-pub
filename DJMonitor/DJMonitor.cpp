#include "all_header.hpp"

#include "globals.hpp"
#include "window.hpp"
#include "update.hpp"

#include <wx/cmdline.h>
#include <wx/log.h>
#include <filesystem>
#include <sstream>
#include <fstream>

/*
TODO:
	1. Create an option to start with windows?  It's already possible by creating a shortcut with the /h command line parameter and placing it into "%appdata%\Microsoft\Windows\Start Menu\Programs\Startup", but I could make something to put it into the registry (which seems to be more reliable)
	2. Add timer and/or stopwatch functions?  For the timer, I could restore the window and flash it, and for the stopwatch, I'd have like mark, lap, and stop functions and report out the times when it stops.  I could put these on the system tray icon too.
	3. If adding window flashing &c with a timer function, add an alarm clock function that does the same thing?
*/

using namespace std;

const string appdatadir = getenv("APPDATA");
const string settingdir = (appdatadir == "" ? filesystem::temp_directory_path().string() : appdatadir) + "\\DJMonitor";
const string settingfile = settingdir + "\\settings.djm";
const string tz_dir = settingdir + "\\timezones";
unordered_set<wxFrame *> app_frames;

MainApp *current_app;

wxIMPLEMENT_APP(MainApp);

bool MainApp::OnInit() {
	if (!wxApp::OnInit()) return false;
	datetime::set_install_dir(tz_dir);
	ifstream setfil {settingfile};
	if (setfil.is_open()) {
		string noupd;
		setfil >> noupd;
		if (noupd == "NoUpdate") {
			noupdate = true;
		}
		setfil.close();
	}
	current_app = this;
	return true;
}

int MainApp::OnRun() {
	if (auto_update(updatepath.ToStdString())) Exit();
	// Usually, the top window would be created in OnInit
	// But I don't want the timezones to be downloaded before we check for updates
	frame = new MainFrame(NULL);
	SetTopWindow(frame);
	if (!hidden || !frame->m_system_tray->IsChecked()) frame->Show(true);
	if (buffer > -1) frame->buffer = buffer % 255;
	if (white_layer > -1) frame->white_layer = white_layer % 255;
	return wxApp::OnRun();
}

int MainApp::OnExit() {
	datetime::clear_cache();
	delete current_app;
	return 0;
}

// There's probably a better way to do this that keeps the two parsers in sync...

static const wxCmdLineEntryDesc cmdLineArgsShown[] = {
	{wxCMD_LINE_SWITCH, "h", "Hidden", "Start the monitor hidden (only works if minimized to system tray)"},
	{wxCMD_LINE_OPTION, "u", "UpdatePath", "Overwrite the default directory to check for updates (in case it moved)"},
	{wxCMD_LINE_OPTION, "w", "WhiteLayer", "Set the white mask to be applied to text entry fields when a custom background color is used", wxCMD_LINE_VAL_NUMBER},
	{wxCMD_LINE_NONE}
};

static const wxCmdLineEntryDesc cmdLineArgsReal[] = {
	{wxCMD_LINE_SWITCH, "h", "Hidden"},
	{wxCMD_LINE_OPTION, "u", "UpdatePath"},
	{wxCMD_LINE_OPTION, "w", "WhiteLayer", "", wxCMD_LINE_VAL_NUMBER},
	{wxCMD_LINE_OPTION, "b", "ScreenBuffer", "", wxCMD_LINE_VAL_NUMBER},
	{wxCMD_LINE_NONE}
};

void MainApp::OnInitCmdLine(wxCmdLineParser &parser) {
	// Run the args through the real parser
	// If it fails, switch to the show parser
	// This way, we can "hide" command-line args while still giving usage info for the non-hidden args
	wxCmdLineParser tmp {cmdLineArgsReal, argc, argv};
	if (tmp.Parse(false) != 0) {
		parser.SetDesc(cmdLineArgsShown);
	} else {
		parser.SetDesc(cmdLineArgsReal);
	}
	// Disable logging and messaging while we check for unknown arguments
	logNo = new wxLogNull;
	msgout = wxMessageOutput::Set(new wxMessageOutputLog);
}

bool MainApp::OnCmdLineParsed(wxCmdLineParser &parser) {
	// Re-enable logging and messaging
	delete wxMessageOutput::Set(msgout);
	delete logNo;
	hidden = parser.Found("h");
	/*META-COMMENT:
	* Default update locations have been changed to protect potentially proprietary information
	*/
#ifdef _DEBUG
	// Network beta location
	if (!parser.Found("u", &updatepath)) updatepath = "C:\\Users\\public\\Documents\\DJ_Monitor\\Beta\\VersionList.txt";
#else
	if (!parser.Found("u", &updatepath)) updatepath = "C:\\Users\\public\\Documents\\DJ_Monitor\\VersionList.txt";
#endif
	if (!parser.Found("w", &white_layer)) white_layer = -1;
	if (!parser.Found("b", &buffer)) buffer = -1;
	return true;
}

bool MainApp::OnCmdLineError(wxCmdLineParser &parser) {
	// Re-enable logging and messaging
	delete wxMessageOutput::Set(msgout);
	delete logNo;
	return wxApp::OnCmdLineError(parser);
}
