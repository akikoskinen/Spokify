#include "Player.h"

namespace libspokify {

AudioChunk::AudioChunk(unsigned int sampleRate, unsigned char numChannels, const void *frames, unsigned int numFrames) :
    SampleRate(sampleRate),
    NumChannels(numChannels),
    Frames(frames),
    NumFrames(numFrames)
{
}

Player::Player(QObject *parent) :
    QObject(parent),
    m_audioConsumer(0)
{
}

Player::~Player() {
}

void Player::registerAudioConsumer(AudioConsumer *consumer) {
    m_audioConsumer = consumer;
}

unsigned int Player::consumeAudio(const AudioChunk &chunk) {
    if (m_audioConsumer != 0) {
        return m_audioConsumer->consumeAudio(chunk);
    }

    return 0;
}

}
