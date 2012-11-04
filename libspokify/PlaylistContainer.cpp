#include "PlaylistContainer.h"

namespace libspokify {

PlaylistContainer::PlaylistContainer(QObject *parent) :
    QObject(parent),
    m_nativeContainer(0)
{
}

PlaylistContainer::~PlaylistContainer() {
}

void PlaylistContainer::setNative(sp_playlistcontainer *native) {
    m_nativeContainer = native;
}

sp_playlistcontainer* PlaylistContainer::native() const {
    return m_nativeContainer;
}

}
