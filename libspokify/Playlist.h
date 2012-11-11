#ifndef LIBSPOKIFY_PLAYLIST_H
#define LIBSPOKIFY_PLAYLIST_H

#include <QObject>

struct sp_playlist;

namespace libspokify {

class Playlist : public QObject {
    Q_OBJECT

public:
    explicit Playlist(QObject *parent = 0);

    virtual ~Playlist();

    virtual QString name() const = 0;

    // TODO remove this from the public interface when it's not needed anymore
    sp_playlist* native() const;

protected:
    sp_playlist *m_nativePlaylist;

};

}

#endif // LIBSPOKIFY_PLAYLIST_H
