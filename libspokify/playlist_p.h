#ifndef SPOKIFYPLAYLIST_P_H
#define SPOKIFYPLAYLIST_P_H

#include <QString>
#include "Track.h"

struct sp_playlist;

namespace libspokify {

class Playlist;

class PlaylistPrivate {
public:
    PlaylistPrivate(sp_playlist *native);
    ~PlaylistPrivate();

    QString name() const;
    void rename(QString newName);
    QList<Track> tracks() const;

    void notifyTracksAdded();
    void notifyTracksRemoved();
    void notifyTracksMoved();
    void notifyPlaylistRenamed();

    Playlist *q_ptr;
    sp_playlist *m_native;
    QList<Track> m_tracks;

    Q_DECLARE_PUBLIC(Playlist)
};

}

#endif // SPOKIFYPLAYLIST_P_H
