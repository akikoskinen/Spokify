#include "PlaylistContainer.h"

namespace libspokify {

PlaylistContainer::PlaylistContainer(QObject *parent) :
    QObject(parent)
{
}

PlaylistContainer::~PlaylistContainer() {
}

QList<Playlist*> PlaylistContainer::playlists() const {
    return m_playlists;
}

}
