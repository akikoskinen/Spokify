#include "Session.h"
#include "sessionMaster.h"

#include "libspotify/api.h"

#include "Error.h"

#include <QMap>

namespace libspokify {

class SessionError : public Error {
public:
    SessionError() : m_error(SP_ERROR_OK)
    {}

    virtual ~SessionError() {
    }

    void setError(sp_error error) {
        m_error = error;
    }

    virtual Type type() const {
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

    virtual QString description() const {
        return QString(sp_error_message(m_error));
    }

private:
    sp_error m_error;
};


// TODO move this to the player
QMap<sp_session*, AudioConsumer*> AudioConsumers;


//BEGIN: SpotifySession - application bridge
static void cbLoggedIn(sp_session *session, sp_error err) {
    SessionError error;
    error.setError(err);

    SessionMaster::get(session)->notifyLoggedIn(error);
}

static void cbLoggedOut(sp_session *session) {
    SessionMaster::get(session)->notifyLoggedOut();
}

static void cbMetadataUpdated(sp_session *session) {
    SessionMaster::get(session)->notifyMetadataUpdated();
}

static void cbNotifyMainThread(sp_session *session) {
    SessionMaster::get(session)->processSessionEvents();
}

static int cbMusicDelivery(sp_session *session, const sp_audioformat *format, const void *frames, int numFrames_) {
    if (AudioConsumers.contains(session)) {
        AudioChunk chunk(format->sample_rate, format->channels, frames, numFrames_);
        return AudioConsumers[session]->consumeAudio(chunk);
    }

    return 0;
}

static void cbPlayTokenLost(sp_session *session) {
    SessionMaster::get(session)->notifyPlayTokenLost();
}

static void cbEndOfTrack(sp_session *session) {
    SessionMaster::get(session)->notifyEndOfTrack();
}
//END: SpotifySession - application bridge


// Currently only one session is supported - as in libspotify also
static sp_session *m_session = 0;
static SessionError m_initializationError;


AudioChunk::AudioChunk(unsigned int sampleRate, unsigned char numChannels, const void *frames, unsigned int numFrames) :
    SampleRate(sampleRate),
    NumChannels(numChannels),
    Frames(frames),
    NumFrames(numFrames)
{
}

Session::Config::Config(const QByteArray &applicationKey, const QString &userAgent) :
    m_applicationKey(applicationKey),
    m_userAgant(userAgent)
{
}

void Session::Config::setCacheLocation(const QString &dir) {
    m_cacheLocation = dir;
}

void Session::Config::setSettingsLocation(const QString &dir) {
    m_settingsLocation = dir;
}

QByteArray Session::Config::applicationKey() const {
    return m_applicationKey;
}

QString Session::Config::userAgent() const {
    return m_userAgant;
}

QString Session::Config::cacheLocation() const {
    return m_cacheLocation;
}

QString Session::Config::settingsLocation() const {
    return m_settingsLocation;
}


Session::Session(Config &config, QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(m_session == 0);

    sp_session_config sessionConfig;
    memset(&sessionConfig, 0, sizeof(sessionConfig));

    sessionConfig.api_version = SPOTIFY_API_VERSION;

    QByteArray cacheLocation(config.cacheLocation().toUtf8());
    sessionConfig.cache_location = cacheLocation.constData();
    QByteArray settingsLocation(config.settingsLocation().toUtf8());
    sessionConfig.settings_location = settingsLocation.constData();

    sessionConfig.application_key = config.applicationKey().constData();
    sessionConfig.application_key_size = config.applicationKey().size();

    QByteArray userAgent(config.userAgent().toUtf8());
    sessionConfig.user_agent = userAgent.constData();

    sp_session_callbacks spotifyCallbacks;
    memset(&spotifyCallbacks, 0, sizeof(spotifyCallbacks));

    spotifyCallbacks.logged_in = &cbLoggedIn;
    spotifyCallbacks.logged_out = &cbLoggedOut;
    spotifyCallbacks.metadata_updated = &cbMetadataUpdated;
    spotifyCallbacks.notify_main_thread = &cbNotifyMainThread;
    spotifyCallbacks.music_delivery = &cbMusicDelivery;
    spotifyCallbacks.play_token_lost = &cbPlayTokenLost;
    spotifyCallbacks.end_of_track = &cbEndOfTrack;

    sessionConfig.callbacks = &spotifyCallbacks;

    m_initializationError.setError(sp_session_create(&sessionConfig, &m_session));

    connectToSessionMasterSignals();
}

Session::Session(QObject *parent) :
    QObject(parent)
{
    connectToSessionMasterSignals();
}

Session::~Session() {
}

bool Session::isInitialized() const {
    return m_session != 0;
}

Error Session::initializationError() const {
    return m_initializationError;
}

Error Session::login(const QString &username, const QString &password) {
    sp_session_login(session(), username.toLatin1(), password.toLatin1(), true
#if SPOTIFY_API_VERSION >= 11
                     , NULL
#endif
                     );

    return Error();
}

Error Session::logout() {
    sp_session_logout(session());

    return Error();
}

Error Session::destroy() {
    if (isInitialized()) {
        sp_session_release(session());

        SessionMaster::destroy(session());
        AudioConsumers.remove(session());

        m_session = 0;
    }

    return Error();
}

void Session::registerAudioConsumer(AudioConsumer *consumer) {
    AudioConsumers[session()] = consumer;
}

sp_session* Session::session() const {
    return m_session;
}

void Session::connectToSessionMasterSignals() {
    if (!isInitialized()) {
        return;
    }

    SessionMaster* sessionMaster = SessionMaster::get(m_session);

    connect(sessionMaster, SIGNAL(loggedIn(libspokify::Error)), SIGNAL(loggedIn(libspokify::Error)));
    connect(sessionMaster, SIGNAL(loggedOut()), SIGNAL(loggedOut()));
    connect(sessionMaster, SIGNAL(metadataUpdated()), SIGNAL(metadataUpdated()));
    connect(sessionMaster, SIGNAL(playTokenLost()), SIGNAL(playTokenLost()));
    connect(sessionMaster, SIGNAL(endOfTrack()), SIGNAL(endOfTrack()));
}

}
