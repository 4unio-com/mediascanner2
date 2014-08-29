/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "MediaStoreWrapper.hh"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <QDBusConnection>
#include <QDebug>
#include <QQmlEngine>

#include <mediascanner/Filter.hh>
#include <mediascanner/MediaStore.hh>
#include <ms-dbus/qtclient.h>

using namespace mediascanner::qml;

MediaStoreWrapper::MediaStoreWrapper(QObject *parent)
    : QObject(parent) {
    const char *use_dbus = getenv("MEDIASCANNER_USE_DBUS");
    if (use_dbus != nullptr && !strcmp(use_dbus, "1")) {
        store.reset(new QtClient(this));
    } else {
        store.reset(new mediascanner::MediaStore(MS_READ_ONLY));
    }

    QDBusConnection::sessionBus().connect(
        "com.canonical.MediaScanner2.Daemon",
        "/com/canonical/unity/scopes",
        "com.canonical.unity.scopes", "InvalidateResults",
        QStringList{"mediascanner-music"}, "s",
        this, SLOT(resultsInvalidated()));
}

QList<QObject*> MediaStoreWrapper::query(const QString &q, MediaType type) {
    QList<QObject*> result;
    try {
        for (const auto &media : store->query(q.toStdString(), static_cast<mediascanner::MediaType>(type), mediascanner::Filter())) {
            auto wrapper = new MediaFileWrapper(media);
            QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
            result.append(wrapper);
        }
    } catch (const std::exception &e) {
        qWarning() << "Failed to retrieve query results:" << e.what();
    }
    return result;
}

MediaFileWrapper *MediaStoreWrapper::lookup(const QString &filename) {
    MediaFileWrapper *wrapper;
    try {
        wrapper = new MediaFileWrapper(store->lookup(filename.toStdString()));
    } catch (std::exception &e) {
        return nullptr;
    }
    QQmlEngine::setObjectOwnership(wrapper, QQmlEngine::JavaScriptOwnership);
    return wrapper;
}

void MediaStoreWrapper::resultsInvalidated() {
    Q_EMIT updated();
}
