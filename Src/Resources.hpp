// *****************************************************************************
// Source code for the Navigator Test Add-On
// API Development Kit 23; Mac/Win
//
// Namespaces:			Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#ifndef RESOURCES_HPP
#define RESOURCES_HPP

namespace GS {
	class UniString;
}

#define	NAVIGATOR_TEST_ADDON_NAME					32000	//'STR#' 32000
#define	NAVIGATOR_TEST_RESID_LOCSTRINGS				32300	//'STR#' 32300
#define	NAVIGATOR_TEST_MENU_STRINGS					32500	//'STR#' 32500

#define NAVIGATOR_TEST_PALETTE_ID					32420	//'GDLG' 32420

enum NavigatorTestMenuIds : short {
	NavigatorTestOpenNavigatorId	= 1,
	NavigatorTestCreateChildGroupId	= 2
};

enum NavigatorPalettePopUpMenuMapIds : short {
	PopupMenuProjectMapItemId		= 1,
	PopupMenuPublicViewItemId		= 2,
	PopupMenuLayoutMapItemId		= 3,
	PopupMenuPublisherSetsItemId	= 4
};

enum NavigatorPaletteControlIds : short {
	NavCloseButton		= 1,
	NavUpdateButton		= 2,
	NavDeleteButton		= 3,
	NavPublishButton	= 4,
	NavTreeView			= 5,
	LayerText			= 6,
	ScaleText			= 7,
	ZoomText			= 8,
	DisplayOpText		= 9,
	D3WindowStyleText	= 10,
	NavPopUp			= 11,
	NavCustomNameText	= 12,
	NavRenameButton		= 13
};

enum NavigatorPaletteIconIds : short {
	BookIcon			= 32408,
	ProjectLinkIcon		= 32409,
	StoryLinkIcon		= 32410,
	SectionLinkIcon		= 32411,
	DetailLinkIcon		= 32412,
	LayoutLinkIcon		= 32413,
	MasterLinkIcon		= 32414,
	Axon3DLinkIcon		= 32415,
	Persp3DLinkIcon		= 32416,
	ScheduleLinkIcon	= 32417,
	ListIcon			= 32418,
	HelpIcon			= 32419,
	OtherIcon			= 32420,
	MasterFolderIcon	= 32421,
	WorksheetLinkIcon	= 32422,
	SubsetIcon			= 32423,
	TOCIcon				= 32424,
	CameraIcon			= 32425,
	CameraSetIcon		= 32426,
	InfoIcon			= 32427,
	ElevationIcon		= 32428,
	IntElevationIcon	= 32429,
	DocumentFrom3DIcon	= 32430
};

enum NavigatorAPILocalisedStringIds : short {
	RootGroupNameStringId	= 1,
	ChildGroupNameStringId	= 2
};

enum NavigatorAPIIconIds : short {
	NodeIconID				= 32600,
	NodeLinkIconID			= 32601,
	GroupIconID				= 32602,
	GroupLinkIconID			= 32603,
	NodeReservedIconID		= 32604,
	GroupReservedIconID		= 32605,
	NodeBeforeSendIconID	= 32606,
	GroupBeforeSendIconID	= 32607,
	NodeLinkTWIconID		= 32608,
	GroupLinkTWIconID		= 32609,
};


GS::UniString GetLocStr (NavigatorAPILocalisedStringIds stringId);

#endif