#ifndef LIBSPOKIFY_PLAYLIST_H
#define LIBSPOKIFY_PLAYLIST_H

#include <QObject>

struct sp_playlist;

namespace libspokify {

class Track;

class Playlist : public QObject {
    Q_OBJECT

public:
    explicit Playlist(QObject *parent = 0);

    virtual ~Playlist();

    virtual QString name() const = 0;

    virtual void rename(QString newName) = 0;

    QList<const Track*> tracks() const;

    // TODO remove this from the public interface when it's not needed anymore
    sp_playlist* native() const;

protected:
    QList<const Track*> m_tracks;

    sp_playlist *m_nativePlaylist;

};

}

#endif // LIBSPOKIFY_PLAYLIST_H
