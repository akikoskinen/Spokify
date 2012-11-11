#include "spokifyplaylistcontainer.h"

#include <libspotify/api.h>
#include <QMap>

static QMap<sp_playlistcontainer*, libspokify::SpokifyPlaylistContainer*> SpokifyPlaylistContainers;

void cbPlaylistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistAdded(playlist, position);
    }
}

void cbPlaylistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata) {
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistRemoved(playlist, position);
    }
}

void cbPlaylistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int fromPosition, int toPosition, void *userdata) {
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyPlaylistMoved(playlist, fromPosition, toPosition);
    }
}

void cbContainerLoaded(sp_playlistcontainer *pc, void *userdata) {
    Q_UNUSED(userdata);

    if (SpokifyPlaylistContainers.contains(pc)) {
        SpokifyPlaylistContainers[pc]->notifyContainerLoaded();
    }
}

namespace libspokify {

SpokifyPlaylistContainer::SpokifyPlaylistContainer(QObject *parent) :
    PlaylistContainer(parent),
    m_nativeContainer(0)
{
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

QList<sp_playlist*> SpokifyPlaylistContainer::playlists() const {
    int numPlaylists = sp_playlistcontainer_num_playlists(native());

    // Collect the playlists that are of type "playlist", discard others
    QList<sp_playlist*> playLists;
    playLists.reserve(numPlaylists);
    for (int i = 0; i < numPlaylists; ++i) {
        if (sp_playlistcontainer_playlist_type(native(), i) == SP_PLAYLIST_TYPE_PLAYLIST) {
            playLists.append(sp_playlistcontainer_playlist(native(), i));
        }
    }

    return playLists;
}

void SpokifyPlaylistContainer::notifyPlaylistAdded(sp_playlist *playlist, int position) {
    emit playlistAdded(playlist, position);
}

void SpokifyPlaylistContainer::notifyPlaylistRemoved(sp_playlist *playlist, int position) {
    emit playlistRemoved(playlist, position);
}

void SpokifyPlaylistContainer::notifyPlaylistMoved(sp_playlist *playlist, int fromPosition, int toPosition) {
    emit playlistMoved(playlist, fromPosition, toPosition);
}

void SpokifyPlaylistContainer::notifyContainerLoaded() {
    emit containerLoaded();
}

sp_playlistcontainer* SpokifyPlaylistContainer::native() const {
    return m_nativeContainer;
}

void SpokifyPlaylistContainer::setNative(sp_playlistcontainer *native) {
    if (native != 0) {
        m_nativeContainer = native;

        sp_playlistcontainer_callbacks callbacks;
        callbacks.playlist_added = &cbPlaylistAdded;
        callbacks.playlist_removed = &cbPlaylistRemoved;
        callbacks.playlist_moved = &cbPlaylistMoved;
        callbacks.container_loaded = &cbContainerLoaded;
        sp_playlistcontainer_add_callbacks(m_nativeContainer, &callbacks, NULL);

        SpokifyPlaylistContainers.insert(m_nativeContainer, this);
    }
}

}
