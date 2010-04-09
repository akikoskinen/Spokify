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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>

#include "chunk.h"

#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QBuffer>
#include <QtCore/QModelIndex>
#include <QtCore/QWaitCondition>

#include <alsa/asoundlib.h>

#include "appkey.h"
#include <spotify/api.h>

class QMovie;
class QLabel;
class QBuffer;
class QListView;
class QProgressBar;

class KAction;
class KComboBox;
class KLineEdit;
class KPushButton;
class KStatusNotifierItem;

class CoverLabel;
class MainWidget;
class SoundFeeder;
class PlaylistModel;

class MainWindow
    : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    bool isExiting() const;

    sp_session *session() const;

    static MainWindow *self();

    MainWidget *mainWidget() const;

    QListView *playlistView() const;

    void signalNotifyMainThread();

    void signalCoverLoaded(const QImage &cover);

    void setIsPlaying(bool isPlaying);
    bool isPlaying() const;

    void spotifyLoggedIn();

    void spotifyLoggedOut();

    void spotifyPlayTokenLost();

    void showTemporaryMessage(const QString &message);

    void showRequest(const QString &request);

    snd_pcm_t *pcmHandle() const;

    QMutex &pcmMutex();

    QMutex &dataMutex();

    QWaitCondition &pcmWaitCondition();

    QWaitCondition &playCondition();

    void newChunk(const Chunk &chunk);

    Chunk nextChunk();

    bool hasChunk() const;

    void endOfTrack();

    void fillPlaylistModel();

public Q_SLOTS:
    void restoreStatusBarSlot();

Q_SIGNALS:
    void notifyMainThreadSignal();
    void newChunkReceived(const Chunk &chunk);
    void coverLoaded(const QImage &cover);

protected:
    virtual void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void notifyMainThread();
    void newChunkReceivedSlot(const Chunk &chunk);
    void coverLoadedSlot(const QImage &cover);
    void loginSlot();
    void logoutSlot();
    void playSlot(const QModelIndex &index);
    void resumeSlot();
    void shuffleSlot();
    void repeatSlot();
    void performSearch();
    void pcmWrittenSlot(const Chunk &chunk);
    void playListChanged(const QModelIndex &index);
    void seekPosition(int position);
    void currentTrackFinishedSlot();

private:
    void play(sp_track *track);
    void initSound();
    void clearSoundQueue();
    QWidget *createSearchWidget();
    QWidget *createCoverWidget();
    void setupActions();
    void clearAllWidgets();

private:
    snd_pcm_t            *m_snd;
    QMutex                m_pcmMutex;
    QMutex                m_dataMutex;
    QWaitCondition        m_pcmWaitCondition;
    QWaitCondition        m_playCondition;
    QQueue<Chunk>         m_data;
    SoundFeeder          *m_soundFeeder;
    bool                  m_isExiting;

    sp_session_config     m_config;
    sp_session           *m_session;
    sp_playlistcontainer *m_pc;

    sp_playlist          *m_currentPlaylist;

    KAction              *m_login;
    KAction              *m_logout;
    KAction              *m_shuffle;
    KAction              *m_repeat;
    QLabel               *m_statusLabel;
    QProgressBar         *m_progress;
    KStatusNotifierItem  *m_notifierItem;
    bool                  m_loggedIn;
    static MainWindow    *s_self;

    KComboBox            *m_searchCategory;
    KLineEdit            *m_searchField;

    CoverLabel           *m_cover;
    QMovie               *m_coverLoading;

    MainWidget           *m_mainWidget;
    PlaylistModel        *m_playlistModel;
    QListView            *m_playlistView;
};
 
#endif
