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

#include"qtservice.h"
#include<QCoreApplication>
#include<QDBusConnection>

using namespace mediascanner;

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const char *busname = getenv("MEDIASCANNER_DBUS_NAME");

    if(argc > 1) {
        busname = argv[1];
    }
    if(!busname) {
        busname = MS_DBUS_NAME;
    }
    QtService *s = new QtService(&app);
    if(!QDBusConnection::sessionBus().registerService(busname)) {
        printf("DBus service name %s already taken.\n", busname);
        return 1;
    }
    if(!QDBusConnection::sessionBus().registerObject(MS_DBUS_OBJECT, &app)) {
        printf("Could not register to DBus session.\n");
        return 1;
    }
    app.exec();
    return 0;
}
