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

#include "../mediascanner/MediaStore.hh"
#include "../mediascanner/MediaFile.hh"
#include "InvalidationSender.hh"
#include "MetadataExtractor.hh"
#include "SubtreeWatcher.hh"

#include<sys/select.h>
#include<stdexcept>
#include<sys/inotify.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<cstring>
#include<cerrno>
#include<string>
#include<map>
#include<memory>

#include <glib.h>
#include <glib-unix.h>

using namespace std;

namespace mediascanner {

struct SubtreeWatcherPrivate {
    MediaStore &store; // Hackhackhack, should be replaced with callback object or something.
    MetadataExtractor &extractor;
    InvalidationSender &invalidator;
    int inotifyid;
    // Ideally use boost::bimap or something instead of these two separate objects.
    std::map<int, std::string> wd2str;
    std::map<std::string, int> str2wd;
    bool keep_going;

    std::unique_ptr<GSource,void(*)(GSource*)> source;

    SubtreeWatcherPrivate(MediaStore &store, MetadataExtractor &extractor, InvalidationSender &invalidator) :
        store(store), extractor(extractor), invalidator(invalidator),
        inotifyid(inotify_init()), keep_going(true),
        source(g_unix_fd_source_new(inotifyid, G_IO_IN), g_source_unref) {
    }

