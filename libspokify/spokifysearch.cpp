#include "spokifysearch.h"

#include "spokifytrack.h"
#include "SearchResults.h"

#include <libspotify/api.h>

namespace libspokify {

void searchComplete(sp_search *result, void *userdata) {
    SpokifySearch *search = static_cast<SpokifySearch*>(userdata);
    search->nativeSearchComplete(result);
}

void dummySearchComplete(sp_search *result, void *userdata) {
    SpokifySearch *search = static_cast<SpokifySearch*>(userdata);

    const int res = sp_search_total_tracks(result);
#if SPOTIFY_API_VERSION >= 12
    sp_search_create(search->session(), search->query().toUtf8().data(), 0, res, 0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD, &searchComplete, userdata);
#else
    sp_search_create(search->session(), search->query().toUtf8().data(), 0, res, 0, 0, 0, 0, &searchComplete, userdata);
#endif
}

SpokifySearch::SpokifySearch(sp_session *session, SearchQuery query, QObject *parent) :
    Search(query, parent),
    m_session(session)
{
}

SpokifySearch::~SpokifySearch() {
}

void SpokifySearch::execute() {
#if SPOTIFY_API_VERSION >= 12
    sp_search_create(m_session, m_query.toUtf8().data(), 0, 1, 0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD, &dummySearchComplete, this);
#else
    sp_search_create(m_session, m_query.toUtf8().data(), 0, 1, 0, 0, 0, 0, &dummySearchComplete, this);
#endif
}

sp_session* SpokifySearch::session() const {
    return m_session;
}

void SpokifySearch::nativeSearchComplete(sp_search *result) {
    QList<Track*> tracks;

    for (int i = 0; i < sp_search_num_tracks(result); ++i) {
        sp_track *const tr = sp_search_track(result, i);
        tracks.append(new SpokifyTrack(tr));
    }

    SearchResults *results = new SearchResults(query(), tracks);
    notifySearchComplete(results);
}

}
