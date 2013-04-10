#include "spokifyplaylistcontainer.h"

#include <libspotify/api.h>
#include <QMap>

#include "spokifyconstructor.h"
#include "Playlist.h"

static QMap<sp_playlistcontainer*, libspokify::SpokifyPlaylistContainer*> SpokifyPlaylistContainers;

void cbPlaylistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(position);
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistAdded();
    }
}

void cbPlaylistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(position);
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistRemoved();
    }
}

void cbPlaylistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int fromPosition, int toPosition, void *userdata) {
    Q_UNUSED(playlist);
    Q_UNUSED(fromPosition);
    Q_UNUSED(toPosition);
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistMoved();
    }
}

void cbContainerLoaded(sp_playlistcontainer *pc, void *userdata) {
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyContainerLoaded();
    }
}

namespace libspokify {

SpokifyPlaylistContainer::SpokifyPlaylistContainer(sp_playlistcontainer *native, QObject *parent) :
    PlaylistContainer(parent),
    m_nativeContainer(native)
{
    Q_ASSERT(m_nativeContainer != 0);

    sp_playlistcontainer_callbacks callbacks;
    callbacks.playlist_added = &cbPlaylistAdded;
    callbacks.playlist_removed = &cbPlaylistRemoved;
    callbacks.playlist_moved = &cbPlaylistMoved;
    callbacks.container_loaded = &cbContainerLoaded;
    sp_playlistcontainer_add_callbacks(m_nativeContainer, &callbacks, NULL);

    SpokifyPlaylistContainers.insert(m_nativeContainer, this);
}

SpokifyPlaylistContainer::~SpokifyPlaylistContainer() {
    SpokifyPlaylistContainers.remove(m_nativeContainer);
}

bool SpokifyPlaylistContainer::addPlaylist(QString playlistName) {
    sp_playlist* pl = sp_playlistcontainer_add_new_playlist(native(), playlistName.toUtf8().data());
    return pl != NULL;
}

bool SpokifyPlaylistContainer::removePlaylist(int index) {
    sp_error result = sp_playlistcontainer_remove_playlist(native(), index);
    return result == SP_ERROR_OK;
}

void SpokifyPlaylistContainer::notifyPlaylistAdded() {
    updatePlaylists();
    emit playlistAdded();
}

void SpokifyPlaylistContainer::notifyPlaylistRemoved() {
    updatePlaylists();
    emit playlistRemoved();
}

void SpokifyPlaylistContainer::notifyPlaylistMoved() {
    updatePlaylists();
    emit playlistMoved();
}

void SpokifyPlaylistContainer::notifyContainerLoaded() {
    updatePlaylists();

    emit containerLoaded();
}

void SpokifyPlaylistContainer::updatePlaylists() {
    int numPlaylists = sp_playlistcontainer_num_playlists(native());

    qDeleteAll(m_playlists);
    m_playlists.clear();
    m_playlists.reserve(numPlaylists);
    // Collect the playlists that are of type "playlist", discard others
    for (int i = 0; i < numPlaylists; ++i) {
        if (sp_playlistcontainer_playlist_type(native(), i) == SP_PLAYLIST_TYPE_PLAYLIST) {
            m_playlists.append(SpokifyConstructor::newPlaylist(sp_playlistcontainer_playlist(native(), i)));
        }
    }
}

sp_playlistcontainer* SpokifyPlaylistContainer::native() const {
    return m_nativeContainer;
}

}
