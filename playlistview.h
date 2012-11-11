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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QtGui/QListView>

namespace libspokify {
class PlaylistContainer;
}

class KMenu;

class PlaylistView
    : public QListView
{
    Q_OBJECT

public:
    PlaylistView(QWidget *parent = 0);
    virtual ~PlaylistView();

    void setPlaylistContainer(libspokify::PlaylistContainer *playlistContainer);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

private Q_SLOTS:
    void newPlaylistSlot();
    void renamePlaylistSlot();
    void deletePlaylistSlot();

private:
    libspokify::PlaylistContainer *m_playlistContainer;
    KMenu *m_contextMenu;
    KMenu *m_contextMenuEmpty;
};

#endif
