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

#include "MediaFile.hh"
#include "FileTypeDetector.hh"
#include <stdexcept>

using namespace std;

MediaFile::MediaFile(std::string filename) : filename(filename) {
    FileTypeDetector d;
    title = "Media title";
    author = "Some Dude";
    type = d.detect(filename);
    if(type == UnknownMedia) {
        throw runtime_error("Tried to create an invalid media type.");
    }
}

std::string MediaFile::getFileName() const {
    return filename;
}
std::string MediaFile::getTitle() const {
    return title;
}

std::string MediaFile::getAuthor() const {
    return author;
}

MediaType MediaFile::getType() const {
    return type;
}