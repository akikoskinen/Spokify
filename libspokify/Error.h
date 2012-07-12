#ifndef LIBSPOKIFY_ERROR_H
#define LIBSPOKIFY_ERROR_H

#include <QString>

namespace libspokify {

class Error {
public:
    enum Type {
        ERROR_OK,
        ERROR_BAD_USERNAME_OR_PASSWORD,
        ERROR_USER_BANNED,
        ERROR_UNABLE_TO_CONTACT_SERVER,
        ERROR_OTHER_PERMANENT,
        ERROR_USER_NEEDS_PREMIUM,
        ERROR_INTERNAL,
        ERROR_UNKNOWN
    };

    Error();

    Error(const Error &other);

    virtual ~Error();

    virtual Type type() const;

    virtual QString description() const;

private:
    Type m_type;
    QString m_description;
};

}

#endif // LIBSPOKIFY_ERROR_H
