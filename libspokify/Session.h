#ifndef LIBSPOKIFY_SESSION_H
#define LIBSPOKIFY_SESSION_H

#include <QString>

struct sp_session;
struct sp_session_config;

#include "Error.h"

namespace libspokify {

class Session {
public:
    Session(sp_session_config &config);
    Session();

    bool isInitialized() const;

    Error initializationError() const;

    sp_session* session() const;

};

}

#endif // LIBSPOKIFY_SESSION_H
