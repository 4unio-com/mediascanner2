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

#ifndef QT_SERVICE_H
#define QT_SERVICE_H

#include <QDBusAbstractAdaptor>
#include <QDBusArgument>
#include <mediascanner/MediaStoreBase.hh>
#include <mediascanner/MediaFile.hh>
#include <mediascanner/MediaStore.hh>

#define MS_DBUS_INTERFACE "com.canonical.MediaScanner2"
#define MS_DBUS_OBJECT "/com/canonical/MediaScanner2"
#define MS_DBUS_NAME "com.canonical.MediaScanner2"

// Trying to register MediaFile directly fails. Seems like a bug in Qt.
// Hence this workaround.
struct MediaFileWire {
    QString fname;
    QString contenttype;
    QString etag;
    QString title;
    QString author;
    QString album;
    QString albumartist;
    QString date;
    QString genre;
    int32_t discnumber;
    int32_t tracknumber;
    int32_t duration;
    int32_t width;
    int32_t height;
    double latitude;
    double longitude;
    int32_t type;

    MediaFileWire() {};
    explicit MediaFileWire(const mediascanner::MediaFile &mf);
    mediascanner::MediaFile toMediaFile() const;
};

struct AlbumWire {
    QString title;
    QString artist;

    AlbumWire(){};
    explicit AlbumWire(const mediascanner::Album &a);
    mediascanner::Album toAlbum() const;
};

Q_DECLARE_METATYPE(MediaFileWire)
Q_DECLARE_METATYPE(QVariantMap)
Q_DECLARE_METATYPE(QList<MediaFileWire>)
Q_DECLARE_METATYPE(AlbumWire)
Q_DECLARE_METATYPE(QList<AlbumWire>)

QDBusArgument &operator<<(QDBusArgument &argument, const MediaFileWire &tt);
const QDBusArgument &operator>>(const QDBusArgument &argument, MediaFileWire &tt);

QDBusArgument &operator<<(QDBusArgument &argument, const AlbumWire &a);
const QDBusArgument &operator>>(const QDBusArgument &argument, AlbumWire &a);

namespace mediascanner {

class QtService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", MS_DBUS_INTERFACE)

public:
    explicit QtService(QObject *parent=nullptr);
    ~QtService();

public Q_SLOTS:

    MediaFileWire lookup(const QString &filename) const;
    QList<MediaFileWire> query(const QString &q, int type, const QVariantMap &filter) const;
    QList<AlbumWire> queryAlbums(const QString &core_term, const QVariantMap &filter) const;
    /*
    std::vector<std::string> queryArtists(const std::string &q, const Filter &filter) const override;
    std::vector<MediaFile> getAlbumSongs(const Album& album) const override;
    std::string getETag(const std::string &filename) const override;
    std::vector<MediaFile> listSongs(const Filter &filter) const override;
    std::vector<Album> listAlbums(const Filter &filter) const override;
    std::vector<std::string> listArtists(const Filter &filter) const override;
    std::vector<std::string> listAlbumArtists(const Filter &filter) const override;
    std::vector<std::string> listGenres(const Filter &filter) const override;
*/
private:
    MediaStore store;
};

}

#endif
