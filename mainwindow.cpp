/*
 * This file is part of Spokify.
 * Copyright (C) 2010 Rafael Fernández López <ereslibre@kde.org>
 *
 * Spokify is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Spokify is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Spokify.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "login.h"
#include "trackmodel.h"
#include "mainwidget.h"
#include "playlistmodel.h"

#include <QtCore/QTimer>
#include <QtCore/QBuffer>

#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QDockWidget>
#include <QtGui/QProgressBar>

#include <KDebug>
#include <KAction>
#include <KLocale>
#include <KStatusBar>
#include <KMessageBox>
#include <KApplication>
#include <KStandardDirs>
#include <KSystemTrayIcon>
#include <KStandardAction>
#include <KActionCollection>

MainWindow *MainWindow::s_self = 0;

//BEGIN: SpotifySession - application bridge
namespace SpotifySession {

    static void loggedIn(sp_session *session, sp_error error)
    {
        Q_UNUSED(session);

        if (error == SP_ERROR_OK) {
            MainWindow::self()->spotifyLoggedIn();
            return;
        }
        MainWindow::self()->restoreStatusBarSlot();
        MainWindow::self()->actionCollection()->action("login")->setEnabled(true);
        switch (error) {
            case SP_ERROR_BAD_API_VERSION:
            case SP_ERROR_API_INITIALIZATION_FAILED:
            case SP_ERROR_RESOURCE_NOT_LOADED:
            case SP_ERROR_BAD_APPLICATION_KEY:
            case SP_ERROR_CLIENT_TOO_OLD:
            case SP_ERROR_BAD_USER_AGENT:
            case SP_ERROR_MISSING_CALLBACK:
            case SP_ERROR_INVALID_INDATA:
            case SP_ERROR_INDEX_OUT_OF_RANGE:
            case SP_ERROR_OTHER_TRANSIENT:
            case SP_ERROR_IS_LOADING:
                KMessageBox::sorry(MainWindow::self(), i18n("An internal error happened with error code (%1).\n\nPlease, report this bug.").arg(error),
                                   i18n("A critical error happened"));
                break;
            case SP_ERROR_BAD_USERNAME_OR_PASSWORD:
                KMessageBox::sorry(MainWindow::self(), i18n("Invalid username or password"),
                                   i18n("Invalid username or password"));
                break;
            case SP_ERROR_USER_BANNED:
                KMessageBox::sorry(MainWindow::self(), i18n("This user has been banned"),
                                   i18n("User banned"));
                break;
            case SP_ERROR_UNABLE_TO_CONTACT_SERVER:
                KMessageBox::sorry(MainWindow::self(), i18n("Cannot connect to server"),
                                   i18n("Cannot connect to server"));
                break;
            case SP_ERROR_OTHER_PERMANENT:
                KMessageBox::sorry(MainWindow::self(), i18n("Something wrong happened.\n\nWhatever it is, it is permanent."),
                                   i18n("Something wrong happened"));
                break;
            case SP_ERROR_USER_NEEDS_PREMIUM:
                KMessageBox::sorry(MainWindow::self(), i18n("You need to be a Premium User in order to login"),
                                   i18n("Premium User access required"));
                break;
            default:
                break;
        }
    }

    static void loggedOut(sp_session *session)
    {
        Q_UNUSED(session);

        MainWindow::self()->spotifyLoggedOut();
    }

    static void metadataUpdated(sp_session *session)
    {
        Q_UNUSED(session);
    }

    static void connectionError(sp_session *session, sp_error error)
    {
        Q_UNUSED(session);
        Q_UNUSED(error);
    }

    static void messageToUser(sp_session *session, const char *message)
    {
        Q_UNUSED(session);
        Q_UNUSED(message);
    }

    static void notifyMainThread(sp_session *session)
    {
        Q_UNUSED(session);
    }

    static int musicDelivery(sp_session *session, const sp_audioformat *format, const void *frames,
                             int numFrames)
    {
        Q_UNUSED(session);

        audio_fifo_t *af = MainWindow::self()->audioFifo();
        audio_fifo_data_t *afd;
        size_t s;

        if (numFrames == 0)
            return 0;

        pthread_mutex_lock(&af->mutex);

        if (af->qlen > format->sample_rate) {
            pthread_mutex_unlock(&af->mutex);
            return 0;
        }

        s = numFrames * sizeof(int16_t) * format->channels;

        afd = (audio_fifo_data_t*) malloc(sizeof(audio_fifo_data_t) + s);
        memcpy(afd->samples, frames, s);

        afd->nsamples = numFrames;

        afd->rate = format->sample_rate;
        afd->channels = format->channels;

        TAILQ_INSERT_TAIL(&af->q, afd, link);
        af->qlen += numFrames;

        pthread_cond_signal(&af->cond);
        pthread_mutex_unlock(&af->mutex);

        return numFrames;
    }

    static void playTokenLost(sp_session *session)
    {
        Q_UNUSED(session);
    }

    static void logMessage(sp_session *session, const char *data)
    {
        Q_UNUSED(session);
        Q_UNUSED(data);
    }

    static void endOfTrack(sp_session *session)
    {
        Q_UNUSED(session);
    }

    static sp_session_callbacks spotifyCallbacks = {
        &SpotifySession::loggedIn,
        &SpotifySession::loggedOut,
        &SpotifySession::metadataUpdated,
        &SpotifySession::connectionError,
        &SpotifySession::messageToUser,
        &SpotifySession::notifyMainThread,
        &SpotifySession::musicDelivery,
        &SpotifySession::playTokenLost,
        &SpotifySession::logMessage,
        &SpotifySession::endOfTrack
    };

}
//END: SpotifySession - application bridge

//BEGIN: SpotifyPlaylists - application bridge
namespace SpotifyPlaylists {

    static void tracksAdded(sp_playlist *pl, sp_track *const *tracks, int numTracks, int position, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(position);
        Q_UNUSED(userdata);
    }

    static void tracksRemoved(sp_playlist *pl, const int *tracks, int numTracks, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(userdata);
    }

    static void tracksMoved(sp_playlist *pl, const int *tracks, int numTracks, int newPosition, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(newPosition);
        Q_UNUSED(userdata);
    }

    static void playlistRenamed(sp_playlist *pl, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(userdata);
    }

    static void playlistStateChanged(sp_playlist *pl, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(userdata);
    }

    static void playlistUpdateInProgress(sp_playlist *pl, bool done, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(done);
        Q_UNUSED(userdata);
    }

    static void playlistMetadataUpdated(sp_playlist *pl, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(userdata);
    }

    static sp_playlist_callbacks spotifyCallbacks = {
        &SpotifyPlaylists::tracksAdded,
        &SpotifyPlaylists::tracksRemoved,
        &SpotifyPlaylists::tracksMoved,
        &SpotifyPlaylists::playlistRenamed,
        &SpotifyPlaylists::playlistStateChanged,
        &SpotifyPlaylists::playlistUpdateInProgress,
        &SpotifyPlaylists::playlistMetadataUpdated
    };

}
//END: SpotifyPlaylists - application bridge

//BEGIN: SpotifyPlaylistContainer - application bridge
namespace SpotifyPlaylistContainer {

    static void playlistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
    {
        Q_UNUSED(pc);
        Q_UNUSED(playlist);
        Q_UNUSED(position);
        Q_UNUSED(userdata);
    }

    static void playlistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
    {
        Q_UNUSED(pc);
        Q_UNUSED(playlist);
        Q_UNUSED(position);
        Q_UNUSED(userdata);
    }

    static void playlistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int newPosition, void *userdata)
    {
        Q_UNUSED(pc);
        Q_UNUSED(playlist);
        Q_UNUSED(position);
        Q_UNUSED(newPosition);
        Q_UNUSED(userdata);
    }

    static void containerLoaded(sp_playlistcontainer *pc, void *userdata)
    {
        Q_UNUSED(pc);
        Q_UNUSED(userdata);
    }

    static sp_playlistcontainer_callbacks spotifyCallbacks = {
        &SpotifyPlaylistContainer::playlistAdded,
        &SpotifyPlaylistContainer::playlistRemoved,
        &SpotifyPlaylistContainer::playlistMoved,
        &SpotifyPlaylistContainer::containerLoaded
    };

}
//END: SpotifyPlaylistContainer - application bridge

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_pc(0)
    , m_statusLabel(new QLabel(i18n("Ready"), this))
    , m_progress(new QProgressBar(this))
    , m_trayIcon(new KSystemTrayIcon(this))
    , m_loggedIn(false)
    , m_mainWidget(new MainWidget(this))
    , m_playlistModel(new PlaylistModel(this))
    , m_playlistView(new QListView(this))
{
    s_self = this;

    m_trayIcon->setIcon(KIconLoader::global()->loadIcon("preferences-desktop-text-to-speech", KIconLoader::NoGroup));
    m_trayIcon->setVisible(true);

    connect(m_mainWidget, SIGNAL(trackRequest(QModelIndex)), this, SLOT(trackRequested(QModelIndex)));

    setCentralWidget(m_mainWidget);
    setupActions();

    //BEGIN: Spotify session init
    {
        m_config.api_version = SPOTIFY_API_VERSION;
        m_config.cache_location = "tmp";
        m_config.settings_location = "tmp";
        m_config.application_key = g_appkey;
        m_config.application_key_size = g_appkey_size;
        m_config.user_agent = "spokify";
        m_config.callbacks = &SpotifySession::spotifyCallbacks;

        sp_session_init(&m_config, &m_session);
    }
    //END: Spotify session init

    //BEGIN: set up playlists widget
    {
        m_playlistView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_playlistView->setModel(m_playlistModel);
        QDockWidget *playlists = new QDockWidget(i18n("Playlists"), this);
        playlists->setObjectName("playlists");
        playlists->setWidget(m_playlistView);
        addDockWidget(Qt::LeftDockWidgetArea, playlists);
        connect(m_playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(playListChanged(QModelIndex)));
    }
    //END: set up playlists widget

    m_progress->setMinimum(0);
    m_progress->setMaximum(0);
    m_progress->setVisible(false);

    startTimer(500);
    statusBar()->insertWidget(0, m_statusLabel);
    statusBar()->insertWidget(1, new QWidget(this), 1);
    statusBar()->insertWidget(2, m_progress);

    clearAllWidgets();

    //BEGIN: init sound
    audio_init(&m_audioFifo);
    //END: init sound
}

MainWindow::~MainWindow()
{
    if (m_loggedIn) {
        sp_session_logout(m_session);
    }
}

sp_session *MainWindow::session() const
{
    return m_session;
}

MainWindow *MainWindow::self()
{
    return s_self;
}

void MainWindow::spotifyLoggedIn()
{
    m_loggedIn = true;
    m_login->setVisible(false);
    m_login->setEnabled(true);
    m_logout->setVisible(true);
    showTemporaryMessage(i18n("Logged in"));
    fillPlaylistModel();
}

void MainWindow::spotifyLoggedOut()
{
    if (!m_loggedIn) {
        return;
    }
    m_loggedIn = false;
    m_login->setVisible(true);
    m_logout->setVisible(false);
    m_logout->setEnabled(true);
    showTemporaryMessage(i18n("Logged out"));
    clearAllWidgets();
}

void MainWindow::showTemporaryMessage(const QString &message)
{
    m_progress->setVisible(false);
    m_statusLabel->setText(message);
    QTimer::singleShot(2000, this, SLOT(restoreStatusBarSlot()));
}

void MainWindow::showRequest(const QString &request)
{
    m_progress->setVisible(true);
    m_statusLabel->setText(request);
}

audio_fifo_t *MainWindow::audioFifo()
{
    return &m_audioFifo;
}

void MainWindow::restoreStatusBarSlot()
{
    m_progress->setVisible(false);
    m_statusLabel->setText(i18n("Ready"));
}

bool MainWindow::event(QEvent *event)
{
    //BEGIN: Spotify event processing
    switch (event->type()) {
        case QEvent::Timer: {
            int timeout = -1;
            sp_session_process_events(m_session, &timeout);
            event->accept();
            return true;
        }
        default:
            break;
    }
    //END: Spotify event processing
    return KXmlGuiWindow::event(event);
}

void MainWindow::loginSlot()
{
    Login *login = new Login(this);
    if (login->exec() == KDialog::Accepted) {
        m_login->setEnabled(false);
        showRequest(i18n("Logging in..."));
    }
    delete login;
}

void MainWindow::logoutSlot()
{
    m_logout->setEnabled(false);
    showRequest(i18n("Logging out..."));
    //BEGIN: Spotify logout
    sp_session_logout(m_session);
    //END: Spotify logout
}

void MainWindow::playListChanged(const QModelIndex &index)
{
    TrackModel *trackModel = m_mainWidget->trackModel();
    trackModel->removeRows(0, trackModel->rowCount());

    sp_playlist *const curr = index.data(PlaylistModel::SpotifyNativePlaylist).value<sp_playlist*>();
    const int numTracks = sp_playlist_num_tracks(curr);
    trackModel->insertRows(0, numTracks);
    for (int i = 0; i < numTracks; ++i) {
        sp_track *const tr = sp_playlist_track(curr, i);
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Title);
            trackModel->setData(index, QString::fromUtf8(sp_track_name(tr)));
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Artist);
            sp_artist *const artist = sp_track_artist(tr, 0);
            if (artist) {
                trackModel->setData(index, QString::fromUtf8(sp_artist_name(artist)));
            }
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Album);
            sp_album *const album = sp_track_album(tr);
            if (album) {
                trackModel->setData(index, QString::fromUtf8(sp_album_name(album)));
            }
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Title);
            trackModel->setData(index, QVariant::fromValue<sp_track*>(tr), TrackModel::SpotifyNativeTrack);
        }
    }
}

void MainWindow::trackRequested(const QModelIndex &index)
{
    sp_session_player_unload(m_session);
    audio_fifo_flush(&m_audioFifo);
    sp_track *const tr = index.data(TrackModel::SpotifyNativeTrack).value<sp_track*>();
    sp_session_player_load(m_session, tr);
    sp_session_player_play(m_session, 1);
}

void MainWindow::clearAllWidgets()
{
    m_playlistModel->removeRows(0, m_playlistModel->rowCount());
    m_playlistView->setEnabled(false);
    TrackModel *trackModel = m_mainWidget->trackModel();
    trackModel->removeRows(0, trackModel->rowCount());
}

void MainWindow::setupActions()
{
    m_login = new KAction(this);
    m_login->setText(i18n("&Login"));
    m_login->setIcon(KIcon("user-online"));
    m_login->setShortcut(Qt::CTRL + Qt::Key_L);
    actionCollection()->addAction("login", m_login);
    connect(m_login, SIGNAL(triggered(bool)), this, SLOT(loginSlot()));

    m_logout = new KAction(this);
    m_logout->setVisible(false);
    m_logout->setText(i18n("L&ogout"));
    m_logout->setIcon(KIcon("user-offline"));
    m_logout->setShortcut(Qt::CTRL + Qt::Key_O);
    actionCollection()->addAction("logout", m_logout);
    connect(m_logout, SIGNAL(triggered(bool)), this, SLOT(logoutSlot()));    

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    setupGUI(Default, "spokifyui.rc");
}

void MainWindow::fillPlaylistModel()
{
    if (!m_pc) {
        m_pc = sp_session_playlistcontainer(m_session);
    }
    m_playlistView->setEnabled(true);
    const int numPlaylists = sp_playlistcontainer_num_playlists(m_pc);
    m_playlistModel->removeRows(0, m_playlistModel->rowCount());
    m_playlistModel->insertRows(0, numPlaylists);
    for (int i = 0; i < numPlaylists; ++i) {
        sp_playlist *pl = sp_playlistcontainer_playlist(m_pc, i);
        const QModelIndex &index = m_playlistModel->index(i);
        m_playlistModel->setData(index, QString::fromUtf8(sp_playlist_name(pl)));
        m_playlistModel->setData(index, QVariant::fromValue<sp_playlist*>(pl), PlaylistModel::SpotifyNativePlaylist);
    }
}
