#ifndef SPOKIFYPLAYLIST_P_H
#define SPOKIFYPLAYLIST_P_H

#include <QString>
#include "Track.h"

struct sp_playlist;
struct sp_session;

namespace libspokify {

class Playlist;

class PlaylistPrivate {
public:
    PlaylistPrivate(sp_playlist *native, sp_session *session);
    ~PlaylistPrivate();

    bool operator==(const PlaylistPrivate &other) const;

    QString name() const;
    void rename(QString newName);
    QList<Track> tracks() const;
    void addTrack(const Track &track);

    void notifyTracksAdded();
    void notifyTracksRemoved();
    void notifyTracksMoved();
    void notifyPlaylistRenamed();

    Playlist *q_ptr;
    sp_playlist *m_native;
    sp_session *m_session;
    QList<Track> m_tracks;

    Q_DECLARE_PUBLIC(Playlist)
};

}

#endif // SPOKIFYPLAYLIST_P_H
