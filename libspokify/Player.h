#ifndef LIBSPOKIFY_PLAYER_H
#define LIBSPOKIFY_PLAYER_H

#include <QObject>

namespace libspokify {

class AudioChunk {
public:
    AudioChunk(unsigned int sampleRate, unsigned char numChannels, const void *frames, unsigned int numFrames);

    const unsigned int SampleRate;
    const unsigned char NumChannels;

    const void *Frames;
    const unsigned int NumFrames;
};


class AudioConsumer {
public:
    virtual ~AudioConsumer() {}
    virtual unsigned int consumeAudio(const AudioChunk &chunk) = 0;
};

class Track;

class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject *parent = 0);

    virtual ~Player();

    void registerAudioConsumer(AudioConsumer *consumer);
    unsigned int consumeAudio(const AudioChunk &chunk);

    /**
     * Seeks to a position in the current track.
     * @param position position in track in milliseconds.
     */
    virtual void seek(unsigned int position) = 0;

    virtual void unload() = 0;

    virtual void play(const Track &track) = 0;
    virtual void pause() = 0;

private:
    AudioConsumer* m_audioConsumer;

};

}

#endif // LIBSPOKIFY_PLAYER_H
