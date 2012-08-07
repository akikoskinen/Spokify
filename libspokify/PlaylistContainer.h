#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QObject>

struct sp_playlistcontainer;

namespace libspokify {

class Session;

class PlaylistContainer : public QObject {
    Q_OBJECT

public:
    /**
     * Creates a playlist container object using the supplied \a session.
     */
    explicit PlaylistContainer(const Session &session, QObject *parent = 0);

    virtual ~PlaylistContainer();

    // TODO remove this when the internal type is not needed anymore
    sp_playlistcontainer* native() const;

private:
    const Session &m_session;
};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
