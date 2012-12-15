#include "Playlist.h"
#include "playlist_p.h"

#include <libspotify/api.h>

#include "spokifyconstructor.h"

namespace libspokify {

PlaylistPrivate::PlaylistPrivate(sp_playlist *native) :
    m_native(native)
{
    const int numTracks = sp_playlist_num_tracks(m_native);
    for (int i = 0; i < numTracks; ++i) {
        m_tracks.append(SpokifyConstructor::newTrack(sp_playlist_track(m_native, i)));
    }
}

QString PlaylistPrivate::name() const {
    return QString::fromUtf8(sp_playlist_name(m_native));
}

void PlaylistPrivate::rename(QString newName) {
    sp_playlist_rename(m_native, newName.toUtf8().data());
}

QList<Track> PlaylistPrivate::tracks() const {
    return m_tracks;
}


Playlist::Playlist(PlaylistPrivate &p, QObject *parent) :
    QObject(parent),
    d_ptr(&p)
{
}

Playlist::~Playlist() {
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

sp_playlist* Playlist::native() const {
    Q_D(const Playlist);
    return d->m_native;
}

}
