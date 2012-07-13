#include "sessionMaster.h"

#include <QTimer>
#include <libspotify/api.h>

namespace libspokify {

SpokifyPlayer::SpokifyPlayer(sp_session* session, QObject *parent) :
    Player(parent),
    m_session(session)
{
}

SpokifyPlayer::~SpokifyPlayer() {
}

void SpokifyPlayer::seek(unsigned int position) {
    sp_session_player_seek(m_session, position);
}

void SpokifyPlayer::load(sp_track *track) {
    sp_session_player_load(m_session, track);
}

void SpokifyPlayer::unload() {
    sp_session_player_unload(m_session);
}

void SpokifyPlayer::play() {
    sp_session_player_play(m_session, true);
}

void SpokifyPlayer::pause() {
    sp_session_player_play(m_session, false);
}

QMap<sp_session*, SessionMaster*> SessionMaster::SessionMasters;

SessionMaster::SessionMaster(sp_session *session) :
    m_session(session),
    m_player(new SpokifyPlayer(session, this))
{
}

SessionMaster::~SessionMaster() {
}

SessionMaster& SessionMaster::get(sp_session* session) {
    if (!SessionMasters.contains(session)) {
        SessionMasters.insert(session, new SessionMaster(session));
    }

    return *SessionMasters[session];
}

void SessionMaster::destroy(sp_session* session) {
    delete SessionMasters.take(session);
}

Player& SessionMaster::player() {
    return *m_player;
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
