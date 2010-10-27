#ifndef CODECCELT_H
#define CODECCELT_H

#include "Codec.h"
#include "AudioInterface.h"
#include "celt/celt.h"

class CodecCELT : public Codec
{
public:
    CodecCELT(int bytes);

    virtual void setup(AudioInterface* audioInterface);
    virtual void stop();
    virtual void encode(int8_t* original, int8_t* compressed);
    virtual void decode(int8_t* compressed, int8_t* uncompressed);
    virtual size_t getTotalCodecSizeInBytes();

private:
    int frameSize;
    celt_int32 sampleRate;
    int bytesPerFrame;
    int nChannel;

    CELTMode *mode;
    CELTEncoder *encoder;
    CELTDecoder *decoder;
};

#endif // CODECCELT_H
