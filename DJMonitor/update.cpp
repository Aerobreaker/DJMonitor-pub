#include "update.hpp"

#include "globals.hpp"
#include "functions.hpp"

#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <filesystem>
#include <system_error>
#include <fstream>

using namespace std;

// There's probably a better way to do this...but I don't know it.

bool auto_update(string updatepath) {
	// Return true if the exe has been updated and needs to close, false otherwise
	string current_path = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	string current_name = current_path, versname = "[" + string(version) + "]";
	size_t name_end = current_name.rfind(EXT);
	// Remove the extension
	if (name_end != string::npos) {
		size_t i = name_end, curname_len = current_name.length(), vername_len = versname.length();
		for (; i < curname_len; curname_len--) current_name.pop_back();
		i -= vername_len;
		for (; i < name_end; ++i) if (current_name[i] != versname[i - name_end + vername_len]) break;
		// This means the filename ends with the current version - it's expressly running outdated; don't relaunch
		if (i == name_end) return false;
	}
	ifstream updatefile(updatepath);
	if (updatefile) {
		string vers {""};
		getline(updatefile, vers);
		if (!vers.empty() && vers.back() == '\r') vers.pop_back();
		while (!updatefile.eof() && (updatefile.peek() == '\r' || updatefile.peek() == '\n')) updatefile.get();
		if (!cmp_version(version, vers)) {
			// Current version is less than the one in the file
			vers += ARCHITECTURE;
			string versnum, verspath, cmd, args, base_path = filesystem::path(updatepath).parent_path().string();;
			for (int i = 1; i < current_app->argc; i++) args += " " + current_app->argv[i].ToStdString();
			while (!updatefile.eof() && (updatefile.peek() == '\r' || updatefile.peek() == '\n')) updatefile.get();
			while (versnum != vers && !updatefile.eof()) {
				getline(updatefile, versnum, '\t');
				getline(updatefile, verspath);
				if (!verspath.empty() && verspath.back() == '\r') verspath.pop_back();
				while (!updatefile.eof() && (updatefile.peek() == '\r' || updatefile.peek() == '\n')) updatefile.get();
			}
			if (versnum != vers) {
				if (!current_app->noupdate) wxMessageBox("Encountered a problem locating the most up-to-date version from the update file!\n \nContinuing with outdated version.", "Update failed", wxOK | wxICON_EXCLAMATION | wxCENTER);
			}
			size_t baselen = base_path.length(), curlen = current_path.length(), i = 0;
			for (; i < baselen && i < curlen; ++i) if (current_path[i] != base_path[i]) break;
			verspath = base_path + verspath;
			if (i < baselen) {
				// If we broke before running out the base path
				// then we're not running in base dir - update
				error_code ec;
				filesystem::rename(current_path, current_name + versname + EXT, ec);
				if (ec != error_code()) {
					if (!current_app->noupdate) wxMessageBox("Failed to update monitor!\n \n" + ec.message() + "\n \nContinuing with outdated version.", "Update failed", wxOK | wxICON_EXCLAMATION | wxCENTER);
					return false;
				}
				if (!filesystem::copy_file(verspath, current_path, ec)) {
					if (!current_app->noupdate) wxMessageBox("Failed to update monitor!\n \n" + ec.message() + "\n \nContinuing with outdated version.", "Update failed", wxOK | wxICON_EXCLAMATION | wxCENTER);
					filesystem::rename(current_name + versname + EXT, current_path);
					// If we can't update, don't relaunch
					return false;
				}
				cmd = current_path;
			} else {
				// If we're running in the base path, just launch the updated version
				cmd = verspath;
			}
			wxLogNull *logNo = new wxLogNull;
			wxMessageOutput *msgout = wxMessageOutput::Set(new wxMessageOutputLog);
			long status = wxExecute("\"" + cmd + "\"" + args);
			delete wxMessageOutput::Set(msgout);
			delete logNo;
			if (!status) {
				if (!current_app->noupdate) wxMessageBox("Failed to launch updated monitor.\n \nContinuing with outdated version.", "Update failed", wxOK | wxICON_EXCLAMATION | wxCENTER);
				// If we couldn't launch the updated version, don't close
				return false;
			} // we miiiiight also get a -1, buuuuut it's something about a process attached to mine under Windows DDE so I'm not exactly sure what it means
			if (current_app->noupdate) {
				wxMessageBox("Update succeeded!\n \nClosing outdated version", "Update succeeded", wxOK | wxICON_INFORMATION | wxCENTER);
			}
			return true;
		} else if (current_app->noupdate) current_app->noupdate = false;
	} else if (!current_app->noupdate) {
		wxMessageBox("Failed to open update file (" + updatepath + ").\n \nContinuing with outdated version.", "Update failed", wxOK | wxICON_ERROR | wxCENTER);
		current_app->noupdate = true;
	}
	// If we can't open the file or didn't update because the version is high enough, don't relaunch
	return false;
}