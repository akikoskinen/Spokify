#include "Session.h"

#include "libspotify/api.h"

#include "Error.h"

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

    virtual QString description() const {
        return QString(sp_error_message(m_error));
    }

private:
    sp_error m_error;
};

static sp_session *m_session = 0;
static SessionError m_initializationError;


Session::Config::Config(const QByteArray &applicationKey, const QString &userAgent, const sp_session_callbacks* callbacks) :
    m_applicationKey(applicationKey),
    m_userAgant(userAgent),
    m_callbacks(callbacks)
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

const sp_session_callbacks* Session::Config::callbacks() const {
    return m_callbacks;
}

QString Session::Config::cacheLocation() const {
    return m_cacheLocation;
}

QString Session::Config::settingsLocation() const {
    return m_settingsLocation;
}


Session::Session(Config &config) {
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
    sessionConfig.callbacks = config.callbacks();

    m_initializationError.setError(sp_session_create(&sessionConfig, &m_session));
}

Session::Session() {
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

sp_session* Session::session() const {
    return m_session;
}

}
