#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <tgmath.h>
#include <string>

#include "api/wallabag_api.h"
#include "api/wallabag_config_loader.h"
#include "api/wallabag_config.h"

#include "database/database.h"

#include "gui/gui.h"

#include "repositories/entry_repository.h"


#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()


class Application
{
public:
	enum entries_mode {MODE_UNREAD=1, MODE_ARCHIVED, MODE_STARRED};

	Application() : entryRepository(db), gui(*this) {
		db.open();
		pageNum = 1;
		numPerPage = 8;
		isLastActionRead = false;
		lastReadEntryId = 0;
		mode = MODE_UNREAD;
	}

	void init();
	void deinit();

	void loadRecentArticles();

	Database &getDb() {
		return db;
	}


	void show();

	void touchStartEvent(int x, int y);
	void touchEndEvent(int x, int y);

	void keypressEvent(int key);

	void read(Entry &entry);

	void foreground();
	void background();

	void handleActionOnReadEntry(int entryId);

	void setMode(int m);

	Gui &getGui() {
		return gui;
	}

private:
	Database db;
	WallabagApi wallabag_api;
	EntryRepository entryRepository;
	Gui gui;

	entries_mode mode;

	int pageNum;
	int numPerPage;

	bool isLastActionRead;
	int lastReadEntryId;

	bool connectToNetwork();

	int countEntriesForCurrentMode();
	void listEntriesForCurrentMode(std::vector<Entry> &entries);

};


#endif /* APPLICATION_H_ */
