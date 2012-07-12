#include "sessionMaster.h"

#include <QTimer>
#include <libspotify/api.h>

namespace libspokify {

QMap<sp_session*, SessionMaster*> SessionMaster::SessionMasters;

SessionMaster::SessionMaster(sp_session *session) :
    m_session(session)
{
}

SessionMaster::~SessionMaster() {
}

SessionMaster* SessionMaster::get(sp_session* session) {
    if (!SessionMasters.contains(session)) {
        SessionMasters.insert(session, new SessionMaster(session));
    }

    return SessionMasters[session];
}

void SessionMaster::destroy(sp_session* session) {
    delete SessionMasters.take(session);
}

void SessionMaster::notifyLoggedIn(const Error &error) {
    emit loggedIn(error);
}

void SessionMaster::notifyLoggedOut() {
    emit loggedOut();
}

void SessionMaster::notifyMetadataUpdated() {
    emit metadataUpdated();
}

void SessionMaster::notifyPlayTokenLost() {
    emit playTokenLost();
}

void SessionMaster::notifyEndOfTrack() {
    emit endOfTrack();
}

void SessionMaster::processSessionEvents() {
    QTimer::singleShot(0, this, SLOT(doProcessSessionEvents()));
}

void SessionMaster::doProcessSessionEvents() {
    int timeout;
    do {
        sp_session_process_events(m_session, &timeout);
    } while (timeout == 0);
}

}
