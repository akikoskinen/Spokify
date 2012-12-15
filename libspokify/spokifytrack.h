#ifndef SPOKIFYTRACK_H
#define SPOKIFYTRACK_H

#include "Track.h"

namespace libspokify {

class SpokifyTrack : public Track {
public:
    SpokifyTrack(sp_track *native);

    virtual ~SpokifyTrack();
};

}

#endif // SPOKIFYTRACK_H
