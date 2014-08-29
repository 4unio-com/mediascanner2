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


#ifndef QT_CLIENT_H
#define QT_CLIENT_H

#include<QObject>
#include<QDBusInterface>
#include<mediascanner/MediaStoreBase.hh>
#include<mediascanner/MediaFile.hh>
#include<mediascanner/Filter.hh>
#include<vector>

class QtClient : public QObject, public virtual mediascanner::MediaStoreBase {
public:

    explicit QtClient(QObject *parent);
    virtual ~QtClient();

    QtClient(const QtClient &other) = delete;
    QtClient& operator=(const QtClient &other) = delete;

    virtual mediascanner::MediaFile lookup(const std::string &filename) const override;
    virtual std::vector<mediascanner::MediaFile> query(const std::string &q, mediascanner::MediaType type, const mediascanner::Filter& filter) const override;
    virtual std::vector<mediascanner::Album> queryAlbums(const std::string &core_term, const mediascanner::Filter &filter) const override;
    virtual std::vector<std::string> queryArtists(const std::string &q, const mediascanner::Filter &filter) const override;
    virtual std::vector<mediascanner::MediaFile> getAlbumSongs(const mediascanner::Album& album) const override;
    virtual std::string getETag(const std::string &filename) const override;
    virtual std::vector<mediascanner::MediaFile> listSongs(const mediascanner::Filter &filter) const override;
    virtual std::vector<mediascanner::Album> listAlbums(const mediascanner::Filter &filter) const override;
    virtual std::vector<std::string> listArtists(const mediascanner::Filter &filter) const override;
    virtual std::vector<std::string>listAlbumArtists(const mediascanner::Filter &filter) const override;
    virtual std::vector<std::string>listGenres(const mediascanner::Filter &filter) const override;

private:
    mutable QDBusInterface service;
};

#endif
