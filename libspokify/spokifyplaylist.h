#ifndef SPOKIFYPLAYLIST_H
#define SPOKIFYPLAYLIST_H

#include "Playlist.h"

struct sp_playlist;

namespace libspokify {

class SpokifyPlaylist : public Playlist {
    Q_OBJECT

public:
    explicit SpokifyPlaylist(sp_playlist *native, QObject *parent = 0);

    virtual ~SpokifyPlaylist();

};

}

#endif // SPOKIFYPLAYLIST_H
