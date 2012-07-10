#include "Error.h"

namespace libspokify {

Error::Error() {
}

Error::Error(const Error &other) {
    m_description = other.description();
}

Error::~Error() {
}

QString Error::description() const {
    return m_description;
}

}
