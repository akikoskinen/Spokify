#include "PlaylistContainer.h"
#include "playlistcontainer_p.h"
#include "Playlist.h"
#include "spokifyconstructor.h"
#include "nativehelper.h"

#include <QMap>
#include <libspotify/api.h>

static QMap<sp_playlistcontainer*, libspokify::PlaylistContainerPrivate*> PlaylistContainers;

void cbPlaylistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(position);
    Q_UNUSED(userdata);

    if (PlaylistContainers.contains(pc)) {
        PlaylistContainers[pc]->notifyPlaylistAdded();
    }
}

void cbPlaylistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(position);
    Q_UNUSED(userdata);

    if (PlaylistContainers.contains(pc)) {
        PlaylistContainers[pc]->notifyPlaylistRemoved();
    }
}

void cbPlaylistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int fromPosition, int toPosition, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(fromPosition);
    Q_UNUSED(toPosition);
    Q_UNUSED(userdata);

    if (PlaylistContainers.contains(pc)) {
        PlaylistContainers[pc]->notifyPlaylistMoved();
    }
}

void cbContainerLoaded(sp_playlistcontainer *pc, void *userdata) {
    Q_UNUSED(userdata);

    if (PlaylistContainers.contains(pc)) {
        PlaylistContainers[pc]->notifyContainerLoaded();
    }
}

namespace libspokify {

PlaylistContainerPrivate::PlaylistContainerPrivate(sp_playlistcontainer *plc) :
    m_nativeContainer(plc)
{
    Q_ASSERT(m_nativeContainer != 0);

    PlaylistContainers.insert(m_nativeContainer, this);
}

PlaylistContainerPrivate::~PlaylistContainerPrivate() {
    PlaylistContainers.remove(m_nativeContainer);
    qDeleteAll(m_playlists);
}

bool PlaylistContainerPrivate::addPlaylist(QString playlistName) {
    sp_playlist* pl = sp_playlistcontainer_add_new_playlist(m_nativeContainer, playlistName.toUtf8().data());
    return pl != NULL;
}

bool PlaylistContainerPrivate::removePlaylist(int index) {
    sp_error result = sp_playlistcontainer_remove_playlist(m_nativeContainer, index);
    return result == SP_ERROR_OK;
}

QList<Playlist*> PlaylistContainerPrivate::playlists() const {
    return m_playlists;
}

void PlaylistContainerPrivate::updatePlaylists() {
    int numPlaylists = sp_playlistcontainer_num_playlists(m_nativeContainer);

    QList<Playlist*> oldPlaylists = m_playlists;
    m_playlists.clear();
    m_playlists.reserve(numPlaylists);
    for (int i = 0; i < numPlaylists; ++i) {
        // Collect the playlists that are of type "playlist", discard others
        if (sp_playlistcontainer_playlist_type(m_nativeContainer, i) == SP_PLAYLIST_TYPE_PLAYLIST) {
            Playlist *playlist = NULL;
            sp_playlist *native_playlist = sp_playlistcontainer_playlist(m_nativeContainer, i);
            foreach (Playlist *pl, oldPlaylists) {
                if (NativeHelper::PlaylistNativeIs(pl, native_playlist)) {
                    playlist = pl;
                    oldPlaylists.removeAll(pl);
                    break;
                }
            }

            if (playlist == NULL) {
                playlist = SpokifyConstructor::newPlaylist(native_playlist);
            }

            m_playlists.append(playlist);
        }
    }

    qDeleteAll(oldPlaylists);
}

void PlaylistContainerPrivate::notifyPlaylistAdded() {
    Q_Q(PlaylistContainer);
    updatePlaylists();
    emit q->playlistAdded();
}

void PlaylistContainerPrivate::notifyPlaylistRemoved() {
    Q_Q(PlaylistContainer);
    updatePlaylists();
    emit q->playlistRemoved();
}

void PlaylistContainerPrivate::notifyPlaylistMoved() {
    Q_Q(PlaylistContainer);
    updatePlaylists();
    emit q->playlistMoved();
}

void PlaylistContainerPrivate::notifyContainerLoaded() {
    Q_Q(PlaylistContainer);
    updatePlaylists();
    emit q->containerLoaded();
}



PlaylistContainer::PlaylistContainer(PlaylistContainerPrivate &p, QObject *parent) :
    QObject(parent),
    d_ptr(&p)
{
    Q_D(PlaylistContainer);
    d->q_ptr = this;

    sp_playlistcontainer_callbacks callbacks;
    callbacks.playlist_added = &cbPlaylistAdded;
    callbacks.playlist_removed = &cbPlaylistRemoved;
    callbacks.playlist_moved = &cbPlaylistMoved;
    callbacks.container_loaded = &cbContainerLoaded;
    sp_playlistcontainer_add_callbacks(p.m_nativeContainer, &callbacks, NULL);
}

bool PlaylistContainer::addPlaylist(QString playlistName) {
    Q_D(PlaylistContainer);
    return d->addPlaylist(playlistName);
}

bool PlaylistContainer::removePlaylist(int index) {
    Q_D(PlaylistContainer);
    return d->removePlaylist(index);
}

QList<Playlist*> PlaylistContainer::playlists() const {
    Q_D(const PlaylistContainer);
    return d->playlists();
}

}
