#ifndef DATABASE_DATABASE_H_
#define DATABASE_DATABASE_H_

#include "inkview.h"

#include <string>
#include "sqlite3.h"

#include "../defines.h"
#include "../log.h"

extern void log_message(const char *msg);


class Internal
{
public:
	bool isNull;
	std::string key;
	std::string value;
};


class Database
{
public:

	~Database();

	void drop(void);
	void open(void);

	void runMigrations(void);

	void saveInternal(std::string key, std::string value);
	Internal selectInternal(std::string key);

	sqlite3 *getDb() { return this->db; }

private:
	const char *DB_FILE = PLOP_BASE_DIRECTORY "/database.sqlite3";
	sqlite3 *db;

	void migration_001_createInternalsTable();
	void migration_002_createEntriesTable();
	void migration_003_createIndexesOnEntries();
	void migration_004_createEpubDownloadQueueTable();
	void migration_005_addIsEmptyFieldOnEntries();
	void migration_006_createIndexSortForDisplayingListOnEntries();

	void insertInternal(std::string key, std::string value);
	void updateInternal(std::string key, std::string value);

};


#endif /* DATABASE_DATABASE_H_ */
