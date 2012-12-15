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
#include "trackview.h"
#include "trackmodel.h"
#include "coverlabel.h"
#include "mainwidget.h"
#include "soundfeeder.h"
#include "playlistview.h"
#include "playlistmodel.h"
#include "searchhistorymodel.h"
#include "scrobblingsettingsdialog.h"
#include "scrobbler.h"
#include "lyricswidget.h"

#include "libspokify/Session.h"
#include "libspokify/Error.h"
#include "libspokify/Player.h"
#include "libspokify/PlaylistContainer.h"
#include "libspokify/Playlist.h"
#include "libspokify/Search.h"
#include "libspokify/SearchResults.h"
#include "libspokify/Track.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QBuffer>

#include <QtGui/QLabel>
#include <QtGui/QMovie>
#include <QtGui/QListView>
#include <QtGui/QBoxLayout>
#include <QtGui/QDockWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QProgressBar>
#include <QtGui/QSortFilterProxyModel>

#include <KMenu>
#include <KDebug>
#include <KAction>
#include <KLocale>
#include <KLineEdit>
#include <KComboBox>
#include <KAboutData>
#include <KStatusBar>
#include <KPushButton>
#include <KMessageBox>
#include <KApplication>
#include <KNotification>
#include <KStandardDirs>
#include <KStandardAction>
#include <KActionCollection>
#include <KStatusNotifierItem>

MainWindow *MainWindow::s_self = 0;

using namespace libspokify;

//BEGIN: SpotifyPlaylists - application bridge
namespace SpotifyPlaylists {

    static void tracksAdded(sp_playlist *pl, sp_track *const *tracks, int numTracks, int position, void *userdata)
    {
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(position);
        Q_UNUSED(userdata);
        MainWidget::Collection &c = MainWindow::self()->mainWidget()->collection(pl);
        c.needsToBeFilled = true;
    }

    static void tracksRemoved(sp_playlist *pl, const int *tracks, int numTracks, void *userdata)
    {
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(userdata);
        MainWidget::Collection &c = MainWindow::self()->mainWidget()->collection(pl);
        c.needsToBeFilled = true;
    }

    static void tracksMoved(sp_playlist *pl, const int *tracks, int numTracks, int newPosition, void *userdata)
    {
        Q_UNUSED(tracks);
        Q_UNUSED(numTracks);
        Q_UNUSED(newPosition);
        Q_UNUSED(userdata);
        MainWidget::Collection &c = MainWindow::self()->mainWidget()->collection(pl);
        c.needsToBeFilled = true;
    }

    static void playlistRenamed(sp_playlist *pl, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(userdata);
        MainWindow::self()->fillPlaylistModel();
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

#if SPOTIFY_API_VERSION > 4
    static void trackCreatedChanged(sp_playlist *pl, int position, sp_user *user, int when, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(position);
        Q_UNUSED(user);
        Q_UNUSED(when);
        Q_UNUSED(userdata);
    }

    static void trackSeenChanged(sp_playlist *pl, int position, bool seen, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(position);
        Q_UNUSED(seen);
        Q_UNUSED(userdata);
    }

    static void descriptionChanged(sp_playlist *pl, const char *desc, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(desc);
        Q_UNUSED(userdata);
    }

    static void imageChanged(sp_playlist *pl, const byte *image, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(image);
        Q_UNUSED(userdata);
    }
#endif

#if SPOTIFY_API_VERSION >= 10
    static void trackMessageChanged(sp_playlist *pl, int position, const char *message, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(position);
        Q_UNUSED(message);
        Q_UNUSED(userdata);
    }

    static void subscribersChanged(sp_playlist *pl, void *userdata)
    {
        Q_UNUSED(pl);
        Q_UNUSED(userdata);
    }
#endif

    static sp_playlist_callbacks spotifyCallbacks = {
        &SpotifyPlaylists::tracksAdded,
        &SpotifyPlaylists::tracksRemoved,
        &SpotifyPlaylists::tracksMoved,
        &SpotifyPlaylists::playlistRenamed,
        &SpotifyPlaylists::playlistStateChanged,
        &SpotifyPlaylists::playlistUpdateInProgress,
        &SpotifyPlaylists::playlistMetadataUpdated
#if SPOTIFY_API_VERSION > 4
        , &SpotifyPlaylists::trackCreatedChanged,
        &SpotifyPlaylists::trackSeenChanged,
        &SpotifyPlaylists::descriptionChanged,
        &SpotifyPlaylists::imageChanged
#endif
#if SPOTIFY_API_VERSION >= 10
        , &SpotifyPlaylists::trackMessageChanged,
        &SpotifyPlaylists::subscribersChanged
#endif
    };

}
//END: SpotifyPlaylists - application bridge

//BEGIN: SpotifyImage - application bridge
namespace SpotifyImage {

