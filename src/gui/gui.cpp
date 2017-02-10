#include "gui.h"

#include "../application.h"


Gui::Gui(Application &aapp) : app_ref(aapp)
{
	mode = Application::entries_mode::MODE_UNREAD;
}


Gui::~Gui()
{
	CloseFont(titleFont);
	CloseFont(smallTitleFont);
	CloseFont(entryTitleFont);
	CloseFont(entryInfosFont);
	CloseFont(statusBarFont);
}


// There are things (like opening a font?) it seems we cannot do in the ctor: the ereader is not ready yet...
// So, we need another initialization method, called later -- like when the ereader is ready to let us use its API...
void Gui::init()
{
	OpenScreen();
	iv_fullscreen();

	// Force a black-flash, to fully redraw the screen -- so we don't see Home screen anymore
	FullUpdate();

	// Allows to effectively be full-screen, even when redrawing
	SetCurrentApplicationAttribute(APPLICATION_READER, 1);

	titleFontSize = 52;
	titleFont = OpenFont("LiberationSans", titleFontSize, 1);
	if (titleFont == NULL) {
		ERROR("%s titleFont is NULL", __PRETTY_FUNCTION__);
	}

	smallTitleFontSize = 28;
	smallTitleFont = OpenFont("LiberationSans", smallTitleFontSize, 1);
	if (smallTitleFont == NULL) {
		ERROR("%s smallTitleFont is NULL", __PRETTY_FUNCTION__);
	}

	entryTitleFontSize = 34;
	entryTitleFont = OpenFont("LiberationSans", entryTitleFontSize, 1);
	if (entryTitleFont == NULL) {
		ERROR("%s entryTitleFont is NULL", __PRETTY_FUNCTION__);
	}

	entryInfosFontSize = 24;
	entryInfosFont = OpenFont("LiberationMono", entryInfosFontSize, 1);
	if (entryInfosFont == NULL) {
		ERROR("%s entryInfosFont is NULL", __PRETTY_FUNCTION__);
	}

	statusBarFontSize = 22;
	statusBarFont = OpenFont("LiberationSans", statusBarFontSize, 1);
	if (statusBarFont == NULL) {
		ERROR("%s statusBarFont is NULL", __PRETTY_FUNCTION__);
	}

	screenWidth = ScreenWidth();
	screenHeight = ScreenHeight();

	exitButton.setCoordinates(5, 5+2, 55, 60+2);
	exitButton.setFont(titleFont);
	exitButton.setSymbol(ARROW_LEFT);

	syncButton.setCoordinates(screenWidth-135, 5+2, 55, 60+2);
	syncButton.setFont(titleFont);
	syncButton.setSymbol(ARROW_UPDOWN);

	menuButton.setCoordinates(screenWidth-60, 5+2, 55, 60+2);
	menuButton.setFont(titleFont);
	menuButton.setSymbol(SYMBOL_MENU);
}


