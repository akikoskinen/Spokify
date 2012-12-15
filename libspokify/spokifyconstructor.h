#ifndef SPOKIFYCONSTRUCTOR_H
#define SPOKIFYCONSTRUCTOR_H

struct sp_track;

namespace libspokify {

class Track;

class SpokifyConstructor {
public:
    static Track newTrack(sp_track *track);
};

}

#endif // SPOKIFYCONSTRUCTOR_H