    void imageLoaded(sp_image *image, void *userdata)
    {
        size_t dataSize = 0;
        const void *imageData = sp_image_data(image, &dataSize);
        const QImage cover = QImage::fromData(static_cast<const uchar*>(imageData), dataSize, "JPEG");

        MainWindow::self()->signalCoverLoaded(cover);

        sp_track *const tr = static_cast<sp_track*>(userdata);
        KNotification *notification = new KNotification("nowListening");
        notification->setTitle(i18n("Spokify - Now Listening"));
        notification->setPixmap(QPixmap::fromImage(cover.scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        notification->setText(i18n("Track: %1\nArtist: %2\nAlbum: %3\nPopularity: %4%", 
                              QString::fromUtf8(sp_track_name(tr)),
                              QString::fromUtf8(sp_artist_name(sp_track_artist(tr, 0))),
                              QString::fromUtf8(sp_album_name(sp_track_album(tr))),
                              sp_track_popularity(tr)));
        notification->sendEvent();
    }

}
//END: SpotifyImage - application bridge

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
    , m_soundFeeder(new SoundFeeder(this))
    , m_isExiting(false)
    , m_session(0)
    , m_currentPlaylist(0)
    , m_statusLabel(new QLabel(i18n("Ready"), this))
    , m_progress(new QProgressBar(this))
    , m_notifierItem(new KStatusNotifierItem(i18n("Spokify"), this))
    , m_loggedIn(false)
    , m_mainWidget(new MainWidget(this))
    , m_playlistModel(new PlaylistModel(this))
    , m_searchHistoryModel(new SearchHistoryModel(this))
    , m_playlistView(0)
    , m_searchHistoryView(new QListView(this))
{
    qRegisterMetaType<Chunk>();

    s_self = this;

    m_notifierItem->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_notifierItem->setAssociatedWidget(this);
    m_notifierItem->setToolTip("preferences-desktop-text-to-speech", "Spokify", KGlobal::mainComponent().aboutData()->shortDescription());
    m_notifierItem->setStatus(KStatusNotifierItem::Active);
    m_notifierItem->setIconByName("preferences-desktop-text-to-speech");

    m_scrobbler = new Scrobbler(this);
    connect(this, SIGNAL(nowPlaying(QString,QString,uint)), m_scrobbler, SLOT(setTrack(QString, QString, uint)));
    connect(this, SIGNAL(scrobble()), m_scrobbler, SLOT(scrobble()));
    setupActions();

    {
        KMenu *contextMenu = m_notifierItem->contextMenu();
        contextMenu->addSeparator();
        contextMenu->addAction(m_pause);
        contextMenu->addAction(m_previousTrack);
        contextMenu->addAction(m_nextTrack);
    }

    connect(this, SIGNAL(newChunkReceived(Chunk)), this, SLOT(newChunkReceivedSlot(Chunk)), Qt::QueuedConnection);
    connect(this, SIGNAL(coverLoaded(QImage)), this, SLOT(coverLoadedSlot(QImage)), Qt::QueuedConnection);
    connect(m_soundFeeder, SIGNAL(pcmWritten(Chunk)), this, SLOT(pcmWrittenSlot(Chunk)));
    connect(m_mainWidget, SIGNAL(play(QModelIndex)), this, SLOT(playSlot(QModelIndex)));
    connect(m_mainWidget, SIGNAL(resume()), this, SLOT(resumeSlot()));
    connect(m_mainWidget, SIGNAL(pausedOrStopped()), this, SLOT(pausedOrStoppedSlot()));
    connect(m_mainWidget, SIGNAL(seekPosition(int)), this, SLOT(seekPosition(int)));
    connect(m_mainWidget, SIGNAL(currentTrackFinished()), this, SLOT(nextTrackSlot()));

    setCentralWidget(m_mainWidget);

    //BEGIN: Spotify session init
    {
        const QString settingsPath = QString("%1/.config/spotify").arg(QDir::homePath());

        Session::Config config(QByteArray((const char *)g_appkey, (int)g_appkey_size), "spokify");
        config.setCacheLocation(settingsPath);
        config.setSettingsLocation(settingsPath);

        m_session = new Session(config, this);
        if (!m_session->isInitialized()) {
            KMessageBox::error(this, i18n("Couldn't create Spotify session.\n\nThe error message is \"%1\".\n\nPlease try again. If the problem persists contact the developers.", m_session->initializationError().description()), i18n("A critical error happened"));
            return;
        }

        m_session->player().registerAudioConsumer(this);

        connect(m_session, SIGNAL(loggedIn(libspokify::Error)), this, SLOT(spotifyLoggedIn(libspokify::Error)));
        connect(m_session, SIGNAL(loggedOut()), this, SLOT(spotifyLoggedOut()));
        connect(m_session, SIGNAL(metadataUpdated()), this, SLOT(fillPlaylistModel()));
        connect(m_session, SIGNAL(playTokenLost()), this, SLOT(spotifyPlayTokenLost()));
        connect(m_session, SIGNAL(endOfTrack()), this, SLOT(endOfTrack()));
    }
    //END: Spotify session init

    //BEGIN: set up playlists widget
    {
        m_playlistView = new PlaylistView(this);
        m_playlistView->setAlternatingRowColors(true);
        m_playlistView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_playlistView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_playlistView->setMouseTracking(true);
        m_playlistView->setDropIndicatorShown(true);
        m_playlistView->viewport()->setAcceptDrops(true);
        m_playlistView->setModel(m_playlistModel);
        QDockWidget *playlists = new QDockWidget(i18n("Playlists"), this);
        playlists->setObjectName("playlists");
        playlists->setWidget(m_playlistView);
        addDockWidget(Qt::LeftDockWidgetArea, playlists);
        connect(m_playlistView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(playlistChanged(QItemSelection)));
        connect(m_playlistView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playPlaylist(QModelIndex)));
    }
    //END: set up playlists widget

    //BEGIN: set up search history widget
    {
        m_searchHistoryView->setAlternatingRowColors(true);
        m_searchHistoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_searchHistoryView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_searchHistoryView->setMouseTracking(true);
        m_searchHistoryView->setModel(m_searchHistoryModel);
        QDockWidget *searchHistory = new QDockWidget(i18n("Search History"), this);
        searchHistory->setObjectName("searchhistory");
        searchHistory->setWidget(m_searchHistoryView);
        addDockWidget(Qt::LeftDockWidgetArea, searchHistory);
        connect(m_searchHistoryView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(searchHistoryChanged(QItemSelection)));
        connect(m_searchHistoryView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(playSearchHistory(QModelIndex)));
    }
    //END: set up search history widget

    //BEGIN: set up search widget
    {
        QDockWidget *search = new QDockWidget(i18n("Search"), this);
        search->setObjectName("search");
        search->setWidget(createSearchWidget());
        addDockWidget(Qt::LeftDockWidgetArea, search);
    }
    //END: set up search widget

    //BEGIN: set up cover widget
    {
        QDockWidget *cover = new QDockWidget(i18n("Cover"), this);
        cover->setObjectName("cover");
        cover->setWidget(createCoverWidget());
        addDockWidget(Qt::LeftDockWidgetArea, cover);
    }
    //END: set up cover widget
    
    //BEGIN: set up lyrics widget
    {
        QDockWidget *lyrics = new QDockWidget(i18n("Lyrics"), this);
        lyrics->setObjectName("lyrics");
        m_lyricsWidget = new LyricsWidget(this);
        lyrics->setWidget(m_lyricsWidget);
        addDockWidget(Qt::LeftDockWidgetArea, lyrics);
        connect(this, SIGNAL(nowPlaying(QString,QString,uint)), m_lyricsWidget, SLOT(setTrack(QString, QString)));
    }
    //END: set up cover widget

    m_progress->setMinimum(0);
    m_progress->setMaximum(0);
    m_progress->setVisible(false);

    statusBar()->insertWidget(0, m_statusLabel);
    statusBar()->insertWidget(1, new QWidget(this), 1);
    statusBar()->insertWidget(2, m_progress);

    clearAllWidgets();
    setAutoSaveSettings();

    initSound();
    m_soundFeeder->start();
    
    Login *login = new Login(this);
    login->exec();
}

MainWindow::~MainWindow()
{
    m_isExiting = true;
    m_pcmWaitCondition.wakeAll();

    if (m_loggedIn) {
        m_session->logout();
    }

#if SPOTIFY_API_VERSION > 4
    m_session->destroy();
#endif
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 600);
}

bool MainWindow::isExiting() const
{
    return m_isExiting;
}

MainWindow *MainWindow::self()
{
    return s_self;
}

MainWidget *MainWindow::mainWidget() const
{
    return m_mainWidget;
}

QListView *MainWindow::playlistView() const
{
    return m_playlistView;
}

SearchHistoryModel *MainWindow::searchHistoryModel() const
{
    return m_searchHistoryModel;
}

QListView *MainWindow::searchHistoryView() const
{
    return m_searchHistoryView;
}

bool MainWindow::isPlaying() const
{
    return m_mainWidget->state() == MainWidget::Playing;
}

void MainWindow::spotifyLoggedIn(const Error &error)
{
    if (error.type() == Error::ERROR_OK) {
        showTemporaryMessage(i18n("Logged in"));

        PlaylistContainer *sessionPlaylistContainer = m_session->playlistContainer();
        connect(sessionPlaylistContainer, SIGNAL(containerLoaded()), this, SLOT(fillPlaylistModel()));
        connect(sessionPlaylistContainer, SIGNAL(playlistAdded(sp_playlist*,int)), this, SLOT(fillPlaylistModel()));
        connect(sessionPlaylistContainer, SIGNAL(playlistMoved(sp_playlist*,int,int)), this, SLOT(fillPlaylistModel()));
        connect(sessionPlaylistContainer, SIGNAL(playlistRemoved(sp_playlist*,int)), this, SLOT(fillPlaylistModel()));

        m_loggedIn = true;
        m_login->setVisible(false);
        m_login->setEnabled(true);
        m_logout->setVisible(true);
        m_playlistView->setPlaylistContainer(sessionPlaylistContainer);
        m_playlistView->setEnabled(true);
        m_searchHistoryView->setEnabled(true);
        m_searchCategory->setEnabled(true);
        m_searchField->setEnabled(true);
        m_cover->setEnabled(true);
        m_mainWidget->loggedIn();
        fillPlaylistModel();

        return;
    }

    restoreStatusBarSlot();
    actionCollection()->action("login")->setEnabled(true);

    switch (error.type()) {
    case Error::ERROR_INTERNAL:
        KMessageBox::sorry(this, i18n("An internal error happened with description (%1).\n\nPlease, report this bug.", error.description()), i18n("A critical error happened"));
        break;
    case Error::ERROR_BAD_USERNAME_OR_PASSWORD:
        KMessageBox::sorry(this, i18n("Invalid username or password"), i18n("Invalid username or password"));
        break;
    case Error::ERROR_USER_BANNED:
        KMessageBox::sorry(this, i18n("This user has been banned"), i18n("User banned"));
        break;
    case Error::ERROR_UNABLE_TO_CONTACT_SERVER:
        KMessageBox::sorry(this, i18n("Cannot connect to server"), i18n("Cannot connect to server"));
        break;
    case Error::ERROR_OTHER_PERMANENT:
        KMessageBox::sorry(this, i18n("Something wrong happened.\n\nWhatever it is, it is permanent."), i18n("Something wrong happened"));
        break;
    case Error::ERROR_USER_NEEDS_PREMIUM:
        KMessageBox::sorry(this, i18n("You need to be a Premium User in order to login"), i18n("Premium User access required"));
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Unhandled spotify error:" << error.description();
        break;
    }
}

void MainWindow::spotifyLoggedOut()
{
    if (!m_loggedIn) {
        return;
    }
    showTemporaryMessage(i18n("Logged out"));

    m_loggedIn = false;
    m_login->setVisible(true);
    m_logout->setVisible(false);
    m_logout->setEnabled(true);
}

void MainWindow::spotifyPlayTokenLost()
{
    KMessageBox::sorry(this, i18n("Music is being played with this account at other client"), i18n("Account already being used"));
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

snd_pcm_t *MainWindow::pcmHandle() const
{
    return m_snd;
}

QMutex &MainWindow::pcmMutex()
{
    return m_pcmMutex;
}

QMutex &MainWindow::dataMutex()
{
    return m_dataMutex;
}

QWaitCondition &MainWindow::pcmWaitCondition()
{
    return m_pcmWaitCondition;
}

QWaitCondition &MainWindow::playCondition()
{
    return m_playCondition;
}

unsigned int MainWindow::consumeAudio(const AudioChunk &chunk) {
    if (chunk.NumFrames == 0) {
        return 0;
    }

    const unsigned int numFrames = qMin(chunk.NumFrames, (unsigned int)8192);

    QMutex &m = dataMutex();
    m.lock();
    Chunk c;
    c.m_data = malloc(numFrames * sizeof(int16_t) * chunk.NumChannels);
    memcpy(c.m_data, chunk.Frames, numFrames * sizeof(int16_t) * chunk.NumChannels);
    c.m_dataFrames = numFrames;
    c.m_rate = chunk.SampleRate;
    newChunk(c);
    m.unlock();
    pcmWaitCondition().wakeAll();

    return numFrames;
}

void MainWindow::newChunk(const Chunk &chunk)
{
    m_data.enqueue(chunk);
    emit newChunkReceived(chunk);
}

void MainWindow::coverLoadedSlot(const QImage &cover)
{
    m_coverLoading->stop();
    m_cover->setPixmap(QPixmap::fromImage(cover));
}

Chunk MainWindow::nextChunk()
{
    Chunk chunk = m_data.dequeue();
    m_mainWidget->updateAnalyzer(chunk);
    return chunk;
}

bool MainWindow::hasChunk() const
{
    return !m_data.isEmpty();
}

void MainWindow::endOfTrack()
{
    Chunk c;
    c.m_data = 0;
    c.m_dataFrames = -1;
    c.m_rate = -1;
    m_dataMutex.lock();
    m_data.enqueue(c);
    m_dataMutex.unlock();
    m_mainWidget->advanceCurrentCacheTrackTime(c);
}

void MainWindow::fillPlaylistModel()
{
    QList<Playlist*> playLists = m_session->playlistContainer()->playlists();

    m_playlistModel->removeRows(0, m_playlistModel->rowCount());
    m_playlistModel->insertRows(0, 1 + playLists.count());  // 1 place needed for the starred tracks list

    int currRow = -1;

    // Add the special playlist for starred tracks
    {
        Playlist *pl = m_session->starredPlaylist();
        Q_ASSERT(pl);
        if (pl->native() == m_currentPlaylist) {
            currRow = 0;
        }

        sp_playlist_add_callbacks(pl->native(), &SpotifyPlaylists::spotifyCallbacks, NULL);
        const QModelIndex &index = m_playlistModel->index(0);
        m_playlistModel->setData(index, QChar(0x2605) + i18n("Starred tracks"));
        m_playlistModel->setData(index, QVariant::fromValue<Playlist*>(pl), PlaylistModel::PlaylistRole);
    }
    
    static QList<sp_playlist*> playlistsWithCallbacksSet;

    int i = 1;
    foreach (Playlist *pl, playLists) {
        if (pl->native() == m_currentPlaylist) {
            currRow = i;
        }

        if (!playlistsWithCallbacksSet.contains(pl->native())) {
            playlistsWithCallbacksSet.append(pl->native());
            sp_playlist_add_callbacks(pl->native(), &SpotifyPlaylists::spotifyCallbacks, NULL);
        }

        const QModelIndex &index = m_playlistModel->index(i);
        m_playlistModel->setData(index, pl->name());
        m_playlistModel->setData(index, QVariant::fromValue<Playlist*>(pl), PlaylistModel::PlaylistRole);

        ++i;
    }

    if (currRow != -1) {
        m_playlistView->setCurrentIndex(m_playlistModel->index(currRow, 0));
    }
}

bool MainWindow::shuffleIsOn() const
{
    return m_shuffle->isChecked();
}

bool MainWindow::repeatIsOn() const
{
    return m_repeat->isChecked();
}

void MainWindow::restoreStatusBarSlot()
{
    m_progress->setVisible(false);
    m_statusLabel->setText(i18n("Ready"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    KMessageBox::information(this,
                i18n("<qt>Closing the main window will keep Spokify running in the System Tray. "
                      "Use <B>Quit</B> from the menu, or the Spokify tray icon to exit the application.</qt>"),
                i18n("Docking in System Tray" ), "hideOnCloseInfo" );
    hide();
    event->ignore();
}

void MainWindow::signalCoverLoaded(const QImage &cover)
{
    emit coverLoaded(cover);
}

void MainWindow::newChunkReceivedSlot(const Chunk &chunk)
{
    m_mainWidget->advanceCurrentCacheTrackTime(chunk);
}

void MainWindow::loginSlot()
{
    Login *login = new Login(this);
    if (login->exec() == KDialog::Accepted) {
        showRequest(i18n("Logging in..."));
        m_login->setEnabled(false);
    }
    delete login;
}

void MainWindow::logoutSlot()
{
    showRequest(i18n("Logging out..."));

    clearSoundQueue();
    m_logout->setEnabled(false);

    //BEGIN: Spotify logout
    m_session->logout();
    //END: Spotify logout

    QTimer::singleShot(0, this, SLOT(clearAllWidgets()));
}

void MainWindow::playSlot(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    m_previousTrack->setEnabled(true);
    m_nextTrack->setEnabled(true);
    play(index.data(TrackModel::TrackRole).value<TrackModel::TrackType>());
}

void MainWindow::resumeSlot()
{
    m_previousTrack->setEnabled(true);
    m_nextTrack->setEnabled(true);
    m_mainWidget->setState(MainWidget::Playing);
    m_playCondition.wakeAll();
}

void MainWindow::pausedOrStoppedSlot()
{
    m_previousTrack->setEnabled(false);
    m_nextTrack->setEnabled(false);
}

void MainWindow::performSearch()
{
    showRequest(i18n("Searching..."));
    m_mainWidget->trackView()->setModel(0);
    m_mainWidget->trackView()->setSearching(true);
    m_mainWidget->clearFilter();
    m_currentPlaylist = 0;
    m_playlistView->setCurrentIndex(QModelIndex());

    QString query;
    switch (m_searchCategory->currentIndex()) {
        case 0: // All
            query = m_searchField->text();
            break;
        case 1: // Tracks
            query = QString("track:%1").arg(m_searchField->text());
            break;
        case 2: // Artist
            query = QString("artist:%1").arg(m_searchField->text());
            break;
        case 3: // Album
            query = QString("album:%1").arg(m_searchField->text());
            break;
        case 4: // Year
            query = QString("year:%1").arg(m_searchField->text());
            break;
        case 5: // Record Company
            query = QString("label:%1").arg(m_searchField->text());
            break;
        default:
            Q_ASSERT(false);
            return;
    }

    Search* search = m_session->newSearch(query);
    connect(search, SIGNAL(searchComplete(libspokify::SearchResults*)), this, SLOT(searchResults(libspokify::SearchResults*)));
    search->execute();
}

void MainWindow::searchResults(libspokify::SearchResults *results)
{
    SearchHistoryModel *const searchHistoryModel = MainWindow::self()->searchHistoryModel();
    searchHistoryModel->insertRow(searchHistoryModel->rowCount());
    const QModelIndex index = searchHistoryModel->index(searchHistoryModel->rowCount() - 1, 0);
    searchHistoryModel->setData(index, results->query());
    searchHistoryModel->setData(index, QVariant::fromValue<SearchResults*>(results), SearchHistoryModel::SpotifySearchResultsRole);
    MainWindow::self()->searchHistoryView()->setCurrentIndex(index);

    MainWindow::self()->mainWidget()->trackView()->setSearching(false);
    TrackModel *const trackModel = MainWindow::self()->mainWidget()->collection(results).trackModel;
    trackModel->insertRows(0, results->tracks().count());
    int i = 0;
    foreach (const Track track, results->tracks()) {
        sp_track *const tr = track.native();
        if (!tr || !sp_track_is_loaded(tr)) {
            const QModelIndex &index = trackModel->index(i, TrackModel::Title);
            trackModel->setData(index, i18n("Loading..."));
            continue;
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Title);
            trackModel->setData(index, track.name());
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
            const QModelIndex &index = trackModel->index(i, TrackModel::Duration);
            trackModel->setData(index, track.duration());
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::Popularity);
            trackModel->setData(index, track.popularity());
        }
        {
            const QModelIndex &index = trackModel->index(i, TrackModel::TrackRole);
            trackModel->setData(index, QVariant::fromValue<TrackModel::TrackType>(track), TrackModel::TrackRole);
        }

        ++i;
    }
    MainWindow::self()->showTemporaryMessage(i18n("Search complete"));
}

void MainWindow::pcmWrittenSlot(const Chunk &chunk)
{
    m_mainWidget->advanceCurrentTrackTime(chunk);
}

void MainWindow::playlistChanged(const QItemSelection &selection)
{
    if (selection.isEmpty()) {
        return;
    }

    const QModelIndex index = selection.indexes().first();

    if (!index.isValid()) {
        return;
    }

    m_searchHistoryView->setCurrentIndex(QModelIndex());

    Playlist* curr = index.data(PlaylistModel::PlaylistRole).value<Playlist*>();
    MainWidget::Collection &c = m_mainWidget->collection(curr->native());
    m_currentPlaylist = curr->native();
    if (c.needsToBeFilled) {
        c.needsToBeFilled = false;
        TrackModel *const trackModel = c.trackModel;
        trackModel->removeRows(0, trackModel->rowCount());
        QList<Track> tracks = curr->tracks();
        const int numTracks = tracks.size();
        trackModel->insertRows(0, numTracks);
        for (int i = 0; i < numTracks; ++i) {
            Track track(tracks.at(i));
            sp_track *const tr = track.native();
            
            if (!tr || !sp_track_is_loaded(tr)) {
                const QModelIndex &index = trackModel->index(i, TrackModel::Title);
                trackModel->setData(index, i18n("Loading..."));
                continue;
            }
            {
                const QModelIndex &index = trackModel->index(i, TrackModel::Title);
                trackModel->setData(index, track.name());
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
                const QModelIndex &index = trackModel->index(i, TrackModel::Duration);
                trackModel->setData(index, track.duration());
            }
            {
                const QModelIndex &index = trackModel->index(i, TrackModel::Popularity);
                trackModel->setData(index, track.popularity());
            }
            {
                const QModelIndex &index = trackModel->index(i, TrackModel::TrackRole);
                trackModel->setData(index, QVariant::fromValue<TrackModel::TrackType>(track), TrackModel::TrackRole);
            }
        }
        
        // ugly, yes
        for (int i = 0; i < trackModel->rowCount(); i++) {
            const QModelIndex &index = trackModel->index(i, TrackModel::TrackRole);

#if SPOTIFY_API_VERSION < 10
            if (!sp_track_is_available(m_session->session(), trackModel->data(index).value<TrackModel::TrackType>().native())) {
#else
            if (sp_track_get_availability(m_session->session(), trackModel->data(index).value<TrackModel::TrackType>().native()) != SP_TRACK_AVAILABILITY_AVAILABLE) {
#endif
                trackModel->removeRow(i);
            }
        }
    }
}

void MainWindow::searchHistoryChanged(const QItemSelection &selection)
{
    if (selection.isEmpty()) {
        return;
    }

    const QModelIndex index = selection.indexes().first();

    if (!index.isValid()) {
        return;
    }

    m_playlistView->setCurrentIndex(QModelIndex());

    SearchResults *const curr = index.data(SearchHistoryModel::SpotifySearchResultsRole).value<SearchResults*>();
    m_mainWidget->collection(curr);
    m_currentPlaylist = 0;
}

void MainWindow::seekPosition(int position)
{
    m_pcmMutex.lock();
    snd_pcm_drop(m_snd);
    while (!m_data.isEmpty()) {
        Chunk c = m_data.dequeue();
        free(c.m_data);
    }
    snd_pcm_prepare(m_snd);
    m_pcmMutex.unlock();

    m_session->player().seek(position);
}

void MainWindow::playPlaylist(const QModelIndex &index)
{
    sp_playlist *playlist = index.data(PlaylistModel::PlaylistRole).value<Playlist*>()->native();
    if (!sp_playlist_num_tracks(playlist)) {
        return;
    }
    m_previousTrack->setEnabled(true);
    m_nextTrack->setEnabled(true);
    MainWidget::Collection &c = m_mainWidget->collection(playlist);
    const QModelIndex currentIndex = c.proxyModel->index(0, 0);
    c.currentTrack = currentIndex.data(TrackModel::TrackRole).value<TrackModel::TrackType>();
    m_mainWidget->setCurrentPlayingCollection(c);
    m_mainWidget->trackView()->highlightTrack(c.currentTrack);
    m_mainWidget->setState(MainWidget::Playing);
    play(c.currentTrack);
}

void MainWindow::playSearchHistory(const QModelIndex &index)
{
    SearchResults *search = index.data(SearchHistoryModel::SpotifySearchResultsRole).value<SearchResults*>();
    if (search->tracks().empty()) {
        return;
    }
    m_previousTrack->setEnabled(true);
    m_nextTrack->setEnabled(true);
    MainWidget::Collection &c = m_mainWidget->collection(search);
    const QModelIndex currentIndex = c.proxyModel->index(0, 0);
    c.currentTrack = currentIndex.data(TrackModel::TrackRole).value<TrackModel::TrackType>();
    m_mainWidget->setCurrentPlayingCollection(c);
    m_mainWidget->trackView()->highlightTrack(c.currentTrack);
    m_mainWidget->setState(MainWidget::Playing);
    play(c.currentTrack);
}

void MainWindow::coverClickedSlot()
{
    if (m_mainWidget->currentPlayingCollection()) {
        MainWidget::Collection *const c = m_mainWidget->currentPlayingCollection();
        for (int i = 0; i < m_playlistModel->rowCount(); ++i) {
            const QModelIndex index = m_playlistModel->index(i, 0);
            sp_playlist *const playlist = m_playlistModel->data(index, PlaylistModel::PlaylistRole).value<Playlist*>()->native();
            if (c == &m_mainWidget->collection(playlist)) {
                m_playlistView->setCurrentIndex(index);
                m_searchHistoryView->setCurrentIndex(QModelIndex());
                m_mainWidget->highlightCurrentTrack();
                return;
            }
        }
        for (int i = 0; i < m_searchHistoryModel->rowCount(); ++i) {
            const QModelIndex index = m_searchHistoryModel->index(i, 0);
            SearchResults *const search = m_searchHistoryModel->data(index, SearchHistoryModel::SpotifySearchResultsRole).value<SearchResults*>();
            if (c == &m_mainWidget->collection(search)) {
                m_searchHistoryView->setCurrentIndex(index);
                m_playlistView->setCurrentIndex(QModelIndex());
                return;
            }
        }
    }
}

void MainWindow::clearAllWidgets()
{
    m_playlistModel->removeRows(0, m_playlistModel->rowCount());
    m_playlistView->setEnabled(false);
    m_searchHistoryModel->removeRows(0, m_searchHistoryModel->rowCount());
    m_searchHistoryView->setEnabled(false);
    m_searchCategory->setEnabled(false);
    m_searchCategory->setCurrentIndex(0);
    m_searchField->setEnabled(false);
    m_searchField->setText(QString());
    m_cover->setEnabled(false);
    m_cover->setPixmap(KStandardDirs::locate("appdata", "images/nocover-200x200.png"));
    m_mainWidget->loggedOut();
}

void MainWindow::previousTrackSlot()
{
    MainWidget::Collection *const c = m_mainWidget->currentPlayingCollection();
    if (!c) {
        return;
    }
    int row = c->rowForTrack(c->currentTrack);
    QSortFilterProxyModel *const proxyModel = c->proxyModel;
    if (!proxyModel->rowCount()) {
        return;
    }
    if (row > -1) {
        const QModelIndex index = proxyModel->index(row - 1, 0);
        if (!index.isValid()) {
            return;
        }
        c->currentTrack = index.data(TrackModel::TrackRole).value<TrackModel::TrackType>();
    } else {
        const QModelIndex index = proxyModel->index(0, 0);
        c->currentTrack = index.data(TrackModel::TrackRole).value<TrackModel::TrackType>();
    }
    m_mainWidget->trackView()->highlightTrack(c->currentTrack);
    m_mainWidget->setState(MainWidget::Playing);
    play(c->currentTrack);
}

void MainWindow::setupScrobblingSlot()
{
    ScrobblingSettingsDialog *dialog = new ScrobblingSettingsDialog(this);
    dialog->show();
}

void MainWindow::play(const Track &track)
{
    // Scrobble the currently playing song
    emit scrobble();

    m_previousTrack->setEnabled(true);
    m_nextTrack->setEnabled(true);

    clearSoundQueue();
    m_pcmMutex.lock();
    snd_pcm_prepare(m_snd);
    m_pcmMutex.unlock();
    m_coverLoading->start();
    m_cover->clear();
    m_cover->setMovie(m_coverLoading);

    sp_album *const album = sp_track_album(track.native());
#if SPOTIFY_API_VERSION >= 12
    const byte *image = sp_album_cover(album, SP_IMAGE_SIZE_NORMAL);
#else
    const byte *image = sp_album_cover(album);
#endif
    sp_image *const cover = sp_image_create(m_session->session(), image);
    sp_image_add_load_callback(cover, &SpotifyImage::imageLoaded, track.native());
    m_session->player().play(track);
    m_mainWidget->setTotalTrackTime(track.duration());

    // Set the currently playing song
    QString artist = QString::fromUtf8(sp_artist_name(sp_track_artist(track.native(), 0)));
    emit nowPlaying(artist, track.name(), track.duration());

    m_playCondition.wakeAll();
}

void MainWindow::nextTrackSlot()
{
    m_mainWidget->setState(MainWidget::Stopped);

    MainWidget::Collection *const c = m_mainWidget->currentPlayingCollection();
    if (!c) {
        return;
    }
    QSortFilterProxyModel *const proxyModel = c->proxyModel;
    if (proxyModel->rowCount() == 0) {
        return;
    }

    QModelIndex nextIndex = proxyModel->index(0, 0);

    int row = c->rowForTrack(c->currentTrack);
    if (row > -1) {
        if (!repeatIsOn() && !shuffleIsOn() && row == proxyModel->rowCount() - 1) {
            return;
        }

        int nNewTrackNum = row + 1;
        if (shuffleIsOn()) {
            nNewTrackNum = rand();
        }
        nextIndex = proxyModel->index(nNewTrackNum % proxyModel->rowCount(), 0);
    }

    c->currentTrack = nextIndex.data(TrackModel::TrackRole).value<TrackModel::TrackType>();
    m_mainWidget->trackView()->highlightTrack(c->currentTrack);
    m_mainWidget->setState(MainWidget::Playing);
    play(c->currentTrack);
}

void MainWindow::initSound()
{
    int d = 0;
    snd_pcm_uframes_t periodSize = 1024;
    snd_pcm_uframes_t bufferSize = periodSize * 4;

    snd_pcm_hw_params_t *hwParams;
    snd_pcm_open(&m_snd, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_hw_params_malloc(&hwParams);
    snd_pcm_hw_params_any(m_snd, hwParams);
    snd_pcm_hw_params_set_access(m_snd, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(m_snd, hwParams, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate(m_snd, hwParams, 44100, 0);
    snd_pcm_hw_params_set_channels(m_snd, hwParams, 2);
    snd_pcm_hw_params_set_period_size_near(m_snd, hwParams, &periodSize, &d);
    snd_pcm_hw_params_set_buffer_size_near(m_snd, hwParams, &bufferSize);
    snd_pcm_hw_params(m_snd, hwParams);
    snd_pcm_hw_params_free(hwParams);

    snd_pcm_sw_params_t *swParams;
    snd_pcm_sw_params_malloc(&swParams);
    snd_pcm_sw_params_current(m_snd, swParams);
    snd_pcm_sw_params_set_avail_min(m_snd, swParams, periodSize);
    snd_pcm_sw_params_set_start_threshold(m_snd, swParams, 0);
    snd_pcm_sw_params(m_snd, swParams);
    snd_pcm_sw_params_free(swParams);

    snd_pcm_prepare(m_snd);
}

void MainWindow::clearSoundQueue()
{
    m_dataMutex.lock();
    if (isPlaying()) {
        m_session->player().pause();
        m_session->player().unload();
        m_pcmMutex.lock();
        snd_pcm_drop(m_snd);
        m_pcmMutex.unlock();
        while (!m_data.isEmpty()) {
            Chunk c = m_data.dequeue();
            free(c.m_data);
        }
    }
    m_dataMutex.unlock();
}

QWidget *MainWindow::createSearchWidget()
{
    QWidget *searchWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    {
        m_searchCategory = new KComboBox(this);
        m_searchCategory->addItem(i18n("All"));
        m_searchCategory->addItem(i18n("Tracks"));
        m_searchCategory->addItem(i18n("Artists"));
        m_searchCategory->addItem(i18n("Album"));
        m_searchCategory->addItem(i18n("Year"));
        m_searchCategory->addItem(i18n("Record Company"));
        m_searchField = new KLineEdit(this);
        m_searchField->setClickMessage(i18n("Search"));
        m_searchField->setClearButtonShown(true);
        connect(m_searchField, SIGNAL(returnPressed()), this, SLOT(performSearch()));
        layout->addWidget(m_searchCategory);
        layout->addWidget(m_searchField);
    }
    layout->addStretch();
    searchWidget->setLayout(layout);
    return searchWidget;
}

QWidget *MainWindow::createCoverWidget()
{
    QWidget *coverWidget = new QWidget(this);
    m_cover = new CoverLabel(coverWidget);
    connect(m_cover, SIGNAL(coverClicked()), this, SLOT(coverClickedSlot()));
    m_cover->setPixmap(KStandardDirs::locate("appdata", "images/nocover-200x200.png"));
    m_coverLoading = new QMovie(KStandardDirs::locate("appdata", "images/cover-loading.gif"), QByteArray(), this);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_cover);
    coverWidget->setLayout(layout);
    return coverWidget;
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

    m_shuffle = new KAction(this);
    m_shuffle->setText(i18n("Shu&ffle"));
    m_shuffle->setIcon(KIcon("tools-wizard"));
    m_shuffle->setShortcut(Qt::CTRL + Qt::Key_F);
    m_shuffle->setCheckable(true);
    actionCollection()->addAction("shuffle", m_shuffle);

    m_repeat = new KAction(this);
    m_repeat->setText(i18n("R&epeat"));
    m_repeat->setIcon(KIcon("view-refresh"));
    m_repeat->setShortcut(Qt::CTRL + Qt::Key_E);
    m_repeat->setCheckable(true);
    m_repeat->setChecked(true);
    actionCollection()->addAction("repeat", m_repeat);

    m_setupScrobbling = new KAction(this);
    m_setupScrobbling->setText(i18n("&Set Last.fm credentials..."));
    m_setupScrobbling->setIcon(KIcon("preferences-desktop-user-password"));
    actionCollection()->addAction("setupScrobbling", m_setupScrobbling);
    connect(m_setupScrobbling, SIGNAL(triggered(bool)), this, SLOT(setupScrobblingSlot()));

    m_pause = new KAction(this);
    m_pause->setText(i18n("&Pause"));
    m_pause->setIcon(KIcon("media-playback-pause"));
    actionCollection()->addAction("pause", m_pause);
    connect(m_pause, SIGNAL(triggered(bool)), m_mainWidget, SLOT(togglePlayPauseSlot()));
    m_pause->setShortcut(Qt::Key_Space);
    m_pause->setGlobalShortcut(KShortcut(Qt::Key_MediaTogglePlayPause));

    m_previousTrack = new KAction(this);
    m_previousTrack->setText(i18nc("Play previous track", "Previous"));    
    m_previousTrack->setIcon(KIcon("media-skip-backward"));
    actionCollection()->addAction("previous-track", m_previousTrack);
    connect(m_previousTrack, SIGNAL(triggered()), this, SLOT(previousTrackSlot()));
//    m_previousTrack->setShortcut(Qt::Key_Left | Qt::Key_Control);
    m_previousTrack->setGlobalShortcut(KShortcut(Qt::Key_MediaPrevious));
    m_previousTrack->setEnabled(false);    
    
    m_nextTrack = new KAction(this);
    m_nextTrack->setText(i18nc("Play next track", "Next"));
    m_nextTrack->setIcon(KIcon("media-skip-forward"));
    actionCollection()->addAction("next-track", m_nextTrack);
    connect(m_nextTrack, SIGNAL(triggered()), this, SLOT(nextTrackSlot()));
//    m_nextTrack->setShortcut(Qt::Key_Right | Qt::Key_Control);
    m_nextTrack->setGlobalShortcut(KShortcut(Qt::Key_MediaNext));
    m_nextTrack->setEnabled(false);

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    setupGUI(Default, "spokifyui.rc");
}
