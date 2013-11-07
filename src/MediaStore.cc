/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../mozilla/fts3_tokenizer.h"
#include"MediaStore.hh"
#include"MediaFile.hh"
#include "sqliteutils.hh"
#include"utils.hh"
#include <sqlite3.h>
#include <cstdio>
#include <stdexcept>
#include<cassert>

using namespace std;

// Increment this whenever changing db schema.
// It will cause dbstore to rebuild its tables.
static const int schemaVersion = 0;

struct MediaStorePrivate {
    sqlite3 *db;
};

extern "C" void sqlite3Fts3PorterTokenizerModule(
    sqlite3_tokenizer_module const**ppModule);

int register_tokenizer(sqlite3 *db) {
    int rc;
    const sqlite3_tokenizer_module *p = NULL;
    sqlite3_stmt *pStmt;
    const char *zSql = "SELECT fts3_tokenizer(?, ?)";

    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc!=SQLITE_OK ){
        return rc;
    }

    sqlite3_bind_text(pStmt, 1, "mozporter", -1, SQLITE_STATIC);
    sqlite3Fts3PorterTokenizerModule(&p);
    sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
    sqlite3_step(pStmt);

    return sqlite3_finalize(pStmt);
}

static void execute_sql(sqlite3 *db, const string &cmd) {
    char *errmsg = nullptr;
    if(sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        throw runtime_error(errmsg);
    }
}

static int getSchemaVersion(sqlite3 *db) {
    int version = -1;
    try {
        Statement select(db, "SELECT version FROM schemaVersion");
        if (select.step())
            version = select.getInt(0);
    } catch (const exception &e) {
        /* schemaVersion table might not exist */
    }
    return version;
}

void deleteTables(sqlite3 *db) {
    string deleteCmd(R"(DROP TABLE IF EXISTS media;
DROP TABLE IF EXISTS media_attic;
DROP TABLE IF EXISTS schemaVersion;
)");
    execute_sql(db, deleteCmd);
}

void createTables(sqlite3 *db) {
    string mediaCreate(R"(
CREATE TABLE schemaVersion (version INTEGER);

CREATE TABLE media (
    filename TEXT PRIMARY KEY NOT NULL,
    title TEXT,
    artist TEXT,    -- Only relevant to audio
    album TEXT,     -- Only relevant to audio
    duration INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE TABLE media_attic (
    filename TEXT PRIMARY KEY NOT NULL,
    title TEXT,
    artist TEXT,    -- Only relevant to audio
    album TEXT,     -- Only relevant to audio
    duration INTEGER,
    type INTEGER   -- 0=Audio, 1=Video
);

CREATE VIRTUAL TABLE media_fts 
USING fts4(content="media", title, artist, album, tokenize=mozporter);
)");

    string triggerCreate(R"(CREATE TRIGGER media_bu BEFORE UPDATE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.rowid;
END;

CREATE TRIGGER media_au AFTER UPDATE ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.rowid, new.title, new.artist, new.album);
END;

CREATE TRIGGER media_bd BEFORE DELETE ON media BEGIN
  DELETE FROM media_fts WHERE docid=old.rowid;
END;

CREATE TRIGGER media_ai AFTER INSERT ON media BEGIN
  INSERT INTO media_fts(docid, title, artist, album) VALUES (new.rowid, new.title, new.artist, new.album);
END;
)");
    string versionStore("INSERT INTO schemaVersion VALUES (");
    versionStore += to_string(schemaVersion);
    versionStore += ");";

    execute_sql(db, mediaCreate);
    execute_sql(db, triggerCreate);
    execute_sql(db, versionStore);
}

MediaStore::MediaStore(const std::string &filename, OpenType access, const std::string &retireprefix) {
    int sqliteFlags = access == MS_READ_WRITE ? SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE : SQLITE_OPEN_READONLY;
    p = new MediaStorePrivate();
    if(sqlite3_open_v2(filename.c_str(), &p->db, sqliteFlags, nullptr) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(p->db));
    }
    if (register_tokenizer(p->db) != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(p->db));
    }
    int detectedSchemaVersion = getSchemaVersion(p->db);
    if(access == MS_READ_WRITE) {
        if(detectedSchemaVersion != schemaVersion) {
            deleteTables(p->db);
            createTables(p->db);
        }
        if(!retireprefix.empty())
            archiveItems(retireprefix);
    } else {
        if(detectedSchemaVersion != schemaVersion) {
            throw runtime_error("Tried to open a db with an unsupported schema version.");
        }
    }
}

MediaStore::~MediaStore() {
    sqlite3_close(p->db);
    delete p;
}

