/*
  JackTrip: A Multimachine System for High-Quality Audio 
  Network Performance over the Internet

  Copyright (c) 2008 Chris Chafe, Juan-Pablo Caceres,
  SoundWIRE group at CCRMA.
  
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

/*
 * main.cpp
 */

#include "MainDialog.h"

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// dotted integer from hostname
QString *
IPv4Addr (char *namebuf)
{
  struct hostent *hp;
  unsigned int addr = 0;
  struct sockaddr_in dest;
  memset (&dest, 0, sizeof (dest));
  hp = gethostbyname (namebuf);
  if (!hp)
    addr = inet_addr (namebuf);
  if ((!hp) && (addr == INADDR_NONE))
    {
      perror ("IPv4Addr, unable to resolve.");
      exit (1);
    }
  if (hp != NULL)
    memcpy (&(dest.sin_addr), hp->h_addr, hp->h_length);
  else
    dest.sin_addr.s_addr = addr;
  if (hp)
    dest.sin_family = hp->h_addrtype;
  else
    dest.sin_family = AF_INET;
  QString *s = new QString (inet_ntoa (dest.sin_addr));
  return s;
}

#include <qapplication.h>
//#include <q3textstream.h>
/****************************************************************************
 * MAIN
 ***************************************************************************/

int
main (int argc, char **argv)
{
  //create application object
  QApplication a (argc, argv);
  JackTrip s;
  // Parse the command line.  If the arguments are invalid,
  // usage instructions will be printed.  Streambd will exit.
  if (s.ParseCommandLine (argc, argv) == 0)
    return 0;
  MainDialog m;
  //a.setMainWidget (&m);
  //if (s.args->gui) m.show ();
  m.init (&s);
  a.processEvents ();
  return a.exec ();
}


