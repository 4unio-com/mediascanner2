/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include "DBusTest.h"
#include "test_config.h"

#include <QtTest/QSignalSpy>
#include <libqtdbustest/QProcessDBusService.h>
#include <QtDBus/QtDBus>

using namespace MediaScannerTestUtils;
using namespace QtDBusTest;

DBusTest::DBusTest() {
    qputenv("MEDIASCANNER_CACHEDIR", TEST_DIR);
    DBusServicePtr dBusService(
            new QProcessDBusService("com.canonical.MediaScanner2",
                    QDBusConnection::SessionBus, MS_DBUS_BINARY,
                    QStringList()));
    dbus.registerService(dBusService);
}

DBusTest::~DBusTest() {
}

void DBusTest::SetUp() {
    dbus.startServices();
}

void DBusTest::TearDown() {
}

const QDBusConnection & DBusTest::systemConnection() const {
    return dbus.systemConnection();
}
