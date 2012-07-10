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

Session::Session(sp_session_config &config) {
    Q_ASSERT(m_session == 0);

    m_initializationError.setError(sp_session_create(&config, &m_session));
}

Session::Session() {
}

bool Session::isInitialized() const {
    return m_session != 0;
}

Error Session::initializationError() const {
    return m_initializationError;
}

sp_session* Session::session() const {
    return m_session;
}

}
