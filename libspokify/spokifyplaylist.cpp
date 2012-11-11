#include "spokifyplaylist.h"

namespace libspokify {

SpokifyPlaylist::SpokifyPlaylist(sp_playlist *native, QObject *parent) :
    Playlist(parent)
{
    m_nativePlaylist = native;
}

SpokifyPlaylist::~SpokifyPlaylist() {
}

}
