#include "spokifyconstructor.h"

#include "PlaylistContainer.h"
#include "playlistcontainer_p.h"
#include "Playlist.h"
#include "playlist_p.h"
#include "Track.h"
#include "track_p.h"

namespace libspokify {

PlaylistContainer* SpokifyConstructor::newPlaylistContainer(sp_playlistcontainer *plc, QObject *parent) {
    return new PlaylistContainer(*new PlaylistContainerPrivate(plc), parent);
}

Playlist* SpokifyConstructor::newPlaylist(sp_playlist *playlist, QObject *parent) {
    return new Playlist(*new PlaylistPrivate(playlist), parent);
}

Track SpokifyConstructor::newTrack(sp_track *track) {
    return Track(*new TrackPrivate(track));
}

}