void Gui::show(int numPage, int numberOfPages, int countAllEntries, std::vector<Entry> &entries)
{
	char buffer[2048];

	iv_fullscreen();
	SetCurrentApplicationAttribute(APPLICATION_READER, 1);

	SetClip(0, 0, screenWidth, screenHeight);
	ClearScreen();

	int y = 0;


	// Chrome : title, button to sync, ...

	exitButton.forceRedraw();
	syncButton.forceRedraw();
	menuButton.forceRedraw();

	exitButton.draw(false);
	syncButton.draw(false);
	menuButton.draw(false);

	SetFont(smallTitleFont, BLACK);

	if (mode == 1) {
		snprintf(buffer, sizeof(buffer), PLOP_APPLICATION_SHORTNAME " - %s", LBL_HEADER_UNREAD_ENTRIES);
	}
	else if (mode == 2) {
		snprintf(buffer, sizeof(buffer), PLOP_APPLICATION_SHORTNAME " - %s", LBL_HEADER_ARCHIVED_ENTRIES);
	}
	else if (mode == 3) {
		snprintf(buffer, sizeof(buffer), PLOP_APPLICATION_SHORTNAME " - %s", LBL_HEADER_STARRED_ENTRIES);
	}
	else {
		snprintf(buffer, sizeof(buffer), PLOP_APPLICATION_SHORTNAME);
	}
	DrawString(90, y, buffer);
	y += 35;

	snprintf(buffer, sizeof(buffer), LBL_HEADER_PAGE_NUM, numPage, numberOfPages, countAllEntries);
	DrawString(90, y, buffer);
	y += 35;

	y += 5;

	DrawLine(0, y, screenWidth, y, BLACK);
	DrawLine(0, y + 1, screenWidth, y +1, BLACK);
	y += 2;

	PartialUpdate(0, 0, screenWidth, y);


	// List of articles in local DB


	// TODO déplacer tout ça vers une méthode de changement / chargement de page
	entriesItems.clear();
	entriesItems.resize(8);

	// Empty the whole area for all items at once
	FillArea(0, y, screenWidth, GuiListItemEntry::getHeight()*8 - 1, WHITE);

	for (unsigned int i=0 ; i<entriesItems.size() ; i++) {
		GuiListItemEntry item(entryTitleFont, entryInfosFont);

		item.setCoordinates(0, y + i*114);

		if (i < entries.size()) {
			Entry entry = entries.at(i);
			item.setEntry(entry);
		}
		entriesItems[i] = item;

		item.draw(false, false, false);

		if (i < entriesItems.size()-1 && i < entries.size()) {
			// Draw separator at the bottom of the entry, except for the last one (the status bar has its own separator at the top)
			const int heightSeparator = 2;
			const int ySeparator = y + i*item.getHeight() + item.getHeight()-heightSeparator;
			DrawLine(0, ySeparator, screenWidth, ySeparator, LGRAY);
			//PartialUpdate(0, ySeparator, screenWidth, heightSeparator);
		}
	}

	// Update screen area for all items at once
	PartialUpdate(0, y, screenWidth, GuiListItemEntry::getHeight()*8 - 1);


	if (countAllEntries == 0) {
		if (mode == 1) {
			statusBarText(LBL_STATUSBAR_NO_UNREAD_ENTRIES_USE_SYNC);
		}
		else if (mode == 2) {
			statusBarText(LBL_STATUSBAR_NO_ARCHIVED_ENTRIES_USE_MENU);
		}
		else if (mode == 3) {
			statusBarText(LBL_STATUSBAR_NO_STARRED_ENTRIES_USE_MENU);
		}
	}
	else {
		statusBarText(LBL_STATUSBAR_USE_KEYS_OR_TOUCH);
	}

	//FullUpdateHQ();
}


void Gui::touchStartEvent(int x, int y)
{
	exitButton.setPressed(false);
	syncButton.setPressed(false);
	menuButton.setPressed(false);

	if (exitButton.hit(x, y)) {
		exitButton.setPressed(true);
	}
	else if (syncButton.hit(x, y)) {
		syncButton.setPressed(true);
	}
	else if (menuButton.hit(x, y)) {
		menuButton.setPressed(true);
	}

	exitButton.draw(true);
	syncButton.draw(true);
	menuButton.draw(true);
}


// This needs to be global, so it can be accessed by the callback function of the menu ;-(
static imenu *menu;

