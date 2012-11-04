#ifndef LIBSPOKIFY_PLAYLISTCONTAINER_H
#define LIBSPOKIFY_PLAYLISTCONTAINER_H

#include <QObject>

struct sp_playlistcontainer;

namespace libspokify {

class PlaylistContainer : public QObject {
    Q_OBJECT

public:
    explicit PlaylistContainer(QObject *parent = 0);

    virtual ~PlaylistContainer();

    // TODO remove this from the public interface when the internal type is not needed anymore
    void setNative(sp_playlistcontainer *native);

    // TODO remove this when the internal type is not needed anymore
    sp_playlistcontainer* native() const;

private:
    sp_playlistcontainer *m_nativeContainer;
};

}

#endif // LIBSPOKIFY_PLAYLISTCONTAINER_H
