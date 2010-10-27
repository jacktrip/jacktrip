#ifndef CODEC_H
#define CODEC_H

#include <cstring>

#include "jacktrip_types.h"

// Forward declaration
class AudioInterface;

class Codec
{
public:
    Codec();

    virtual void setup(AudioInterface* audioInterface);
    virtual void stop();
    virtual void encode(int8_t* original, int8_t* compressed);
    virtual void decode(int8_t* compressed, int8_t* uncompressed);
    virtual size_t getTotalCodecSizeInBytes();

protected:
    AudioInterface* audioInterface;

private:
    int totalSizeBytes;
};

#endif // CODEC_H
