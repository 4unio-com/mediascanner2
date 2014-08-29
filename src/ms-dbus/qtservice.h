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
#include <QStringList>
#include <QDBusVariant>

/*
typedef QMap<QString, QDBusVariant> Hints;

Q_DECLARE_METATYPE(Hints)
*/

#define MS_DBUS_INTERFACE "com.canonical.MediaScanner2"
#define MS_DBUS_OBJECT "/com/canonical/MediaScanner2"
#define MS_DBUS_NAME "com.canonical.MediaScanner2"

class QtService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", MS_DBUS_INTERFACE)

public:
    explicit QtService(QObject *parent=nullptr);
    ~QtService();

public Q_SLOTS:
    QString ping();
};

#endif
