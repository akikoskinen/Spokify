#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QObject>

struct sp_playlistcontainer;
struct sp_playlist;

namespace libspokify {

class PlaylistContainer : public QObject {
    Q_OBJECT

public:
    explicit PlaylistContainer(QObject *parent = 0);

    virtual ~PlaylistContainer();

    // TODO remove this when the internal type is not needed anymore
    sp_playlistcontainer* native() const;

Q_SIGNALS:
    void playlistAdded(sp_playlist *playlist, int position);
    void playlistRemoved(sp_playlist *playlist, int position);
    void playlistMoved(sp_playlist *playlist, int fromPosition, int toPosition);
    void containerLoaded();

protected:
    sp_playlistcontainer *m_nativeContainer;
};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
