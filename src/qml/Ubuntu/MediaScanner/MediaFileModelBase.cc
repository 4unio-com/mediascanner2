/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    James Henstridge <james.henstridge@canonical.com>
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

#include "MediaFileModelBase.hh"
#include "MediaFileWrapper.hh"

using namespace mediascanner::qml;

MediaFileModelBase::MediaFileModelBase(QObject *parent)
    : QAbstractListModel(parent) {
    roles[Roles::RoleModelData] = "modelData";
    roles[Roles::RoleFilename] = "filename";
    roles[Roles::RoleUri] = "uri";
    roles[Roles::RoleContentType] = "contentType";
    roles[Roles::RoleETag] = "eTag";
    roles[Roles::RoleTitle] = "title";
    roles[Roles::RoleAuthor] = "author";
    roles[Roles::RoleAlbum] = "album";
    roles[Roles::RoleAlbumArtist] = "albumArtist";
    roles[Roles::RoleDate] = "date";
    roles[Roles::RoleTrackNumber] = "trackNumber";
    roles[Roles::RoleDuration] = "duration";
}

int MediaFileModelBase::rowCount(const QModelIndex &) const {
    return results.size();
}

QVariant MediaFileModelBase::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= (ptrdiff_t)results.size()) {
        return QVariant();
    }
    const mediascanner::MediaFile &media = results[index.row()];
    switch (role) {
    case RoleModelData:
        return QVariant::fromValue(new MediaFileWrapper(media));
    case RoleFilename:
        return QString::fromStdString(media.getFileName());
    case RoleUri:
        return QString::fromStdString(media.getUri());
    case RoleContentType:
        return QString::fromStdString(media.getContentType());
    case RoleETag:
        return QString::fromStdString(media.getETag());
    case RoleTitle:
        return QString::fromStdString(media.getTitle());
    case RoleAuthor:
        return QString::fromStdString(media.getAuthor());
    case RoleAlbum:
        return QString::fromStdString(media.getAlbum());
    case RoleAlbumArtist:
        return QString::fromStdString(media.getAlbumArtist());
    case RoleDate:
        return QString::fromStdString(media.getDate());
    case RoleTrackNumber:
        return media.getTrackNumber();
    case RoleDuration:
        return media.getDuration();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MediaFileModelBase::roleNames() const {
    return roles;
}

void MediaFileModelBase::updateResults(const std::vector<mediascanner::MediaFile> &results) {
    beginResetModel();
    this->results = results;
    endResetModel();
}