void Gui::displayMainMenu()
{
	DEBUG("Opening main menu");

	const char *str0 = PLOP_APPLICATION_FULLNAME;
	const char *str1 = LBL_MAINMENU_MODE_UNREAD_ENTRIES;
	const char *str2 = LBL_MAINMENU_MODE_ARCHIVED_ENTRIES;
	const char *str3 = LBL_MAINMENU_MODE_STARRED_ENTRIES;
	const char *str_reset = LBL_MAINMENU_DELETE_ALL_LOCAL_DATA;
	const char *str4 = LBL_MAINMENU_ABOUT;

	menu = (imenu *)calloc(7, sizeof(imenu));

	menu[0].type = 1;
	menu[0].index = 0;
	menu[0].text = (char *)str0;
	menu[0].submenu = &menu[1];

	menu[1].type = 2;
	menu[1].index = 1;
	menu[1].text = (char *)str1;
	menu[1].submenu = &menu[2];

	menu[2].type = 2;
	menu[2].index = 2;
	menu[2].text = (char *)str2;
	menu[2].submenu = &menu[3];

	menu[3].type = 2;
	menu[3].index = 3;
	menu[3].text = (char *)str3;
	menu[3].submenu = &menu[4];

	menu[4].type = 2;
	menu[4].index = 5;
	menu[4].text = (char *)str_reset;
	menu[4].submenu = NULL;

	menu[5].type = 2;
	menu[5].index = 4;
	menu[5].text = (char *)str4;
	menu[5].submenu = NULL;

	auto callback = [](int index) {
		free(menu);

		if (index == 1) {
			app.setMode(Application::MODE_UNREAD);
		}
		else if (index == 2) {
			app.setMode(Application::MODE_ARCHIVED);
		}
		else if (index == 3) {
			app.setMode(Application::MODE_STARRED);
		}
		else if (index == 4) {
			DEBUG("Opening About dialog");

			const char *text = PLOP_APPLICATION_FULLNAME " " PLOP_VERSION_STR "\n"
					PLOP_WEBSITE_URL "\n"
					"\n"
					"%1$s\n"
					"\n"
					"%2$s\n"
					"@pascal_martin\n"
					"https://blog.pascal-martin.fr\n"
					"\n"
					"%3$s\n"
					PLOP_OPENSOURCE_URL;

			char buffer[2048];
			snprintf(buffer, sizeof(buffer), text,
				LBL_ABOUT_MAIN_DESCRIPTION,
				LBL_ABOUT_DEVELOPED_BY_PM,
				LBL_ABOUT_CONTRIBUTE
			);

			DialogSynchro(ICON_INFORMATION, PLOP_APPLICATION_FULLNAME, buffer, "OK", NULL, NULL);

			app.getGui().statusBarText(LBL_STATUSBAR_FEEL_FREE_TO_CONTRIBUTE, PLOP_OPENSOURCE_URL);

			DEBUG("Opening About dialog - closed");
		}
		else if (index == 5) {
			int result = DialogSynchro(ICON_QUESTION, "Delete all local data?", "Do you really want to delete all local data?\nYou will need to sync from server to fetch new data.\nData updated locally and not already synced to server will be lost.", "Delete local data", "Cancel", NULL);
			if (result == 1) {
				DEBUG("Deleting all local data...");
				app.deleteAllLocalData();
				DEBUG("Deleting all local data: done");

				app.getGui().statusBarText("Local data deleted. You should now run a sync to fetch data from server ;-)");
			}
			else if (result == 2) {
				app.getGui().statusBarText("Local data has been left untouched.");
			}
		}
	};

	statusBarText(LBL_STATUSBAR_MAINMENU);

	SetMenuFont(entryTitleFont);
	irect rect = GetMenuRect(menu);
	OpenMenu(menu, 0, (screenWidth-rect.w)/2, (screenHeight-rect.h)/3, callback);

	DEBUG("Opening main menu: done");
}


void Gui::touchEndEvent(int x, int y)
{
	if (exitButton.hit(x, y)) {
		statusBarText("Closing the application. Bye, see you soon ;-)");

		CloseApp();
	}
	else if (syncButton.hit(x, y)) {
		statusBarText("Launching synchronization with server...");

		app.loadRecentArticles();
	}
	else if (menuButton.hit(x, y)) {
		statusBarText("Opening the application's main menu...");

		displayMainMenu();
	}
	else {
		for (unsigned int i=0 ; i<entriesItems.size() ; i++) {
			GuiListItemEntry item = entriesItems.at(i);
			if (item.hit(x, y)) {
				if (item.hasEntry()) {
					item.draw(false, true, true);

					statusBarText("Loading reader app for entry#%d - %s...", item.getEntry().id, item.getEntry().title.c_str());

					app.read(item.getEntry());
				}
			}
		}
	}

	exitButton.setPressed(false);
	syncButton.setPressed(false);
	menuButton.setPressed(false);

	exitButton.draw(true);
	syncButton.draw(true);
	menuButton.draw(true);
}


void Gui::touchLong(int x, int y)
{
	for (unsigned int i=0 ; i<entriesItems.size() ; i++) {
		GuiListItemEntry item = entriesItems.at(i);
		if (item.hit(x, y)) {
			if (item.hasEntry()) {
				item.draw(false, true, true);

				statusBarText("Opening context menu for entry#%d - %s...", item.getEntry().id, item.getEntry().title.c_str());

				displayContextMenuOnEntry(item, x, y);
			}
		}
	}
}