    ~SubtreeWatcherPrivate() {
        for(auto &i : wd2str) {
            inotify_rm_watch(inotifyid, i.first);
        }
        close(inotifyid);
    }

};

static gboolean source_callback(GIOChannel *, GIOCondition, gpointer data) {
    SubtreeWatcher *watcher = static_cast<SubtreeWatcher*>(data);
    watcher->processEvents();
    return TRUE;
}

SubtreeWatcher::SubtreeWatcher(MediaStore &store, MetadataExtractor &extractor, InvalidationSender &invalidator) {
    p = new SubtreeWatcherPrivate(store, extractor, invalidator);
    if(p->inotifyid == -1) {
        string msg("Could not init inotify: ");
        msg += strerror(errno);
        delete p;
        throw runtime_error(msg);
    }
    g_source_set_callback(p->source.get(), reinterpret_cast<GSourceFunc>(source_callback), static_cast<gpointer>(this), nullptr);
    g_source_attach(p->source.get(), nullptr);
}

SubtreeWatcher::~SubtreeWatcher() {
    g_source_destroy(p->source.get());
    delete p;
}

void SubtreeWatcher::addDir(const string &root) {
    if(root[0] != '/')
        throw runtime_error("Path must be absolute.");
    if(p->str2wd.find(root) != p->str2wd.end())
        return;
    unique_ptr<DIR, int(*)(DIR*)> dir(opendir(root.c_str()), closedir);
    if(!dir) {
        return;
    }
    int wd = inotify_add_watch(p->inotifyid, root.c_str(),
            IN_CREATE | IN_DELETE_SELF | IN_DELETE | IN_CLOSE_WRITE |
            IN_MOVED_FROM | IN_MOVED_TO | IN_ONLYDIR);
    if(wd == -1) {
        string msg("Could not create inotify watch object: ");
        msg += strerror(errno);
        throw runtime_error(msg);
    }
    p->wd2str[wd] = root;
    p->str2wd[root] = wd;
    printf("Watching subdirectory %s, %ld watches in total.\n", root.c_str(),
            (long)p->wd2str.size());
    unique_ptr<struct dirent, void(*)(void*)> entry((dirent*)malloc(sizeof(dirent) + NAME_MAX),
                    free);
    struct dirent *de;
        while(readdir_r(dir.get(), entry.get(), &de) == 0 && de ) {
        struct stat statbuf;
        string fname = entry.get()->d_name;
        if(fname[0] == '.') // Ignore hidden entries and also "." and "..".
            continue;
        string fullpath = root + "/" + fname;
        lstat(fullpath.c_str(), &statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            addDir(fullpath);
        }
    }
}

bool SubtreeWatcher::removeDir(const string &abspath) {
    if(p->str2wd.find(abspath) == p->str2wd.end())
        return false;
    int wd = p->str2wd[abspath];
    inotify_rm_watch(p->inotifyid, wd);
    p->wd2str.erase(wd);
    p->str2wd.erase(abspath);
    printf("Stopped watching %s, %ld directories remain.\n", abspath.c_str(),
            (long)p->wd2str.size());
    if(p->wd2str.empty())
        p->keep_going = false;
    return true;
}

void SubtreeWatcher::fileAdded(const string &abspath) {
    printf("New file was created: %s.\n", abspath.c_str());
    try {
        DetectedFile d = p->extractor.detect(abspath);
        // Only extract and insert the file if the ETag has changed.
        if (d.etag != p->store.getETag(d.filename)) {
            p->store.insert(p->extractor.extract(d));
        }
    } catch(const exception &e) {
        fprintf(stderr, "Error when adding new file: %s\n", e.what());
    }
}

void SubtreeWatcher::fileDeleted(const string &abspath) {
    printf("File was deleted: %s\n", abspath.c_str());
    p->store.remove(abspath);
}

void SubtreeWatcher::dirAdded(const string &abspath) {
    printf("New directory was created: %s.\n", abspath.c_str());
    addDir(abspath);
}

void SubtreeWatcher::dirRemoved(const string &abspath) {
    printf("Subdirectory was deleted: %s.\n", abspath.c_str());
    removeDir(abspath);
}


bool SubtreeWatcher::pumpEvents() {
    GMainContext *context = g_main_context_default();
    while (g_main_context_iteration(context, false)) {
    }
    return false;
}

void SubtreeWatcher::processEvents() {
    const int BUFSIZE=4096;
    char buf[BUFSIZE];
    bool changed = false;
    ssize_t num_read;
    num_read = read(p->inotifyid, buf, BUFSIZE);
    if(num_read == 0) {
        printf("Inotify returned 0.\n");
        return;
    }
    if(num_read == -1) {
        printf("Read error.\n");
        return;
    }
    for(char *d = buf; d < buf + num_read;) {
        struct inotify_event *event = (struct inotify_event *) d;
        if (p->wd2str.find(event->wd) == p->wd2str.end()) {
            // Ignore events for unknown watches.  We may receive
            // such events when a directory is removed.
            d += sizeof(struct inotify_event) + event->len;
            continue;
        }
        string directory = p->wd2str[event->wd];
        string filename(event->name);
        string abspath = directory + '/' + filename;
        bool is_dir = false;
        bool is_file = false;
        struct stat statbuf;
        lstat(abspath.c_str(), &statbuf);
        // Remember: these are not valid in case of delete event.
        if(S_ISDIR(statbuf.st_mode))
            is_dir = true;
        if(S_ISREG(statbuf.st_mode))
            is_file = true;

        if(event->mask & IN_CREATE) {
            if(is_dir) {
                dirAdded(abspath);
                changed = true;
            }
            // Do not add files upon creation because we can't parse
            // their metadata until it is fully written.
        } else if((event->mask & IN_CLOSE_WRITE) || (event->mask & IN_MOVED_TO)) {
            if(is_dir) {
                dirAdded(abspath);
                changed = true;
            }
            if(is_file) {
                fileAdded(abspath);
                changed = true;
            }
        } else if((event->mask & IN_DELETE) || (event->mask & IN_MOVED_FROM)) {
            if(p->str2wd.find(abspath) != p->str2wd.end()) {
                dirRemoved(abspath);
                changed = true;
            } else {
                fileDeleted(abspath);
                changed = true;
            }
        } else if((event->mask & IN_IGNORED) || (event->mask & IN_UNMOUNT) || (event->mask & IN_DELETE_SELF)) {
            removeDir(abspath);
            changed = true;
        }
        d += sizeof(struct inotify_event) + event->len;
    }
    if (changed) {
        p->invalidator.invalidate();
    }
}

int SubtreeWatcher::getFd() const {
    return p->inotifyid;
}

int SubtreeWatcher::directoryCount() const {
    return (int) p->wd2str.size();
}

}
