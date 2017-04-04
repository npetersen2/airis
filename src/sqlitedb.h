#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <iostream>
#include <string>
#include <sqlite3.h>

class SQLiteDB {
public:
	SQLiteDB(const std::string &dbFile) {
		this->dbFile = dbFile;
		open_db();
	}

	~SQLiteDB() {
		close_db();
	}

	void close() {
		close_db();
	}

	void beginTransaction() {
		sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	}

	void endTransaction() {
		sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
	}

	void vacuum() {
		std::cout << "Compressing database file: " << dbFile << "..." << std::endl;
		sqlite3_exec(db, "VACUUM;", NULL, NULL, NULL);
	}

	void rollback() {
		std::cout << "Rolling back database file: " << dbFile << "..." << std::endl;
		sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
	}
private:
	std::string dbFile;

	void open_db() {
		if (sqlite3_open(dbFile.c_str(), &(this->db))) {
			std::cout
				<< "Error opening stock history database... "
				<< sqlite3_errmsg(this->db)
				<< std::endl;
			exit(0);
		}

		exec_query("PRAGMA journal_mode=WAL", nullptr, nullptr);
	}

	void close_db() {
		sqlite3_close(this->db);
	}

protected:
	sqlite3 *db;

	void exec_query(const std::string sql, int (*callback)(void*, int, char**, char**), void *ptr) {
		if (sqlite3_exec(this->db, sql.c_str(), callback, ptr, nullptr) != SQLITE_OK) {
			std::cout 
				<< "Error executing SQL command: " 
				<< sqlite3_errmsg(this->db) << std::endl;

			std::cout << "SQL statement:" << std::endl;
			std::cout << sql << std::endl;
			close_db();
			exit(0);
		}
	}

};

#endif // SQLITEDB_H
