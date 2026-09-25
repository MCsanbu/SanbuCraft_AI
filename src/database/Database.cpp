#include "database/Database.h"
#include <sqlite3.h>
namespace sanbucraft::database { Database::~Database() { if (connection_) sqlite3_close(connection_); }
bool Database::open(const std::string& path, std::string& error) { if (sqlite3_open_v2(path.c_str(), &connection_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) { error = sqlite3_errmsg(connection_); return false; } return migrate(error); }
bool Database::execute(const char* sql, std::string& error) { char* message = nullptr; if (sqlite3_exec(connection_, sql, nullptr, nullptr, &message) != SQLITE_OK) { error = message ? message : "SQLite error"; sqlite3_free(message); return false; } return true; }
bool Database::migrate(std::string& error) { return execute("PRAGMA foreign_keys=ON; CREATE TABLE IF NOT EXISTS schema_migrations(version INTEGER PRIMARY KEY); INSERT OR IGNORE INTO schema_migrations VALUES(1); CREATE TABLE IF NOT EXISTS worlds(id INTEGER PRIMARY KEY,name TEXT NOT NULL,path TEXT UNIQUE NOT NULL,seed INTEGER,game_time INTEGER,data_version INTEGER); CREATE TABLE IF NOT EXISTS players(id INTEGER PRIMARY KEY,world_id INTEGER NOT NULL REFERENCES worlds(id) ON DELETE CASCADE,uuid TEXT,name TEXT,dimension TEXT,x REAL,y REAL,z REAL,health REAL,food_level INTEGER,xp_level INTEGER); CREATE TABLE IF NOT EXISTS items(id INTEGER PRIMARY KEY,item_id TEXT UNIQUE NOT NULL); CREATE TABLE IF NOT EXISTS containers(id INTEGER PRIMARY KEY,world_id INTEGER NOT NULL REFERENCES worlds(id) ON DELETE CASCADE,dimension TEXT,x INTEGER,y INTEGER,z INTEGER,type TEXT); CREATE TABLE IF NOT EXISTS container_items(container_id INTEGER REFERENCES containers(id) ON DELETE CASCADE,item_id TEXT,count INTEGER,slot INTEGER); CREATE TABLE IF NOT EXISTS chunks(id INTEGER PRIMARY KEY,world_id INTEGER REFERENCES worlds(id) ON DELETE CASCADE,dimension TEXT,x INTEGER,z INTEGER,updated_at INTEGER); CREATE TABLE IF NOT EXISTS structures(id INTEGER PRIMARY KEY,world_id INTEGER REFERENCES worlds(id) ON DELETE CASCADE,type TEXT,dimension TEXT,x INTEGER,y INTEGER,z INTEGER); CREATE TABLE IF NOT EXISTS analysis_results(id INTEGER PRIMARY KEY,world_id INTEGER REFERENCES worlds(id) ON DELETE CASCADE,key TEXT,value TEXT,updated_at INTEGER); CREATE INDEX IF NOT EXISTS idx_containers_world_position ON containers(world_id,dimension,x,z); CREATE INDEX IF NOT EXISTS idx_container_items_item ON container_items(item_id);", error); }
bool Database::replaceWorldData(const sanbucraft::minecraft::WorldInfo& world,
                                const std::vector<sanbucraft::minecraft::Player>& players,
                                const std::vector<sanbucraft::minecraft::Container>& containers,
                                std::string& error) {
    // The schema is intentionally ahead of the current incremental persistence implementation.
    // Retain the arguments so callers do not need an API-breaking change when player/container
    // repositories are introduced.
    (void)players;
    (void)containers;
    if (!execute("BEGIN IMMEDIATE;", error)) return false;
    sqlite3_stmt* statement = nullptr;
    const char* sql = "INSERT INTO worlds(name,path,seed,game_time,data_version) VALUES(?,?,?,?,?) "
                      "ON CONFLICT(path) DO UPDATE SET name=excluded.name,seed=excluded.seed,"
                      "game_time=excluded.game_time,data_version=excluded.data_version;";
    if (sqlite3_prepare_v2(connection_, sql, -1, &statement, nullptr) != SQLITE_OK) {
        execute("ROLLBACK;", error);
        error = sqlite3_errmsg(connection_);
        return false;
    }
    sqlite3_bind_text(statement, 1, world.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, world.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement, 3, world.seed);
    sqlite3_bind_int64(statement, 4, world.gameTime);
    sqlite3_bind_int(statement, 5, world.dataVersion);
    const bool success = sqlite3_step(statement) == SQLITE_DONE;
    sqlite3_finalize(statement);
    if (!success) {
        execute("ROLLBACK;", error);
        error = sqlite3_errmsg(connection_);
        return false;
    }
    return execute("COMMIT;", error);
}
}
