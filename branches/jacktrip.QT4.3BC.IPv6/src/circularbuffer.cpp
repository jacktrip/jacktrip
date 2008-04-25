//circularbuffer.cpp 
#include "circularbuffer.h"
#include <iostream>
#include <time.h>
using namespace std;

/**
 * The CircularBuffer provides numChunks of buffering.
 * When created, it has (numChunks / 2) chunks of 0s.
 * This provides tolerance for an offset of numChunks/2
 * between the number of reads and writes.
 *
 * FIXME - It might be an idea to let the open threshold vary
 * (asymmetric tolerance.)  I don't really know...
 *
 * If read is called on an empty CircularBuffer, it is reset
 * to its original state.
 *
 * If write is called on a full CircularBuffer, the readPosition is
 * moved forward (numChunks / 2) and the write proceeds (the
 * oldest numChunks/2 chunks are skipped).
 */

CircularBuffer::CircularBuffer (int chunkSize, int numChunks,
				bool blockOnEmpty, int maxSeq):
writePosition (0),
readPosition (0),
blockOnEmpty (blockOnEmpty),
numBufChunks (numChunks),
numChunksFree (numChunks - 1),
chunkSize (chunkSize),
maxSeq (maxSeq)
{
	cbBuffer = (void *) new char[chunkSize * numChunks];
	seqBuffer = new int[numChunks];
	for (int i = 0; i < numChunks; i++) seqBuffer[i] = -1;
	lastSeq = -1;

	if (cbBuffer == NULL)
		cerr << "CircularBuffer Out Of Memory!" << endl;

	prepareReaderWriter ();
	dataLock = new QSemaphore (1);
	//cout << (*dataLock).available() << " FDF SDFJSDIF JSDFIJ DSIFJSDOI JOSI" << endl;
	secondTry = false;
}

CircularBuffer::CircularBuffer (CircularBuffer * copyMe):
writePosition (0), readPosition (0), isOpen (false)
{
	chunkSize = copyMe->chunkSize;
	numBufChunks = copyMe->numBufChunks;
	numChunksFree = numBufChunks - 1;
	blockOnEmpty = copyMe->blockOnEmpty;

	cbBuffer = (void *) new char[chunkSize * numBufChunks];

	if (cbBuffer == NULL)
		cerr << "CircularBuffer Out Of Memory!" << endl;

	prepareReaderWriter ();
	dataLock = new QSemaphore (1);
}

CircularBuffer::~CircularBuffer ()
{
	delete readSemaphore;
	delete dataLock;
}

void
CircularBuffer::debugDump ()
{
	cerr << "========================================" << endl;
	cerr << "chunkSize=" << chunkSize << endl;
	cerr << "numChunks=" << numBufChunks << endl;
	cerr << "cbBuffer=" << cbBuffer << endl;
	cerr << "readPosition=" << readPosition << endl;
	cerr << "writePosition=" << writePosition << endl << endl;
}

int
CircularBuffer::write (const void *writeChunk)
{
	if (isFull () == false)
	{
		memcpy ((void *) ((char *) cbBuffer +
				  writePosition * chunkSize), writeChunk,
			chunkSize);
		writePosition = ++writePosition % numBufChunks;
	}
	else
	{
		this->overflowReset ();
	}
	if (blockOnEmpty)
	  //(*readSemaphore)--;
	  (*readSemaphore).release();//****JPC qt4 porting******
	return chunkSize;
}

int
CircularBuffer::writeRedundant (const void *writeChunk, int z, int seq)
{
//      if (isFull () == false)
	{
	  //(*dataLock)++;
	(*dataLock).acquire();//****JPC qt4 porting******
		int tmpPosition = writePosition - z;
		if (tmpPosition < 0)
		{
			tmpPosition += numBufChunks; // numChunks;
		}
		int tmpSeq = seqBuffer[tmpPosition];
		int dif = seq - tmpSeq;
		if (dif < -numBufChunks); // -numChunks)
		{
			dif += maxSeq;
		}
		if ((dif > 0) || (tmpSeq == -1))
		{
			seqBuffer[tmpPosition] = seq;
			memcpy ((void *) ((char *) cbBuffer +
					  tmpPosition * chunkSize),
				writeChunk, chunkSize);
		}
//              cout << "seq " << seq  <<"\t";
		if (z == 0)
			writePosition = ++writePosition % numBufChunks;
		//(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
	}
	if (blockOnEmpty) {
	  //(*readSemaphore)--;
	  (*readSemaphore).release();//****JPC qt4 porting******
	}
//      return chunkSize;
	return writePosition;
}

