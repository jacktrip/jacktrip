//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 JackTrip Labs, Inc.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file audio_socket_tests.cpp
 * \author Mike Dickey
 * \date December 2024
 * \license MIT
 */

#include <iostream>
#include <QCoreApplication>

#include "AudioSocket.h"
#include "jacktrip_globals.h"

using std::cout;
using std::cerr;
using std::endl;

const int SAMPLE_RATE = 48000;
const int BUFFER_SIZE = 128;
const int NUM_CHANNELS = 2;

class MyThread : public QThread
{
public:
    MyThread(AudioSocket& socket) : s(socket) {}
    virtual ~MyThread() {}
    void run() override {
        float **inputs = new float*[NUM_CHANNELS];
        float **outputs = new float*[NUM_CHANNELS];
        for (int i = 0; i < NUM_CHANNELS; i++) {
            inputs[i] = new float[BUFFER_SIZE];
            outputs[i] = new float[BUFFER_SIZE];
            for (int j = 0; j < BUFFER_SIZE; j++) {
                inputs[i][j] = j;
            }
        }

        setRealtimeProcessPriority();

        do {
            s.compute(BUFFER_SIZE, inputs, outputs);
            QThread::usleep(BUFFER_SIZE * 1000000 / SAMPLE_RATE);
        } while (isRunning());

        cout << "Exiting" << endl;
    }

private:
    AudioSocket& s;
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    AudioSocket s;
    if (!s.connect(SAMPLE_RATE, BUFFER_SIZE)) {
        cerr << "Failed to connect: " << s.getSocket().errorString().toStdString() << endl;
        return -1;
    }
    s.setRetryConnection(true);

    MyThread thread(s);
    QObject::connect(&thread, &QThread::finished, &app, &QCoreApplication::quit);
    thread.start();

    return app.exec();
}
