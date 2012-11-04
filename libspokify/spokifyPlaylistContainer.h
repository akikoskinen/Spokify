#ifndef SPOKIFYPLAYLISTCONTAINER_H
#define SPOKIFYPLAYLISTCONTAINER_H

#include "PlaylistContainer.h"

namespace libspokify {

class SpokifyPlaylistContainer : public PlaylistContainer {
    Q_OBJECT

public:
    explicit SpokifyPlaylistContainer(QObject *parent = 0);

    virtual ~SpokifyPlaylistContainer();

    void notifyPlaylistAdded(sp_playlist *playlist, int position);
    void notifyPlaylistRemoved(sp_playlist *playlist, int position);
    void notifyPlaylistMoved(sp_playlist *playlist, int fromPosition, int toPosition);
    void notifyContainerLoaded();

    void setNative(sp_playlistcontainer *native);

};

}

#endif // SPOKIFYPLAYLISTCONTAINER_H
