#ifndef SESSIONMASTER_H
#define SESSIONMASTER_H

#include <QObject>
#include <QMap>

struct sp_session;

namespace libspokify {

class Error;

/**
 * A class that handles a single libspotify session. libspotify currently only
 * supports one session but with class it is possible to support multiple
 * sessions if libspotify ever starts to do that.
 */
class SessionMaster : public QObject {
    Q_OBJECT

public:
    virtual ~SessionMaster();

    static SessionMaster* get(sp_session* session);
    static void destroy(sp_session* session);

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

};

}

#endif // SESSIONMASTER_H
