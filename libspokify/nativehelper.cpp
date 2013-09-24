#include "nativehelper.h"
#include "Playlist.h"
#include "playlist_p.h"

namespace libspokify {

bool NativeHelper::PlaylistNativeIs(Playlist *playlist, sp_playlist *nativePlaylist) {
    return playlist->d_ptr->m_native == nativePlaylist;
}

}
