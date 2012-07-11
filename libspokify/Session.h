#ifndef LIBSPOKIFY_SESSION_H
#define LIBSPOKIFY_SESSION_H

#include <QString>

struct sp_session;
struct sp_session_callbacks;

#include "Error.h"

namespace libspokify {

class Session {
public:
    class Config {
    public:
        Config(const QByteArray &applicationKey, const QString &userAgent, const sp_session_callbacks* callbacks);

        void setCacheLocation(const QString &dir);

        void setSettingsLocation(const QString &dir);

        QByteArray applicationKey() const;
        QString userAgent() const;
        const sp_session_callbacks* callbacks() const;
        QString cacheLocation() const;
        QString settingsLocation() const;

    private:
        QByteArray m_applicationKey;
        QString m_userAgant;
        const sp_session_callbacks* m_callbacks;
        QString m_cacheLocation;
        QString m_settingsLocation;
    };


    Session(Config &config);
    Session();

    bool isInitialized() const;

    Error initializationError() const;

    // Currently returns no error
    Error login(const QString &username, const QString &password);

    // Currently returns no error
    Error logout();

    // Currently returns no error
    Error destroy();

    sp_session* session() const;

};

}

#endif // LIBSPOKIFY_SESSION_H
