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

    sp_playlist* const m_native;

    Q_DECLARE_PUBLIC(Playlist)

private:
    Playlist *q_ptr;
    sp_session *m_session;
    mutable QList<Track> m_tracks;

    void setTracksDirty();
    void ensureTracks() const;
};

}

#endif // SPOKIFYPLAYLIST_P_H
