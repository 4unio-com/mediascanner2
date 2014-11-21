/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include"qtclient.h"
#include"qtservice.h"
#include"comcanonicalmediascanner2.h"
#include <QDBusMetaType>
#include<mediascanner/Album.hh>
#include<QDBusReply>
#include<QDebug>

using namespace mediascanner;

QtClient::QtClient(QObject *parent) : QObject(parent) {
    const char *busname = getenv("MEDIASCANNER_DBUS_NAME");
    if(!busname) {
        busname = MS_DBUS_NAME;
    }
    service = new ComCanonicalMediascanner2(busname, MS_DBUS_OBJECT, QDBusConnection::sessionBus(), this);
    qDBusRegisterMetaType<MediaFileWire>();
    qDBusRegisterMetaType<AlbumWire>();
    qDBusRegisterMetaType<QList<MediaFileWire>>();
    qDBusRegisterMetaType<QList<AlbumWire>>();
    qDBusRegisterMetaType<QVariantMap>();

}

QtClient:: ~QtClient() {
}

mediascanner::MediaFile QtClient::lookup(const std::string &filename) const {
    QDBusPendingReply<MediaFileWire> reply = service->lookup(filename.c_str());
    reply.waitForFinished();
    if(!reply.isValid() || reply.isError()) {
        throw std::runtime_error(reply.error().message().toUtf8().data());
    }
    return reply.value().toMediaFile();
}

std::vector<mediascanner::MediaFile> QtClient::query(const std::string &q, mediascanner::MediaType type, const mediascanner::Filter& filter) const {
    QDBusReply<QList<MediaFileWire>> reply = service->query(q.c_str(), (int32_t)(type), filter2vmap(filter));
    std::vector<mediascanner::MediaFile> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &mf : reply.value()) {
            result.push_back(mf.toMediaFile());
        }
    }
    return result;
}

std::vector<mediascanner::Album> QtClient::queryAlbums(const std::string &core_term, const mediascanner::Filter &filter) const {
    QDBusReply<QList<AlbumWire>> reply = service->queryAlbums(core_term.c_str(), filter2vmap(filter));
    std::vector<mediascanner::Album> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toAlbum());
        }
    }
    return result;
}

std::vector<std::string> QtClient::queryArtists(const std::string &q, const mediascanner::Filter &filter) const {
    QDBusReply<QStringList> reply = service->queryArtists(q.c_str(), filter2vmap(filter));
    std::vector<std::string> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toStdString());
        }
    }
    return result;
}

std::vector<mediascanner::MediaFile> QtClient::getAlbumSongs(const mediascanner::Album& album) const {
    QDBusReply<QList<MediaFileWire>> reply = service->getAlbumSongs(AlbumWire(album));
    std::vector<mediascanner::MediaFile> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &s : reply.value()) {
            result.push_back(s.toMediaFile());
        }
    }
    return result;
}

std::string QtClient::getETag(const std::string &filename) const {
    QDBusReply<QString> reply = service->getETag(filename.c_str());
    std::string result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        result = reply.value().toStdString();
    }
    return result;
}

std::vector<mediascanner::MediaFile> QtClient::listSongs(const mediascanner::Filter &filter) const {
    QDBusReply<QList<MediaFileWire>> reply = service->listSongs(filter2vmap(filter));
    std::vector<mediascanner::MediaFile> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &s : reply.value()) {
            result.push_back(s.toMediaFile());
        }
    }
    return result;
}

std::vector<mediascanner::Album> QtClient::listAlbums(const mediascanner::Filter &filter) const {
    QDBusReply<QList<AlbumWire>> reply = service->listAlbums(filter2vmap(filter));
    std::vector<mediascanner::Album> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toAlbum());
        }
    }
    return result;

}

std::vector<std::string> QtClient::listArtists(const mediascanner::Filter &filter) const {
    QDBusReply<QStringList> reply = service->listArtists(filter2vmap(filter));
    std::vector<std::string> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toStdString());
        }
    }
    return result;

}

std::vector<std::string> QtClient::listAlbumArtists(const mediascanner::Filter &filter) const {
    QDBusReply<QStringList> reply = service->listAlbumArtists(filter2vmap(filter));
    std::vector<std::string> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toStdString());
        }
    }
    return result;

}

std::vector<std::string> QtClient::listGenres(const mediascanner::Filter &filter) const {
    QDBusReply<QStringList> reply = service->listGenres(filter2vmap(filter));
    std::vector<std::string> result;
    if(!reply.isValid()) {
        qWarning() << "DBus call failed: " << reply.error().message() << "\n";
    } else {
        for(const auto &a : reply.value()) {
            result.push_back(a.toStdString());
        }
    }
    return result;
}
