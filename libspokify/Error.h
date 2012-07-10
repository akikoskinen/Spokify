#ifndef LIBSPOKIFY_ERROR_H
#define LIBSPOKIFY_ERROR_H

#include <QString>

namespace libspokify {

class Error {
public:
    Error();

    Error(const Error &other);

    virtual ~Error();

    virtual QString description() const;

private:
    QString m_description;
};

}

#endif // LIBSPOKIFY_ERROR_H
