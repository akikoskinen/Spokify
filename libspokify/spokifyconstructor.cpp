#include "spokifyconstructor.h"

#include "Track.h"
#include "track_p.h"

namespace libspokify {

Track SpokifyConstructor::newTrack(sp_track *track) {
    return Track(*new TrackPrivate(track));
}

}
