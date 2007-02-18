/*! \mainpage StreamBD Documentation
 *
 *
 * Download streamBD <a 
href="http://www-ccrma.stanford.edu/groups/soundwire/software/newstream/newstream1_0b.tar.gz">here</a>.
 * \n
 * View installation and run <a href="INSTRUCTIONS.TXT">instructions</a>.
 * \n
 * If you encounter CommonC++ errors (compile-time errors that begin cc_ )
 * Download CommonC++1.5.0 (http://cplusplus.sourceforge.net) from here:
 * <a 
href="http://www-ccrma.stanford.edu/groups/soundwire/software/newstream/commomc++-1.5.0.tar>http://www-ccrma.stanford.edu/groups/soundwire/software/newstream/commonc++-1.5.0.tar</a>.
 * \n
 *\section intro Introduction
 *
 * StreamBD provides low-latency uncompressed audio streaming over high quality networks.
 * It can be used to route audio streams across a high quality network, or to use the network
 * connection as the delay line in a simple string synthesis to "pluck" the network, and generate
 * a multi-channel network harp as described in our ICASSP2002 <a href="http://www-ccrma.stanford.edu/%7Ecc/soundwire/icassp02.pdf">paper</a>.
 *
 * As an audio stream router it consists of the following four nodes:
 * \n\n
 *	   audioin, audioout, netin, netout.
 * \n\n
 * Streambd can be run in any of the following modes to achieve any
 * possible connection of input to output.
 * \n\n
 * <B>Transmit</B>\n
 *              Run with a remote hostname, this mode establishes a 
 *		connection to a remote host running in RECEIVE mode.  It
 * 		patches audio in exactly the same way RECEIVE does:
 *		audioin to netout, netin to audioout.  Use one machine in
 *		TRANSMIT mode and one machine in RECEIVE mode to establish
 * 		a bidirectional connection.
 * \n\n
 * <B>Receive</B>\n
 *              Listen for a connection from another machine running in
 *		TRANSMIT mode.  When contacted, patch audioin to netout and
 *		netin to audioout.  Use one machine in RECEIVE mode and one
 * 		machine in TRANSMIT mode to establish a bidirectional
 *		connection.
 * \n\n
 * <B>Netmirror</B>\n
 *              Patch netin to netout.  Do not create audioin or audioout.
 *		(No audio interface is needed.) The input stream from the
 *		network is streamed straight to the network output.  When run
 *		the program listens for a connection.  When contacted by 
 * 		another machine running in TRANSMIT mode, it establishes a
 * 		return connection.  Use this mode to test your connection to 
 *		a remote machine.
 * \n\n
 * <B>Sndmirror</B> \n
 *              Patch audioin to audioout.  Do not create netin or netout.
 *		(No network interface is needed.) All sound input is streamed
 *		straight to the sound output.  Use this mode to test audio
 *		hardware.
 * \n\n
 * <B>HarpT</B>\n
 *              Run with a hostname, this mode connects to a HARPR receiver
 *              machine, and adds STK low pass filtering or delay (to
 *              "tune" the different strings of the Netharp).  Coefficients
 *              for the STK processes are specified at the command line (off
 *              by default).
 * \n\n
 * <B>HarpR</B>\n
 *              Connected to by a HARPT.  Can also apply processes so that
 *              synthesis CPU requirements can be shared between the
 *              two machines (low-pass filtering on one, delay on the other.)
 * \n\n
 * Questions or comments: Daniel Walling (<a href="mailto:dwalling@ccrma.stanford.edu">dwalling@ccrma.stanford.edu</a>).
 */




/**********************************************************************
 * File: streambd.h
 **********************************************************************
 * A project of the SoundWIRE research group at CCRMA
 * http://www-ccrma.stanford.edu/groups/soundwire/
 * --------------------------------------------------------------------
 * Coded by Daniel Walling (dwalling@ccrma.stanford.edu)
 * Based on Scott Wilson's streambd code (rswilson@ccrma.stanford.edu)
 * --------------------------------------------------------------------
 */
 
#ifndef _STREAM_BD_H
#define _STREAM_BD_H




#endif
