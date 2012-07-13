#ifndef LIBSPOKIFY_PLAYER_H
#define LIBSPOKIFY_PLAYER_H

#include <QObject>

struct sp_track;

namespace libspokify {

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject *parent = 0);

    virtual ~Player();

    /**
     * Seeks to a position in the current track.
     * @param position position in track in milliseconds.
     */
    virtual void seek(unsigned int position) = 0;

    virtual void load(sp_track *track) = 0;
    virtual void unload() = 0;

    virtual void play() = 0;
    virtual void pause() = 0;

};

}

#endif // LIBSPOKIFY_PLAYER_H