size_t MediaStore::size() const {
    Statement count(p->db, "SELECT COUNT(*) FROM media");
    count.step();
    return count.getInt(0);
}

static int yup(void* arg, int /*num_cols*/, char **/*data*/, char ** /*colnames*/) {
    bool *t = reinterpret_cast<bool *> (arg);
    *t = true;
    return 0;
}

void MediaStore::insert(const MediaFile &m) {
    char *errmsg;

    const char *insert_templ = "INSERT INTO media VALUES(%s, %s, %s, %s, %d, %d);";
    const char *query_templ = "SELECT * FROM media WHERE filename=%s;";
    const size_t bufsize = 1024;
    char qcmd[bufsize];
    char icmd[bufsize];

    string fname = sqlQuote(m.getFileName());
    string title;
    if(m.getTitle().empty())
        title = sqlQuote(filenameToTitle(m.getFileName()));
    else
        title = sqlQuote(m.getTitle());
    string author = sqlQuote(m.getAuthor());
    string album = sqlQuote(m.getAlbum());
    int duration = m.getDuration();
    int type = (int)m.getType();
    snprintf(qcmd, bufsize, query_templ, fname.c_str());
    snprintf(icmd, bufsize, insert_templ, fname.c_str(), title.c_str(),
            author.c_str(), album.c_str(), duration, type);

    bool was_in = false;
    if(sqlite3_exec(p->db, qcmd, yup, &was_in, &errmsg ) != SQLITE_OK) {
        throw runtime_error(errmsg);
    }
    if(was_in) {
        return;
    }
    if(sqlite3_exec(p->db, icmd, NULL, NULL, &errmsg) != SQLITE_OK) {
        throw runtime_error(errmsg);
    }
    const char *typestr = m.getType() == AudioMedia ? "song" : "video";
    printf("Added %s to backing store: %s\n", typestr, m.getFileName().c_str());
    printf(" author   : '%s'\n", m.getAuthor().c_str());
    printf(" title    : %s\n", title.c_str());
    printf(" album    : '%s'\n", m.getAlbum().c_str());
    printf(" duration : %d\n", m.getDuration());
}

void MediaStore::remove(const string &fname) {
    Statement del(p->db, "DELETE FROM media WHERE filename = ?");
    del.bind(1, fname);
    del.step();
}

vector<MediaFile> MediaStore::query(const std::string &core_term, MediaType type) {
    Statement query(p->db, R"(SELECT * FROM media
WHERE rowid IN (SELECT docid FROM media_fts WHERE title MATCH ?)
AND type == ?)");
    vector<MediaFile> result;

    query.bind(1, core_term + "*");
    query.bind(2, (int)type);
    while (query.step()) {
        string filename = query.getText(0);
        string title = query.getText(1);
        string author = query.getText(2);
        string album = query.getText(3);
        int duration = query.getInt(4);
        MediaType type = (MediaType)query.getInt(5);
        result.push_back(MediaFile(filename, title, author, album, duration, type));
    }
    return result;
}

void MediaStore::pruneDeleted() {
    vector<string> deleted;
    Statement query(p->db, "SELECT filename FROM media");
    while (query.step()) {
        const string filename = query.getText(0);
        FILE *f = fopen(filename.c_str(), "r");
        if(f) {
            fclose(f);
        } else {
            deleted.push_back(filename);
        }
    }
    query.finalize();
    printf("%d files deleted from disk.\n", (int)deleted.size());
    for(const auto &i : deleted) {
        remove(i);
    }
}

void MediaStore::archiveItems(const std::string &prefix) {
    const char *templ = R"(BEGIN TRANSACTION;
INSERT INTO media_attic SELECT * FROM media WHERE filename LIKE %s;
DELETE FROM media WHERE filename LIKE %s;
COMMIT;
)";
    string cond = sqlQuote(prefix + "%");
    const size_t bufsize = 1024;
    char cmd[bufsize];
    snprintf(cmd, bufsize, templ, cond.c_str(), cond.c_str());
    char *errmsg;
    if(sqlite3_exec(p->db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(p->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }
}

void MediaStore::restoreItems(const std::string &prefix) {
    const char *templ = R"(BEGIN TRANSACTION;
INSERT INTO media SELECT * FROM media_attic WHERE filename LIKE %s;
DELETE FROM media_attic WHERE filename LIKE %s;
COMMIT;
)";
    string cond = sqlQuote(prefix + "%");
    const size_t bufsize = 1024;
    char cmd[bufsize];
    snprintf(cmd, bufsize, templ, cond.c_str(), cond.c_str());
    char *errmsg;
    if(sqlite3_exec(p->db, cmd, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        sqlite3_exec(p->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw runtime_error(errmsg);
    }

}
