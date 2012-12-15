#ifndef SPOKIFYSEARCH_H
#define SPOKIFYSEARCH_H

#include "Search.h"

struct sp_session;
struct sp_search;

namespace libspokify {

class SpokifySearch : public Search {
    Q_OBJECT

public:
    explicit SpokifySearch(sp_session *session, SearchQuery query, QObject *parent = 0);

    virtual ~SpokifySearch();

    virtual void execute();

    sp_session* session() const;

    void nativeSearchComplete(sp_search *result);

private:
    sp_session *m_session;
};

}

#endif // SPOKIFYSEARCH_H
