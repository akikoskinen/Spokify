#include "Playlist.h"
#include "playlist_p.h"

#include <libspotify/api.h>

#include "spokifyconstructor.h"

void cbTracksAdded(sp_playlist *pl, sp_track *const *tracks, int numTracks, int position, void *userdata) {
    Q_UNUSED(pl);
    Q_UNUSED(tracks);
    Q_UNUSED(numTracks);
    Q_UNUSED(position);
    Q_UNUSED(userdata);

    static_cast<libspokify::PlaylistPrivate*>(userdata)->notifyTracksAdded();
}

void cbTracksRemoved(sp_playlist *pl, const int *tracks, int numTracks, void *userdata) {
    Q_UNUSED(pl);
    Q_UNUSED(tracks);
    Q_UNUSED(numTracks);
    Q_UNUSED(userdata);

    static_cast<libspokify::PlaylistPrivate*>(userdata)->notifyTracksRemoved();
}

void cbTracksMoved(sp_playlist *pl, const int *tracks, int numTracks, int newPosition, void *userdata) {
    Q_UNUSED(pl);
    Q_UNUSED(tracks);
    Q_UNUSED(numTracks);
    Q_UNUSED(newPosition);
    Q_UNUSED(userdata);

    static_cast<libspokify::PlaylistPrivate*>(userdata)->notifyTracksMoved();
}

void cbPlaylistRenamed(sp_playlist *pl, void *userdata) {
    Q_UNUSED(pl);
    Q_UNUSED(userdata);

    static_cast<libspokify::PlaylistPrivate*>(userdata)->notifyPlaylistRenamed();
}

static sp_playlist_callbacks callbacks;

namespace libspokify {


PlaylistPrivate::PlaylistPrivate(sp_playlist *native, sp_session *session) :
    m_native(native),
    m_session(session)
{
    callbacks.tracks_added = &cbTracksAdded;
    callbacks.tracks_removed = &cbTracksRemoved;
    callbacks.tracks_moved = &cbTracksMoved;
    callbacks.playlist_renamed = &cbPlaylistRenamed;
    sp_playlist_add_callbacks(m_native, &callbacks, this);
}

PlaylistPrivate::~PlaylistPrivate() {
    sp_playlist_remove_callbacks(m_native, &callbacks, this);
}

bool PlaylistPrivate::operator==(const PlaylistPrivate &other) const {
    return m_native == other.m_native;
}

QString PlaylistPrivate::name() const {
    return QString::fromUtf8(sp_playlist_name(m_native));
}

void PlaylistPrivate::rename(QString newName) {
    sp_playlist_rename(m_native, newName.toUtf8().data());
}

QList<Track> PlaylistPrivate::tracks() const {
    ensureTracks();
    return m_tracks;
}

void PlaylistPrivate::addTrack(const Track &track) {
    sp_track* const trackToAdd[] = { track.native() };
    sp_playlist_add_tracks(m_native, trackToAdd, 1, sp_playlist_num_tracks(m_native), m_session);
}

void PlaylistPrivate::notifyTracksAdded() {
    setTracksDirty();

    Q_Q(Playlist);
    emit q->tracksAdded();
}

void PlaylistPrivate::notifyTracksRemoved() {
    setTracksDirty();

    Q_Q(Playlist);
    emit q->tracksRemoved();
}

void PlaylistPrivate::notifyTracksMoved() {
    setTracksDirty();

    Q_Q(Playlist);
    emit q->tracksMoved();
}

void PlaylistPrivate::notifyPlaylistRenamed() {
    Q_Q(Playlist);
    emit q->playlistRenamed();
}

void PlaylistPrivate::setTracksDirty() {
    m_tracks.clear();
}

void PlaylistPrivate::ensureTracks() const {
    if (m_tracks.isEmpty()) {
        const int numTracks = sp_playlist_num_tracks(m_native);
        for (int i = 0; i < numTracks; ++i) {
            m_tracks.append(SpokifyConstructor::newTrack(sp_playlist_track(m_native, i)));
        }
    }
}


Playlist::Playlist(PlaylistPrivate &p, QObject *parent) :
    QObject(parent),
    d_ptr(&p)
{
    Q_D(Playlist);
    d->q_ptr = this;
}

Playlist::~Playlist() {
}

bool Playlist::operator==(const Playlist &other) const {
    Q_D(const Playlist);
    return *d == *other.d_ptr;
}

QString Playlist::name() const {
    Q_D(const Playlist);
    return d->name();
}

void Playlist::rename(QString newName) {
    Q_D(Playlist);
    d->rename(newName);
}

QList<Track> Playlist::tracks() const {
    Q_D(const Playlist);
    return d->tracks();
}

void Playlist::addTrack(const Track &track) {
    Q_D(Playlist);
    d->addTrack(track);
}

sp_playlist* Playlist::native() const {
    Q_D(const Playlist);
    return d->m_native;
}

}
