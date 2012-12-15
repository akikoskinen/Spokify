#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QObject>

namespace libspokify {

class Playlist;

class PlaylistContainer : public QObject {
    Q_OBJECT

public:
    explicit PlaylistContainer(QObject *parent = 0);

    virtual ~PlaylistContainer();

    /**
     * Adds a new playlist to the playlist container.
     * Returns \c true if the addition was successful, \c false if not.
     */
    virtual bool addPlaylist(QString playlistName) = 0;

    /**
     * Removes a playlist at \a index.
     * Returns \c true if the removal was successful, \c false if not.
     */
    virtual bool removePlaylist(int index) = 0;

    virtual QList<Playlist*> playlists() const;

Q_SIGNALS:
    void playlistAdded();
    void playlistRemoved();
    void playlistMoved();
    void containerLoaded();

protected:
    QList<Playlist*> m_playlists;

};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
