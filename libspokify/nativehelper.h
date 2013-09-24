#ifndef NATIVEHELPER_H
#define NATIVEHELPER_H

struct sp_playlist;

namespace libspokify {

class Playlist;

class NativeHelper {
public:
    static bool PlaylistNativeIs(Playlist *playlist, sp_playlist *nativePlaylist);
};

}

#endif // NATIVEHELPER_H
