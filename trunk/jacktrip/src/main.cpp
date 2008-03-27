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
#include <q3textstream.h>
/****************************************************************************
 * MAIN
 ***************************************************************************/

int
main (int argc, char **argv)
{
	//create application object
	QApplication a (argc, argv);
	StreamBD s;
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


