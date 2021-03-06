/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *    James Henstridge <james.henstridge@canonical.com>
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

#ifndef EXTRACTOR_IMAGEEXTRACTOR_H
#define EXTRACTOR_IMAGEEXTRACTOR_H

#include <string>

namespace mediascanner {

class MediaFileBuilder;
struct DetectedFile;

class ImageExtractor final {
public:
    ImageExtractor() = default;
    ~ImageExtractor() = default;
    ImageExtractor(const ImageExtractor&) = delete;
    ImageExtractor& operator=(ImageExtractor &o) = delete;

    void extract(const DetectedFile &d, MediaFileBuilder &builder);

private:
    bool extract_exif(const DetectedFile &d, MediaFileBuilder &builder);
    void extract_pixbuf(const DetectedFile &d, MediaFileBuilder &builder);
};

}

#endif
