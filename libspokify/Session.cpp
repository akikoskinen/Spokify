#include "Session.h"
#include "sessionmaster.h"

#include "libspotify/api.h"

#include <QMap>

namespace libspokify {

// Currently only one session is supported - as in libspotify also
static sp_session *m_session = 0;
static SessionError m_initializationError;


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
    fillSessionCallbacks(spotifyCallbacks);
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

        m_session = 0;
    }

    return Error();
}

Playlist* Session::starredPlaylist() const {
    return SessionMaster::get(session()).starredPlaylist();
}

PlaylistContainer* Session::playlistContainer() const {
    return SessionMaster::get(session()).playlistContainer();
}

Player& Session::player() {
    return SessionMaster::get(session()).player();
}

sp_session* Session::session() const {
    return m_session;
}

void Session::connectToSessionMasterSignals() {
    if (!isInitialized()) {
        return;
    }

    SessionMaster* sessionMaster = &SessionMaster::get(m_session);

    connect(sessionMaster, SIGNAL(loggedIn(libspokify::Error)), SIGNAL(loggedIn(libspokify::Error)));
    connect(sessionMaster, SIGNAL(loggedOut()), SIGNAL(loggedOut()));
    connect(sessionMaster, SIGNAL(metadataUpdated()), SIGNAL(metadataUpdated()));
    connect(sessionMaster, SIGNAL(playTokenLost()), SIGNAL(playTokenLost()));
    connect(sessionMaster, SIGNAL(endOfTrack()), SIGNAL(endOfTrack()));
}

}
