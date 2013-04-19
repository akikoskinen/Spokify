#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QSharedPointer>

namespace libspokify {

class Playlist;
class PlaylistContainerPrivate;

class PlaylistContainer : public QObject {
    Q_OBJECT

public:
    /**
     * Adds a new playlist to the playlist container.
     * Returns \c true if the addition was successful, \c false if not.
     */
    bool addPlaylist(QString playlistName);

    /**
     * Removes a playlist at \a index.
     * Returns \c true if the removal was successful, \c false if not.
     */
    bool removePlaylist(int index);

    QList<Playlist*> playlists() const;

Q_SIGNALS:
    void playlistAdded();
    void playlistRemoved();
    void playlistMoved();
    void containerLoaded();

private:
    explicit PlaylistContainer(PlaylistContainerPrivate &p, QObject *parent = 0);

    QSharedPointer<PlaylistContainerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PlaylistContainer)
    Q_DISABLE_COPY(PlaylistContainer)

    friend class SpokifyConstructor;
};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
