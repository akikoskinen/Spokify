#ifndef SPOKIFYPLAYLIST_P_H
#define SPOKIFYPLAYLIST_P_H

#include <QString>
#include "Track.h"

struct sp_playlist;

namespace libspokify {

class PlaylistPrivate {
public:
    PlaylistPrivate(sp_playlist *native);

    QString name() const;
    void rename(QString newName);
    QList<Track> tracks() const;

    sp_playlist *m_native;
    QList<Track> m_tracks;
};

}

#endif // SPOKIFYPLAYLIST_P_H
