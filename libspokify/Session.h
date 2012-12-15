#ifndef LIBSPOKIFY_SESSION_H
#define LIBSPOKIFY_SESSION_H

#include <QObject>
#include <QString>

struct sp_session;

#include "Error.h"
#include "Search.h"

namespace libspokify {

class Playlist;
class PlaylistContainer;
class Player;

class Session : public QObject {
    Q_OBJECT

public:
    class Config {
    public:
        Config(const QByteArray &applicationKey, const QString &userAgent);

        void setCacheLocation(const QString &dir);

        void setSettingsLocation(const QString &dir);

        QByteArray applicationKey() const;
        QString userAgent() const;
        QString cacheLocation() const;
        QString settingsLocation() const;

    private:
        QByteArray m_applicationKey;
        QString m_userAgant;
        QString m_cacheLocation;
        QString m_settingsLocation;
    };


    explicit Session(Config &config, QObject *parent = 0);
    explicit Session(QObject *parent = 0);

    virtual ~Session();

    bool isInitialized() const;

    Error initializationError() const;

    // Currently returns no error
    Error login(const QString &username, const QString &password);

    // Currently returns no error
    Error logout();

    // Currently returns no error
    Error destroy();

    Playlist* starredPlaylist() const;

    PlaylistContainer* playlistContainer() const;

    Player& player();

    /**
     * The caller gains the ownership of the returned object and is responsible
     * for releasing it.
     */
    Search* newSearch(SearchQuery query) const;

    // TODO remove this eventually
    sp_session* session() const;

Q_SIGNALS:
    void loggedIn(const libspokify::Error &error);
    void loggedOut();
    void metadataUpdated();
    void playTokenLost();
    void endOfTrack();

private:
    void connectToSessionMasterSignals();

};

}

#endif // LIBSPOKIFY_SESSION_H
