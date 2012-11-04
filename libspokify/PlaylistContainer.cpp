#include "PlaylistContainer.h"

namespace libspokify {

PlaylistContainer::PlaylistContainer(QObject *parent) :
    QObject(parent),
    m_nativeContainer(0)
{
}

PlaylistContainer::~PlaylistContainer() {
}

sp_playlistcontainer* PlaylistContainer::native() const {
    return m_nativeContainer;
}

}
