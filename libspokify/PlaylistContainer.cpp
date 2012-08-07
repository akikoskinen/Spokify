#include "PlaylistContainer.h"
#include "Session.h"

#include <libspotify/api.h>

namespace libspokify {

PlaylistContainer::PlaylistContainer(const Session &session, QObject *parent) :
    QObject(parent),
    m_session(session)
{
}

PlaylistContainer::~PlaylistContainer() {
}

sp_playlistcontainer* PlaylistContainer::native() const {
    return sp_session_playlistcontainer(m_session.session());
}

}
