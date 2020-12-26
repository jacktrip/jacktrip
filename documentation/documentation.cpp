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
 * \file documentation.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */


// Main Page Documentation
//-----------------------------------------------------------------
/**
\mainpage JackTrip Documentation
    
\section intro_sec About JackTrip

JackTrip: A System for High-Quality Audio Network Performance
over the Internet.

JackTrip is a Linux and Mac OS X-based system used for multi-machine network
performance over the Internet. It supports any number of channels
(as many as the computer/network can handle) of bidirectional, high quality,
uncompressed audio signal steaming.

You can use it between any combination of Linux and Mac OS X
(i.e., one end using Linux can connect to the other using Mac OS X).

It is currently being developed and actively tested at CCRMA by the SoundWIRE group.



\section install_sec Installation

Download the latest release:
<a href="http://code.google.com/p/jacktrip/">Download</a>

Please read the documentation inside the packet to install.

\subsection install_subsec_osx Mac OS X Requirements

You'll need: <a href="https://jackaudio.org/downloads/">Jack OS X</a>.
The documentation explains how to install it and set it up, and it's highly recommended.

Jack OS X comes with JackPilot to do the audio routing.
An alternative is <a href="http://qjackctl.sourceforge.net/">qjackctl</a>,
which I find easier to use. You can find the binary here:
<a href="http://www.ardour.org/osx_system_requirements">qjackctl mac binary</a>

If you use Leopard, you won't need to configure the UDP ports manually.

\subsubsection install_linux Linux Requirements

Please read the documentation inside the packet to compile and install in linux.\n
The older version of JackTrip is documented 
<a href="http://ccrma.stanford.edu/groups/soundwire/software/jacktrip/">here</a>.
Please follow those instructions to configure the firewall under 



\section using Using JackTrip

Type jacktrip in a terminal window to display a help list with all the options.
JackTrip uses Jack as its audio server. You have to make sure that the settings in
Jack are the same in the local and remote machine.

There are two parameters that you want to tweak: Frames/Period and Sample Rate.
The Lower the Frames/Period, the lower the latency.
The higher the Sampling Rate, the higher the bandwidth requirements.
You have to make sure these settings match in both machines. 

\image html jack_main_settings.jpg

You also may want to look at the internal buffering
<tt>-q, --queue</tt> parameter in JackTrip. If your connection is very unstable, with a lot of jitter,
you should increase this number at the expense of a higher latency.

The audio bit resolution parameter, <tt>-b, --bitres</tt>, can be use to decrease (or increase)
the bandwidth requirements, at the expense of a lower audio quality.

A basic connection will have one of the nodes as a server:

<tt>jacktrip -s</tt>

And the other as a client

<tt>jacktrip -c [SERVER-IP-NUMBER]</tt>

You'll see a JackTrip client in Jack. Everything you connect into the send ports
will be transmitted to your peer. You'll receive what your peer sends you on the receive ports. 

\image html jack_routing.png

*/


