#include "spokifyconstructor.h"

#include "Playlist.h"
#include "playlist_p.h"
#include "Track.h"
#include "track_p.h"

namespace libspokify {

Playlist* SpokifyConstructor::newPlaylist(sp_playlist *playlist, QObject *parent) {
    return new Playlist(*new PlaylistPrivate(playlist), parent);
}

Track SpokifyConstructor::newTrack(sp_track *track) {
    return Track(*new TrackPrivate(track));
}

}
