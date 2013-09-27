#ifndef SPOKIFYCONSTRUCTOR_H
#define SPOKIFYCONSTRUCTOR_H

class QObject;

struct sp_session;
struct sp_playlistcontainer;
struct sp_playlist;
struct sp_track;

namespace libspokify {

class PlaylistContainer;
class Playlist;
class Track;

class SpokifyConstructor {
public:
    static PlaylistContainer* newPlaylistContainer(sp_playlistcontainer *plc, sp_session *session, QObject *parent = 0);
    static Playlist* newPlaylist(sp_playlist *playlist, sp_session *session, QObject *parent = 0);
    static Track newTrack(sp_track *track);
};

}

#endif // SPOKIFYCONSTRUCTOR_H
