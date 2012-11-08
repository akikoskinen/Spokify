#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QObject>

struct sp_playlist;

namespace libspokify {

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

    virtual QList<sp_playlist*> playlists() const = 0;

Q_SIGNALS:
    void playlistAdded(sp_playlist *playlist, int position);
    void playlistRemoved(sp_playlist *playlist, int position);
    void playlistMoved(sp_playlist *playlist, int fromPosition, int toPosition);
    void containerLoaded();

};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
