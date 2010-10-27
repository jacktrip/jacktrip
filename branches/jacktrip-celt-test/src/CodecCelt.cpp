#include "CodecCelt.h"

#include <iostream>

using std::cout; using std::endl;

CodecCELT::CodecCELT(int bytes) :
bytesPerFrame(bytes)
{
}


void CodecCELT::setup(AudioInterface* audioInterface)
{
    int error = 0;

    this->audioInterface = audioInterface;
    sampleRate = audioInterface->getSampleRate();
    frameSize = audioInterface->getBufferSizeInSamples();
    nChannel = audioInterface->getNumInputChannels();

    mode = celt_mode_create(sampleRate, frameSize, &error);
    if (error) cout << "Error creating CELT mode: "
            << celt_strerror(error) << endl;

    encoder = celt_encoder_create(mode, nChannel, &error);
    if (error) cout << "Error creating CELT encoder: "
            << celt_strerror(error) << endl;

    decoder = celt_decoder_create(mode, nChannel, &error);
    if (error) cout << "Error creating CELT decoder: "
            << celt_strerror(error) << endl;
}

void CodecCELT::stop()
{
    celt_encoder_destroy(encoder);
    celt_decoder_destroy(decoder);
    celt_mode_destroy(mode);
}

void CodecCELT::encode(int8_t* original, int8_t* compressed)
{
    celt_encode_float(encoder, (float *) original, frameSize,
                      (unsigned char *) compressed, bytesPerFrame);
}

void CodecCELT::decode(int8_t* compressed, int8_t* uncompressed)
{
    celt_decode_float(decoder, (unsigned char *) compressed, bytesPerFrame,
                      (float *) uncompressed, frameSize);
}

size_t CodecCELT::getTotalCodecSizeInBytes()
{
    return bytesPerFrame;
}
