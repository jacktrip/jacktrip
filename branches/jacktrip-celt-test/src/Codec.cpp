#include "Codec.h"
#include "AudioInterface.h"

Codec::Codec() {}

void Codec::setup(AudioInterface* audioInterface)
{
    this->audioInterface = audioInterface;
    totalSizeBytes = audioInterface->getSizeInBytesPerChannel() * audioInterface->getNumInputChannels();
}

void Codec::stop() {}

void Codec::encode(int8_t* original, int8_t* compressed)
{
    std::memcpy(compressed, original, totalSizeBytes);
}

void Codec::decode(int8_t* compressed, int8_t* uncompressed)
{
    std::memcpy(uncompressed, compressed, totalSizeBytes);
}

size_t Codec::getTotalCodecSizeInBytes()
{
    return totalSizeBytes;
}
