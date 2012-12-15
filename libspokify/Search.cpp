#include "Search.h"
#include "SearchResults.h"

namespace libspokify {

Search::Search(SearchQuery query, QObject *parent) :
    QObject(parent),
    m_query(query),
    m_latestResults(0)
{
}

Search::~Search() {
    delete m_latestResults;
}

SearchQuery Search::query() const {
    return m_query;
}

void Search::notifySearchComplete(SearchResults *results) {
    delete m_latestResults;
    m_latestResults = results;

    emit searchComplete(results);
}

}
