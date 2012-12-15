#include "SearchResults.h"

namespace libspokify {

SearchResults::SearchResults(const SearchQuery &query, QList<Track*> tracks) :
    m_query(query),
    m_tracks(tracks)
{
}

SearchResults::~SearchResults() {
}

SearchQuery SearchResults::query() const {
    return m_query;
}

QList<Track*> SearchResults::tracks() const {
    return m_tracks;
}

}
