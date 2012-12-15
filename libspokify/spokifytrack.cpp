#include "spokifytrack.h"

namespace libspokify {

SpokifyTrack::SpokifyTrack(sp_track *native) {
    m_native = native;
}

SpokifyTrack::~SpokifyTrack() {
}

}
