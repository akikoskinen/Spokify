#include "Track.h"
#include "track_p.h"

#include <libspotify/api.h>

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

QString TrackPrivate::name() const {
    QString ret;
    if (isValid()) {
        ret = QString::fromUtf8(sp_track_name(m_native));
    }
    return ret;
}

int TrackPrivate::duration() const {
    int dur = 0;
    if (isValid()) {
        dur = sp_track_duration(m_native);
    }
    return dur;
}

int TrackPrivate::popularity() const {
    int pop = 0;
    if (isValid()) {
        pop = sp_track_popularity(m_native);
    }
    return pop;
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

QString Track::name() const {
    Q_D(const Track);
    return d->name();
}

int Track::duration() const {
    Q_D(const Track);
    return d->duration();
}

int Track::popularity() const {
    Q_D(const Track);
    return d->popularity();
}

sp_track* Track::native() const {
    Q_D(const Track);
    return d->m_native;
}

}
