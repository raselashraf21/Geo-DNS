
#ifndef GEODNS_SQLITE_DB_BACKUP_H
#define GEODNS_SQLITE_DB_BACKUP_H

#include <sqlite3.h>

int loadOrSaveDb(sqlite3 *pInMemory, const char *zFilename, int isSave);

#endif // GEODNS_SQLITE_DB_BACKUP_H