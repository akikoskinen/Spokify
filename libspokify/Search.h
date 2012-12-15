#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include "SearchQuery.h"

namespace libspokify {

class SearchResults;

class Search : public QObject {
    Q_OBJECT

public:
    explicit Search(SearchQuery query, QObject *parent = 0);

    virtual ~Search();

    virtual void execute() = 0;

    SearchQuery query() const;

Q_SIGNALS:
    void searchComplete(libspokify::SearchResults *results);

protected:
    const SearchQuery m_query;
    SearchResults *m_latestResults;

    void notifySearchComplete(SearchResults *results);
};

}

#endif // SEARCH_H
