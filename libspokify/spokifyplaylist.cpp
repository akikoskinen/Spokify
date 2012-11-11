#include "spokifyplaylist.h"

#include <libspotify/api.h>

namespace libspokify {

SpokifyPlaylist::SpokifyPlaylist(sp_playlist *native, QObject *parent) :
    Playlist(parent)
{
    m_nativePlaylist = native;
}

SpokifyPlaylist::~SpokifyPlaylist() {
}

QString SpokifyPlaylist::name() const {
    return QString::fromUtf8(sp_playlist_name(native()));
}

}
