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

void SpokifyPlaylist::rename(QString newName) {
    sp_playlist_rename(native(), newName.toUtf8().data());
}

}
