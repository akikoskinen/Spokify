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

#include "playlistview.h"
#include "mimedata.h"
#include "playlistmodel.h"
#include "libspokify/Session.h"
#include "libspokify/PlaylistContainer.h"
#include "libspokify/Playlist.h"

#include <libspotify/api.h>

#include <QtGui/QLabel>
#include <QtGui/QBoxLayout>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>

#include <KMenu>
#include <KLocale>
#include <KDialog>
#include <KLineEdit>
#include <KMessageBox>

using namespace libspokify;

PlaylistView::PlaylistView(QWidget *parent)
    : QListView(parent)
    , m_playlistContainer(0)
    , m_contextMenu(new KMenu(this))
    , m_contextMenuEmpty(new KMenu(this))
{
    QAction *const newPlaylist = m_contextMenu->addAction(i18n("Create new Playlist"));
    m_contextMenu->addSeparator();
    QAction *const renamePlaylist = m_contextMenu->addAction(i18n("Rename Playlist"));
    QAction *const deletePlaylist = m_contextMenu->addAction(i18n("Delete Playlist"));

    m_contextMenuEmpty->addAction(newPlaylist);

    connect(newPlaylist, SIGNAL(triggered()), this, SLOT(newPlaylistSlot()));
    connect(renamePlaylist, SIGNAL(triggered()), this, SLOT(renamePlaylistSlot()));
    connect(deletePlaylist, SIGNAL(triggered()), this, SLOT(deletePlaylistSlot()));
}

PlaylistView::~PlaylistView()
{
}

void PlaylistView::setPlaylistContainer(PlaylistContainer *playlistContainer) {
    m_playlistContainer = playlistContainer;
}

void PlaylistView::dragEnterEvent(QDragEnterEvent *event)
{
    if (dynamic_cast<const MimeData*>(event->mimeData())) {
        // ### Check if playlist is ours. If not, check if collaborative
        event->accept();
    }
}

void PlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    if (dynamic_cast<const MimeData*>(event->mimeData())) {
        // ### Check if playlist is ours. If not, check if collaborative
        event->accept();
    }
}

void PlaylistView::dropEvent(QDropEvent *event)
{
    const QModelIndex target = indexAt(event->pos());
    if (!target.isValid()) {
        return;
    }
    const MimeData *mimeData = static_cast<const MimeData*>(event->mimeData());
    sp_track* const trackToAdd[] = { mimeData->track() };
    sp_playlist *targetPlaylist = target.data(PlaylistModel::PlaylistRole).value<Playlist*>()->native();
    sp_playlist_add_tracks(targetPlaylist, trackToAdd, 1, sp_playlist_num_tracks(targetPlaylist), Session().session());
}

void PlaylistView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        m_contextMenu->popup(QPoint(event->globalX(), event->globalY()));
    } else if (m_playlistContainer != 0) {
        m_contextMenuEmpty->popup(QPoint(event->globalX(), event->globalY()));
    }
}

void PlaylistView::newPlaylistSlot()
{
    if (m_playlistContainer != 0) {
        KDialog *dialog = new KDialog(this);
        dialog->setCaption(i18n("New Playlist"));
        dialog->setButtons(KDialog::Ok | KDialog::Cancel);

        QWidget *widget = new QWidget(dialog);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(new QLabel(i18n("Choose a name for the new playlist:"), widget));
        KLineEdit *playlistName = new KLineEdit(widget);
        playlistName->setClearButtonShown(true);
        layout->addWidget(playlistName);
        widget->setLayout(layout);
        dialog->setMainWidget(widget);
        playlistName->setFocus();

        if (dialog->exec() == KDialog::Accepted && !playlistName->text().isEmpty()) {
            m_playlistContainer->addPlaylist(playlistName->text());
        }
    }
}

void PlaylistView::renamePlaylistSlot()
{
    Playlist *targetPlaylist = currentIndex().data(PlaylistModel::PlaylistRole).value<Playlist*>();

    KDialog *dialog = new KDialog(this);
    dialog->setCaption(i18n("Rename Playlist"));
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget *widget = new QWidget(dialog);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(i18n("Choose a new name for the playlist \"%1\":", targetPlaylist->name()), widget));
    KLineEdit *playlistName = new KLineEdit(widget);
    playlistName->setClearButtonShown(true);
    playlistName->setText(targetPlaylist->name());
    layout->addWidget(playlistName);
    widget->setLayout(layout);
    dialog->setMainWidget(widget);
    playlistName->selectAll();
    playlistName->setFocus();

    if (dialog->exec() == KDialog::Accepted) {
        targetPlaylist->rename(playlistName->text());
    }
}

void PlaylistView::deletePlaylistSlot()
{
    if (m_playlistContainer != 0) {
        Playlist *targetPlaylist = currentIndex().data(PlaylistModel::PlaylistRole).value<Playlist*>();
        if (KMessageBox::questionYesNo(this, i18n("Are you sure that you want to delete the playlist \"%1\"?", targetPlaylist->name()),
                                             i18n("Delete Playlist")) == KMessageBox::Yes) {
            m_playlistContainer->removePlaylist(currentIndex().row());
        }
    }
}
