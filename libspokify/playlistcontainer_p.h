#ifndef PLAYLISTCONTAINER_P_H
#define PLAYLISTCONTAINER_P_H

#include <QList>

struct sp_playlistcontainer;
struct sp_session;

namespace libspokify {

class PlaylistContainer;
class Playlist;

class PlaylistContainerPrivate {
public:
    PlaylistContainerPrivate(sp_playlistcontainer *plc, sp_session *session);
    ~PlaylistContainerPrivate();

    bool addPlaylist(QString playlistName);
    bool removePlaylist(int index);
    QList<Playlist*> playlists() const;

    void updatePlaylists();
    void notifyPlaylistAdded();
    void notifyPlaylistRemoved();
    void notifyPlaylistMoved();
    void notifyContainerLoaded();

    PlaylistContainer *q_ptr;
    sp_playlistcontainer *m_nativeContainer;
    sp_session *m_session;
    QList<Playlist*> m_playlists;

    Q_DECLARE_PUBLIC(PlaylistContainer)
};

}

#endif // PLAYLISTCONTAINER_P_H
