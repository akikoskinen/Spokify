#ifndef SPOKIFYTRACK_P_H
#define SPOKIFYTRACK_P_H

#include "Track.h"

namespace libspokify {

class TrackPrivate {
public:
    TrackPrivate(sp_track *native);

    ~TrackPrivate();

    bool operator==(const TrackPrivate &other) const;

    bool isValid() const;

    QString name() const;

    int duration() const;

    sp_track *m_native;
};

}

#endif // SPOKIFYTRACK_P_H
