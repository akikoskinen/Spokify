#ifndef SPOKIFYPLAYLISTCONTAINER_H
#define SPOKIFYPLAYLISTCONTAINER_H

#include "PlaylistContainer.h"

struct sp_playlistcontainer;

namespace libspokify {

class SpokifyPlaylistContainer : public PlaylistContainer {
    Q_OBJECT

public:
    explicit SpokifyPlaylistContainer(sp_playlistcontainer *native, QObject *parent = 0);

    virtual ~SpokifyPlaylistContainer();

    virtual bool addPlaylist(QString playlistName);

    virtual bool removePlaylist(int index);

    virtual QList<sp_playlist*> playlists() const;

    void notifyPlaylistAdded(sp_playlist *playlist, int position);
    void notifyPlaylistRemoved(sp_playlist *playlist, int position);
    void notifyPlaylistMoved(sp_playlist *playlist, int fromPosition, int toPosition);
    void notifyContainerLoaded();

    sp_playlistcontainer* native() const;

private:
    sp_playlistcontainer *m_nativeContainer;

};

}

#endif // SPOKIFYPLAYLISTCONTAINER_H
