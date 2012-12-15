#include "Track.h"

namespace libspokify {

Track::Track() :
    m_native(0)
{
}

Track::~Track() {
}

sp_track* Track::native() const {
    return m_native;
}

}
