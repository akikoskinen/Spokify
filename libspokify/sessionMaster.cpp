#include "sessionMaster.h"

#include <QTimer>
#include <libspotify/api.h>
#include "spokifyplaylist.h"

namespace libspokify {

SessionError::SessionError() :
    m_error(SP_ERROR_OK)
{
}

SessionError::~SessionError() {
}

void SessionError::setError(sp_error error) {
    m_error = error;
}

SessionError::Type SessionError::type() const {
    Error::Type errorType = Error::ERROR_OK;

    switch (m_error) {
    case SP_ERROR_OK:
        errorType = Error::ERROR_OK;
        break;
    case SP_ERROR_BAD_API_VERSION:
    case SP_ERROR_API_INITIALIZATION_FAILED:
#if SPOTIFY_API_VERSION < 9
    case SP_ERROR_RESOURCE_NOT_LOADED:
#endif
    case SP_ERROR_BAD_APPLICATION_KEY:
    case SP_ERROR_CLIENT_TOO_OLD:
    case SP_ERROR_BAD_USER_AGENT:
    case SP_ERROR_MISSING_CALLBACK:
    case SP_ERROR_INVALID_INDATA:
    case SP_ERROR_INDEX_OUT_OF_RANGE:
    case SP_ERROR_OTHER_TRANSIENT:
    case SP_ERROR_IS_LOADING:
        errorType = Error::ERROR_INTERNAL;
        break;
    case SP_ERROR_BAD_USERNAME_OR_PASSWORD:
        errorType = Error::ERROR_BAD_USERNAME_OR_PASSWORD;
        break;
    case SP_ERROR_USER_BANNED:
        errorType = Error::ERROR_USER_BANNED;
        break;
    case SP_ERROR_UNABLE_TO_CONTACT_SERVER:
        errorType = Error::ERROR_UNABLE_TO_CONTACT_SERVER;
        break;
    case SP_ERROR_OTHER_PERMANENT:
        errorType = Error::ERROR_OTHER_PERMANENT;
        break;
    case SP_ERROR_USER_NEEDS_PREMIUM:
        errorType = Error::ERROR_USER_NEEDS_PREMIUM;
        break;
    default:
        errorType = Error::ERROR_UNKNOWN;
        break;
    }

    return errorType;
}

QString SessionError::description() const {
    return QString(sp_error_message(m_error));
}


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
    m_starredPlaylist(NULL),
    m_playlistContainer(this),
    m_player(session, this)
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

Playlist* SessionMaster::starredPlaylist() const {
    if (m_starredPlaylist == 0) {
        sp_playlist *starred = sp_session_starred_create(m_session);
        if (starred != 0) {
            m_starredPlaylist = new SpokifyPlaylist(starred, const_cast<SessionMaster*>(this));
        }
    }

    return m_starredPlaylist;
}

PlaylistContainer& SessionMaster::playlistContainer() {
    if (m_playlistContainer.native() == 0) {
        sp_playlistcontainer *plc = sp_session_playlistcontainer(m_session);
        if (plc != 0) {
            m_playlistContainer.setNative(plc);
        }
    }

    return m_playlistContainer;
}

Player& SessionMaster::player() {
    return m_player;
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


//BEGIN: SpotifySession - application bridge
static void cbLoggedIn(sp_session *session, sp_error err) {
    SessionError error;
    error.setError(err);

    SessionMaster::get(session).notifyLoggedIn(error);
}

static void cbLoggedOut(sp_session *session) {
    SessionMaster::get(session).notifyLoggedOut();
}

static void cbMetadataUpdated(sp_session *session) {
    SessionMaster::get(session).notifyMetadataUpdated();
}

static void cbNotifyMainThread(sp_session *session) {
    SessionMaster::get(session).processSessionEvents();
}

static int cbMusicDelivery(sp_session *session, const sp_audioformat *format, const void *frames, int numFrames_) {
    AudioChunk chunk(format->sample_rate, format->channels, frames, numFrames_);
    return SessionMaster::get(session).player().consumeAudio(chunk);
}

static void cbPlayTokenLost(sp_session *session) {
    SessionMaster::get(session).notifyPlayTokenLost();
}

static void cbEndOfTrack(sp_session *session) {
    SessionMaster::get(session).notifyEndOfTrack();
}
//END: SpotifySession - application bridge


void fillSessionCallbacks(sp_session_callbacks &callbacks) {
    callbacks.logged_in = &cbLoggedIn;
    callbacks.logged_out = &cbLoggedOut;
    callbacks.metadata_updated = &cbMetadataUpdated;
    callbacks.notify_main_thread = &cbNotifyMainThread;
    callbacks.music_delivery = &cbMusicDelivery;
    callbacks.play_token_lost = &cbPlayTokenLost;
    callbacks.end_of_track = &cbEndOfTrack;
}

}
