#include "Track.h"
#include "track_p.h"

namespace libspokify {

TrackPrivate::TrackPrivate(sp_track *native) :
    m_native(native)
{
}

TrackPrivate::~TrackPrivate() {
}

bool TrackPrivate::operator==(const TrackPrivate &other) const {
    return m_native == other.m_native;
}

bool TrackPrivate::isValid() const {
    return m_native != NULL;
}



Track::Track() :
    d_ptr(new TrackPrivate(NULL))
{
}

Track::Track(TrackPrivate &p) :
    d_ptr(&p)
{
}

Track::Track(const Track &track) :
    d_ptr(track.d_ptr)
{
}

Track::~Track() {
}

Track& Track::operator=(const Track &other) {
    d_ptr = other.d_ptr;
    return *this;
}

bool Track::operator==(const Track &other) const {
    Q_D(const Track);
    return (*d) == (*other.d_ptr);
}

bool Track::isValid() const {
    Q_D(const Track);
    return d->isValid();
}

sp_track* Track::native() const {
    Q_D(const Track);
    return d->m_native;
}

}
