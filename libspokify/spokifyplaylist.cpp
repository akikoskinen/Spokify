#include "spokifyplaylist.h"

#include "spokifyconstructor.h"

#include <libspotify/api.h>

namespace libspokify {

SpokifyPlaylist::SpokifyPlaylist(sp_playlist *native, QObject *parent) :
    Playlist(parent)
{
    m_nativePlaylist = native;

    const int numTracks = sp_playlist_num_tracks(m_nativePlaylist);
    for (int i = 0; i < numTracks; ++i) {
        m_tracks.append(SpokifyConstructor::newTrack(sp_playlist_track(m_nativePlaylist, i)));
    }
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
