#pragma once

#include <wx/defs.h>

#ifdef _DEBUG
constexpr char version[] = "3.0.2.beta.0";
#else
constexpr char version[] = "3.0.2";
#endif
constexpr char min_version[] = "3.0.alpha";

constexpr char version_date[] = " (October 2021)";

enum {
	ID_OnTop = wxID_HIGHEST + 1,
	ID_Overrides,
	ID_SavePos,
	ID_TZOffs,
	ID_SystemTray,
	ID_CustomFont,
	ID_CustomColor,
	ID_CustomFormat,
	ID_RestoreParent,
	ID_FindParent,
	ID_HideParent,
	ID_Help_OnTop,
	ID_Help_Overrides,
	ID_Help_Override_Date,
	ID_Help_Override_Time,
	ID_Help_Override_Date_Type,
	ID_Help_SavePos,
	ID_Help_TZOffs,
	ID_Help_SystemTray,
	ID_Help_CustomFont,
	ID_Help_CustomColor,
	ID_Help_CustomFormat,
	ID_Bitmap,
	ID_FixTag,
	ID_ExpandTag,
	ID_CollapseTag,
	ID_BuildLoop,
	ID_Help_Bitmap,
	ID_Help_FixTag,
	ID_Help_ExpandTag,
	ID_Help_CollapseTag,
	ID_Help_BuildLoop,
	ID_ShowExtras,
	ID_CloseExtras,
	ID_UpdateSiteDict,
	ID_Help_UpdateSiteDict,
	ID_Show_Bitmap,
	ID_Help_Small_Bitmap,
	ID_Help_Show_Bitmap,
	ID_Child_OnTop,
	ID_ShowPieces,
	ID_ClosePieces,
	ID_LeftClick,
	ID_Help_LeftClick,
	ID_Help_Piece_Data,
	ID_Help_Piece_Delim,
	ID_Help_Piece_Search,
	ID_Help_Piece_Piece,
	ID_Custom_FontCol,
	ID_Help_Custom_FontCol
};

class MainFrame;
class ChildFrame;
class ExtraFunctions_Frame;
class PieceMan_Frame;
class Main_TextCtrl;

constexpr unsigned char def_white_layer = 75;
