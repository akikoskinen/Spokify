#ifndef SESSIONMASTER_H
#define SESSIONMASTER_H

#include <QObject>
#include <QMap>

#include <libspotify/api.h>

#include "spokifyplaylistcontainer.h"
#include "Player.h"
#include "Error.h"

namespace libspokify {

class SessionError : public Error {
public:
    SessionError();

    virtual ~SessionError();

    void setError(sp_error error);

    virtual Type type() const;

    virtual QString description() const;

private:
    sp_error m_error;
};


class SpokifyPlayer : public Player {
    Q_OBJECT

public:
    explicit SpokifyPlayer(sp_session* session, QObject *parent = 0);

    virtual ~SpokifyPlayer();

    virtual void seek(unsigned int position);

    virtual void load(sp_track *track);
    virtual void unload();

    virtual void play();
    virtual void pause();

private:
    sp_session *m_session;

};

class Playlist;

/**
 * A class that handles a single libspotify session. libspotify currently only
 * supports one session but with this class it is possible to support multiple
 * sessions if libspotify ever starts to do that.
 */
class SessionMaster : public QObject {
    Q_OBJECT

public:
    virtual ~SessionMaster();

    static SessionMaster& get(sp_session* session);
    static void destroy(sp_session* session);

    Playlist* starredPlaylist() const;
    PlaylistContainer* playlistContainer();
    Player& player();

    void notifyLoggedIn(const Error &error);
    void notifyLoggedOut();
    void notifyMetadataUpdated();
    void notifyPlayTokenLost();
    void notifyEndOfTrack();

    void processSessionEvents();

Q_SIGNALS:
    void loggedIn(const libspokify::Error &error);
    void loggedOut();
    void metadataUpdated();
    void playTokenLost();
    void endOfTrack();

private Q_SLOTS:
    void doProcessSessionEvents();

private:
    SessionMaster(sp_session *session);
    sp_session *m_session;

    static QMap<sp_session*, SessionMaster*> SessionMasters;

    SpokifyPlaylistContainer* m_playlistContainer;
    mutable Playlist* m_starredPlaylist;
    SpokifyPlayer m_player;

};

void fillSessionCallbacks(sp_session_callbacks &callbacks);

}

#endif // SESSIONMASTER_H