static icontext_menu *contextMenu;
static Entry contextMenuEntry;

static void contextEntryOnMenuHandler(int index)
{
	free(menu);
	CloseContextMenu(contextMenu);

	if (index == 1) {
		app.getGui().statusBarText("Loading reader app for entry#%d (HTML) - %s...", contextMenuEntry.id, contextMenuEntry.title.c_str());
		app.read(contextMenuEntry, Application::FORMAT_HTML);
	}
	else if (index == 2) {
		app.getGui().statusBarText("Loading reader app for entry#%d (EPUB) - %s...", contextMenuEntry.id, contextMenuEntry.title.c_str());
		app.read(contextMenuEntry, Application::FORMAT_EPUB);
	}

	DEBUG("Closing context menu: done");

	app.show();
}

void Gui::displayContextMenuOnEntry(GuiListItemEntry &item, int xTouch, int yTouch)
{
	Entry entry = item.getEntry();
	contextMenuEntry = entry;

	DEBUG("Opening context menu on entry %d", entry.id);

	const char *id = "context-menu";
	contextMenu = CreateContextMenu(id);

	const char *str0 = entry.title.c_str();
	const char *str1 = "Read HTML content";
	const char *str2 = "Read EPUB version";

	menu = (imenu *)calloc(4, sizeof(imenu));

	int menu_idx = 0;
	menu[menu_idx].type = 1;
	menu[menu_idx].index = 0;
	menu[menu_idx].text = (char *)str0;
	menu[menu_idx].submenu = &menu[menu_idx+1];

	menu_idx++;
	menu[menu_idx].type = 2;
	menu[menu_idx].index = 1;
	menu[menu_idx].text = (char *)str1;
	menu[menu_idx].submenu = &menu[menu_idx+1];

	if (!entry.local_content_file_epub.empty()) {
		// We have a path to a local EPUB file => we can offer to open it
		menu_idx++;
		menu[menu_idx].type = 2;
		menu[menu_idx].index = 2;
		menu[menu_idx].text = (char *)str2;
		menu[menu_idx].submenu = &menu[menu_idx+1];
	}

	menu[menu_idx].submenu = NULL;
	contextMenu->menu = menu;

	contextMenu->hproc = contextEntryOnMenuHandler;

	// if flag == 0 => the marker is on the right side of the menu
	// if flag == 1 => the marker is on the left side of the menu
	// if h == 0 => automatic height to fit the content of the menu
	// if w == 0 => width is 100% of the screen
	irect_s post_menu = {x: xTouch, y: yTouch, w: screenWidth / 2 + 100, h: 0, flags: 0};
	contextMenu->pos_menu = post_menu;

	// To highlight the entry the context menu applies to
	irect_s pos_selected_item = {x: item.x, y: item.y, w: screenWidth, h: item.getHeight(), flags: 0};
	contextMenu->pos_selected_item = pos_selected_item;

	OpenContextMenu(contextMenu);
	DEBUG("Opening context menu: done");
}


void Gui::keypressEvent(int key)
{

}


void Gui::statusBarText(const char *format, va_list args)
{
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), format, args);

	FillArea(0, screenHeight - 33, screenWidth, 33, WHITE);
	DrawLine(0, screenHeight - 34, screenWidth, screenHeight - 34, BLACK);

	SetFont(statusBarFont, DGRAY);
	DrawString(0, screenHeight - 34, buffer);

	PartialUpdate(0, screenHeight - 35, screenWidth, 35);
}

void Gui::statusBarText(const char *format...)
{
	va_list args;

	va_start(args, format);
	statusBarText(format, args);
	va_end(args);
}



void Gui::openProgressBar(int icon, const char *title, const char *text, int percent, iv_dialoghandler hproc)
{
	OpenProgressbar(icon, title, text, percent, hproc);
}


int Gui::updateProgressBar(const char *text, int percent)
{
	return UpdateProgressbar(text, percent);
}


void Gui::closeProgressBar()
{
	CloseProgressbar();
}

