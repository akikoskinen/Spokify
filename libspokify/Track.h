#ifndef TRACK_H
#define TRACK_H

struct sp_track;

namespace libspokify {

class Track {
public:
    virtual ~Track();

    sp_track* native() const;

protected:
    Track();

    sp_track *m_native;
};

}

#endif // TRACK_H