int
CircularBuffer::read (void *readChunk)
{
	if (blockOnEmpty)
	{
	  //(*readSemaphore)++;
	  (*readSemaphore).acquire();//****JPC qt4 porting******


		if (isEmpty () == false)
		{
			memcpy (readChunk,
				(void *) ((char *) cbBuffer +
					  readPosition * chunkSize),
				chunkSize);
			readPosition = ++readPosition % numBufChunks;	// circular increment
		}
		else
		{
			this->underrunReset ();
			memset (readChunk, 0, chunkSize);
		}
	}
	else
	{			// not blockOnEmpty
		if (lastSeq == -1)
			lastSeq = 0;
		//(*dataLock)++;
		(*dataLock).acquire();//****JPC qt4 porting******
		int nextSeq = (lastSeq + 1) % maxSeq;
		int i = 0;
		int lag = -maxSeq;
		int bestReadPosition = readPosition;
		int curReadPosition = readPosition;
		while ((lag != 0) && (i < numBufChunks))
		{
			readPosition = ++readPosition % numBufChunks;
			lastSeq = seqBuffer[readPosition];
			int dif = lastSeq - nextSeq;
			if (dif < -numBufChunks) dif += maxSeq;
			if ((dif == 0) || (dif > lag)) 
			{
				lag = dif; // either it's 0 or find latest entry
				bestReadPosition = readPosition;
			}
			i++;
		}
		if (lag < 0) // don't go backwards in time
		{
				bestReadPosition = curReadPosition;
		}
		if (lag != 0) // skipping or stalling, record position
		{
			lastSeq = seqBuffer[bestReadPosition];			
		}
		memcpy (readChunk,
			(void *) ((char *) cbBuffer +
				  bestReadPosition * chunkSize), chunkSize);
		//(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
	}
	if (blockOnEmpty)
		return chunkSize;
	else
		return readPosition;
}

/** Full if writer is one circular position behind the reader. */
bool CircularBuffer::isFull ()
{
  //(*dataLock)++;
	(*dataLock).acquire();//****JPC qt4 porting******
	// cerr << "(" << writePosition << "," << readPosition << ") ";
	// cerr << ((readPosition + numChunks - writePosition) % numChunks) << endl;
	if (writePosition == (readPosition + numBufChunks - 1) % numBufChunks)
	{
	  //(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
		return true;
	}
	else
	{
	  //(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
		return false;
	}
}


/** Empty if writer is equal to reader */
bool CircularBuffer::isEmpty ()
{
  //(*dataLock)++;
	(*dataLock).acquire();//****JPC qt4 porting******
	if (writePosition == readPosition)
	{
	  //(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
		return true;
	}
	else
	{
	  //(*dataLock)--;
		(*dataLock).release();//****JPC qt4 porting******
		return false;
	}
}

void
CircularBuffer::prepareReaderWriter ()
{
	readSemaphore = new QSemaphore (1);
	//(*readSemaphore)++;
	(*readSemaphore).acquire();//****JPC qt4 porting******
	fillChunksWithZeros (0, numBufChunks / 2 - 1);
}

void
CircularBuffer::fillChunksWithZeros (int from, int to)
{
	if (from < 0 || to >= numBufChunks - 1)
		cerr << "Invalid parameters to CircularBuffer::fillChunksWithZeros!" << endl;

	for (writePosition = from; writePosition < to; writePosition++)
	{
		memset (((char *) cbBuffer + writePosition * chunkSize), 0,
			chunkSize);
		if (blockOnEmpty) {
		  //(*readSemaphore)--;
		  (*readSemaphore).release();//****JPC qt4 porting******
		}
	}
	writePosition = writePosition % numBufChunks;
	readPosition = from;
}

void
CircularBuffer::underrunReset ()
{
	if (blockOnEmpty)
		cerr << "----output- underflow!!" << endl;
	else
		cerr << "-input--- underflow!!" << endl;
	//(*dataLock)++;
	(*dataLock).acquire();//****JPC qt4 porting******
	delete readSemaphore;
	prepareReaderWriter ();
	//(*dataLock)--;
	(*dataLock).release();//****JPC qt4 porting******
}

void
CircularBuffer::overflowReset ()
{
	if (blockOnEmpty)
		cerr << "----output- Overflow" << endl;
	else
		cerr << "-input--- Overflow!" << endl;

	//(*dataLock)++;
	(*dataLock).acquire();//****JPC qt4 porting******
	readPosition = (writePosition + (numBufChunks / 2)) % numBufChunks;
	//(*dataLock)--;
	(*dataLock).release();//****JPC qt4 porting******
}
