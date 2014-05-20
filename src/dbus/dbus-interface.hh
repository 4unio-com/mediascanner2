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
#ifndef MEDIASCANNER_DBUS_INTERFACE_HH
#define MEDIASCANNER_DBUS_INTERFACE_HH

#include <chrono>
#include <string>

#include <core/dbus/object.h>
#include <core/dbus/service.h>

namespace mediascanner {
namespace dbus {

struct MediaStoreInterface {
    inline static const std::string& name() {
        static std::string s = "com.canonical.MediaScanner2";
        return s;
    }

    struct Errors {
        struct Error {
            inline static const std::string& name() {
                static std::string s = "com.canonical.MediaScanner2.Error";
                return s;
            }
        };
    };

    struct Lookup {
        typedef MediaStoreInterface Interface;

        inline static const std::string& name() {
            static std::string s = "Lookup";
            return s;
        }

        inline static const std::chrono::milliseconds default_timeout() {
            return std::chrono::seconds{1};
        }
    };

    struct Query {
        typedef MediaStoreInterface Interface;

        inline static const std::string& name() {
            static std::string s = "Query";
            return s;
        }

        inline static const std::chrono::milliseconds default_timeout() {
            return std::chrono::seconds{1};
        }
    };
};

}
}
#endif
