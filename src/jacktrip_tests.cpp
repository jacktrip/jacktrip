//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

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
 * \file jacktrip_tests.cpp
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#include <QVector>
#include <iostream>

#include "JackTripThread.h"

using std::cout;
using std::endl;

const int num_jacktrips = 5;
const int base_port     = 4464;

void main_tests(int argc, char** argv);
void test_threads_server();
void test_threads_client(const char* peer_address);

void main_tests(int /*argc*/, char** argv)
{
    if (argv[1][0] == 's') {
        test_threads_server();
    } else if (argv[1][0] == 'c') {
        test_threads_client("171.64.197.209");
    }
}

// Test many servers running at the same time
void test_threads_server()
{
    QVector<JackTripThread*> jacktrips;
    jacktrips.resize(num_jacktrips);
    int port_num;
    for (int i = 0; i < num_jacktrips; i++) {
        port_num = base_port + i * 10;
        cout << "Port Number: " << port_num << endl;
        jacktrips[i] = new JackTripThread(JackTrip::SERVER);
        jacktrips[i]->setPort(port_num);
        jacktrips[i]->start(QThread::NormalPriority);
        // sleep(1);
    }
}

// Test many servers running at the same time
void test_threads_client(const char* peer_address)
{
    QVector<JackTripThread*> jacktrips;
    jacktrips.resize(num_jacktrips);
    int port_num;
    for (int i = 0; i < num_jacktrips; i++) {
        port_num = base_port + i * 10;
        cout << "Port Number: " << port_num << endl;
        jacktrips[i] = new JackTripThread(JackTrip::CLIENT);
        jacktrips[i]->setPort(port_num);
        jacktrips[i]->setPeerAddress(peer_address);
        // sleep(1);
        jacktrips[i]->start(QThread::NormalPriority);
        // sleep(1);
    }
}
