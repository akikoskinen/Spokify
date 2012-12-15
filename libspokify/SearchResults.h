#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <QList>
#include "SearchQuery.h"
#include "Track.h"

namespace libspokify {

class SearchResults {
public:
    SearchResults(const SearchQuery &query, QList<Track> tracks);

    virtual ~SearchResults();

    SearchQuery query() const;

    QList<Track> tracks() const;

private:
    const SearchQuery m_query;
    QList<Track> m_tracks;
};

}

#endif // SEARCHRESULTS_H
