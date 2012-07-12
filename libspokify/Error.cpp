#include "Error.h"

namespace libspokify {

Error::Error() :
    m_type(ERROR_OK)
{
}

Error::Error(const Error &other) :
    m_type(other.type()),
    m_description(other.description())
{
}

Error::~Error() {
}

Error::Type Error::type() const {
    return m_type;
}

QString Error::description() const {
    return m_description;
}

}
