#include "spokifyconstructor.h"

#include "PlaylistContainer.h"
#include "playlistcontainer_p.h"
#include "Playlist.h"
#include "playlist_p.h"
#include "Track.h"
#include "track_p.h"

namespace libspokify {

PlaylistContainer* SpokifyConstructor::newPlaylistContainer(sp_playlistcontainer *plc, sp_session *session, QObject *parent) {
    return new PlaylistContainer(*new PlaylistContainerPrivate(plc, session), parent);
}

Playlist* SpokifyConstructor::newPlaylist(sp_playlist *playlist, sp_session *session, QObject *parent) {
    return new Playlist(*new PlaylistPrivate(playlist, session), parent);
}

Track SpokifyConstructor::newTrack(sp_track *track) {
    return Track(*new TrackPrivate(track));
}

}
