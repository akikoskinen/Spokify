/*
 * This file is part of Spokify.
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 *
 * Spokify is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Spokify is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Spokify.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIMEDATA_H
#define MIMEDATA_H

#include <QtCore/QMimeData>

#include "libspokify/Track.h"

class MimeData
    : public QMimeData
{
public:
    MimeData();
    virtual ~MimeData();

    void setTrack(const libspokify::Track &track);
    libspokify::Track track() const;

private:
    libspokify::Track m_track;
};

#endif
