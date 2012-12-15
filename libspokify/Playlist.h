#ifndef LIBSPOKIFY_PLAYLIST_H
#define LIBSPOKIFY_PLAYLIST_H

#include <QObject>
#include <QScopedPointer>

#include "Track.h"

struct sp_playlist;

namespace libspokify {

class PlaylistPrivate;

class Playlist : public QObject {
    Q_OBJECT

public:
    virtual ~Playlist();

    QString name() const;

    void rename(QString newName);

    QList<Track> tracks() const;

    // TODO remove this from the public interface when it's not needed anymore
    sp_playlist* native() const;

Q_SIGNALS:
    void tracksAdded();
    void tracksRemoved();
    void tracksMoved();
    void playlistRenamed();

private:
    explicit Playlist(PlaylistPrivate &p, QObject *parent = 0);

    QScopedPointer<PlaylistPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Playlist)

    Q_DISABLE_COPY(Playlist)

    friend class SpokifyConstructor;

};

}

#endif // LIBSPOKIFY_PLAYLIST_H
