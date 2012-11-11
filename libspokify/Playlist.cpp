#include "Playlist.h"

namespace libspokify {

Playlist::Playlist(QObject *parent) :
    QObject(parent),
    m_nativePlaylist(0)
{
}

Playlist::~Playlist() {
}

sp_playlist* Playlist::native() const {
    return m_nativePlaylist;
}

}
