#ifndef TRACK_H
#define TRACK_H

struct sp_track;

#include <QSharedPointer>

namespace libspokify {

class TrackPrivate;

class Track {
public:
    Track();

    Track(const Track &track);

    ~Track();

    Track& operator=(const Track &other);

    bool operator==(const Track &other) const;

    bool isValid() const;

    QString name() const;

    /**
     * Returns the duration of the track in milliseconds.
     */
    int duration() const;

    sp_track* native() const;

private:
    Track(TrackPrivate &p);

    QSharedPointer<TrackPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Track)

    friend class SpokifyConstructor;
};

}

#endif // TRACK_H
