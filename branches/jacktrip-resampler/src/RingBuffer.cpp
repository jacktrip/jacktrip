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
 * \file RingBuffer.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#define SIZEVECTOR 15

#include "RingBuffer.h"
#include <jack/jack.h>
#include "JackTrip.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <stdexcept>



using std::cout; using std::endl;

//*******************************************************************************
RingBuffer::RingBuffer(JackTrip* jackTrip, int size,char type) :
  mJackTrip(jackTrip),
  mTotalSize(size),
  mLastReadSlot(new int8_t[size+ mJackTrip->getAudioBitResolution()/8*mJackTrip->getNumChans()]),
  buffer(0),
  inputSampleInterleaved(new sample_t[(int)ceil((float)mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel()*mJackTrip->getNumChans()) + mJackTrip->getNumChans()]),
  inputSampleInByte(new int8_t[(int)ceil(mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel()*mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8) - mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8]),
  outputSampleDeInterleaved(new sample_t[mJackTrip->getBufferSizeInSamples()*mJackTrip->getNumChans()]),
  outputSampleInterleaved(new sample_t[mJackTrip->getBufferSizeInSamples()*mJackTrip->getNumChans()]),
  tmpForResampler(new char[mJackTrip->getNumChans()*mJackTrip->getPeerBufferSize()*mJackTrip->getAudioBitResolution()/8]),
  tmpByte(new int8_t[mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8]),
  tmpSample(new sample_t(mJackTrip->getNumChans())),
  tmpNRead(0),
  mType(type),
  xrunTime(0),
  xrunTime_prev(0),
  deltaXrun(0),
  arr(0),
  countC(0),
  peerPeriodVector(new QVector<uint32_t>(0)),
  localPeriodVector(new QVector<uint32_t>(0)),
  ratioVector(new QVector<float>(0)),
  arrVector(new QVector<float>(0)),
  totalSizeVector(new QVector<int32_t>(0)),
  readSpaceVector(new QVector<int16_t>(0)),
  writeSpaceVector(new QVector<int16_t>(0)),
  nReadVector(new QVector<int32_t>(0)),
  countCVector(new QVector<int>(0)),
  XRunHappened(0)

{
	buffer = jack_ringbuffer_create( mTotalSize );
	//jack_ringbuffer_reset (buffer);
	jack_ringbuffer_mlock(buffer);
	//size_t tmp = (buffer->write_ptr + mTotalSize/2) & buffer->size_mask;
	//buffer->read_ptr = tmp;
	readPointerAdvance(ceil(mTotalSize/2));
#ifdef __DEBUG__
  info();
#endif
	//exit(1);
}


//*******************************************************************************
RingBuffer::~RingBuffer()
{
  jack_ringbuffer_free( buffer );
  delete[] mLastReadSlot;
  mLastReadSlot = NULL;
}


//*******************************************************************************
//void RingBuffer::insertSlotBlocking( int8_t* ptrToSlot, int dimension)
//{
//  QMutexLocker locker(&mMutex); // lock the mutex
//
//  // Check if there is space available to write a slot
//  // If the Ringbuffer is full, it waits for the bufferIsNotFull condition
//  while (writeSpace() <= 0) {
//    //std::cout << "OUPUT OVERFLOW BLOCKING" << std::endl;
//    mBufferIsNotFull.wait(&mMutex);
//  }
//  write(reinterpret_cast<char*>(ptrToSlot),dimension);
//  mBufferIsNotEmpty.wakeAll();
//}


//*******************************************************************************
void RingBuffer::readSlotBlocking(int8_t* ptrToReadSlot, int dimension)
{
  QMutexLocker locker(&mMutex); // lock the mutex

  // Check if there are slots available to read
  while (readSpace() <= 0) {
    mBufferIsNotEmpty.wait(&mMutex);
  }
#ifdef __DEBUG__
  info();
#endif
  read(reinterpret_cast<char*>(ptrToReadSlot),dimension);
  if(XRunHappened)
    return;
  std::memset(mLastReadSlot, 0, dimension); // set buffer to 0
  std::memcpy(mLastReadSlot, ptrToReadSlot, dimension);
  mBufferIsNotFull.wakeAll();

}


//*******************************************************************************
void RingBuffer::insertSlotNonBlocking(int8_t* ptrToSlot, int dimension)
{
  QMutexLocker locker(&mMutex); // lock the mutex
#ifdef __DEBUG__
  info();
#endif
  write(reinterpret_cast<char*>(ptrToSlot), dimension);
  mBufferIsNotEmpty.wakeAll();
}

//*******************************************************************************
void RingBuffer::insertSlotForResampler(int8_t* ptrToSlot, int dimension)
{
  QMutexLocker locker(&mMutex); // lock the mutex
  int n = 0;
  // Resampler work with interleaved samples.
  //Interleave
    for(int i=0; i<mJackTrip->getNumChans(); i++)
      for(uint16_t j=0; j<mJackTrip->getPeerBufferSize(); j++){
        for(int q=0; q<mJackTrip->getAudioBitResolution()/8; q++){
		  tmpForResampler[i*mJackTrip->getAudioBitResolution()/8 +
		                  mJackTrip->getAudioBitResolution()/8*j*mJackTrip->getNumChans() +
		                  q] = ptrToSlot[(q+j*mJackTrip->getAudioBitResolution()/8) +
		                		            i*(mJackTrip->getPeerBufferSize()*mJackTrip->getAudioBitResolution()/8)];
    	  }
      }
#ifdef __DEBUG__
  info();
#endif
  write(reinterpret_cast<char*>(tmpForResampler), dimension);
  mBufferIsNotEmpty.wakeAll();

}

//*******************************************************************************
void RingBuffer::readSlotNonBlocking(int8_t* ptrToReadSlot, int dimension)
{
  QMutexLocker locker(&mMutex); // lock the mutex
#ifdef __DEBUG__
  info();
#endif
  read(reinterpret_cast<char*>(ptrToReadSlot),dimension);
  if(XRunHappened)
    return;
  std::memset(mLastReadSlot, 0, dimension); // set buffer to 0
  std::memcpy(mLastReadSlot, ptrToReadSlot, dimension);
  mBufferIsNotFull.wakeAll();


}

//*******************************************************************************
int RingBuffer::readSlotAndResampler(int8_t* ptrToReadSlot)
{
  QMutexLocker locker(&mMutex); // lock the mutex
  XRunHappened = 0;
#ifdef __DEBUG__
  info();
  countC = 0;
#endif
  if(mJackTrip->isOnAdaptive() ==true)
	  this->setupAdaptive();
  nRead = 0;
  int frameWorstCase = ceil(mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel()*mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8) -
		  mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8;

  readNoAdvance(reinterpret_cast<char*>(inputSampleInByte), frameWorstCase);
  if(XRunHappened)
    return 1;
  for(int i = 0; i < ceil(mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel()*mJackTrip->getNumChans()) - mJackTrip->getNumChans(); i++)
    mJackTrip->fromBitToSampleConversion(&(inputSampleInByte[i*mJackTrip->getAudioBitResolution()/8]),&inputSampleInterleaved[i], mJackTrip->getAudioBitResolution());


  // Setup the resampler and process the samp
  mJackTrip->getUniformResampler()->inp_count = ceil(mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel()) -  1;
  tmpNRead = mJackTrip->getUniformResampler()->inp_count ;
  mJackTrip->getUniformResampler()->inp_data  = inputSampleInterleaved;
  mJackTrip->getUniformResampler()->out_count = mJackTrip->getBufferSizeInSamples() ;
  mJackTrip->getUniformResampler()->out_data  = outputSampleInterleaved;
  while(mJackTrip->getUniformResampler()->out_count && !XRunHappened){
	  if (mJackTrip->getUniformResampler()->process())
		  emit signalError("Resampler Error!");
	  tmpNRead -= mJackTrip->getUniformResampler()->inp_count ;
	  //cout << tmpNRead << endl;
	  nRead += tmpNRead;
	  if (tmpNRead > mJackTrip->getPeerBufferSize()*mJackTrip->getRatioDel())
		  emit signalError("Resampler Error!");
	  //cout << "nRead "<< nRead << endl;
	  readPointerAdvance(tmpNRead*mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8);
	  if(mJackTrip->getUniformResampler()->out_count == 0)
		  continue;
	  readNoAdvance(reinterpret_cast<char*>(tmpByte), mJackTrip->getNumChans()*mJackTrip->getAudioBitResolution()/8);
	  if(XRunHappened)
	    continue;
	  for(int i = 0; i < mJackTrip->getNumChans(); i++)
	    mJackTrip->fromBitToSampleConversion(&tmpByte[i*mJackTrip->getAudioBitResolution()/8],&tmpSample[i], mJackTrip->getAudioBitResolution());
	  mJackTrip->getUniformResampler()->inp_data  = tmpSample;
	  mJackTrip->getUniformResampler()->inp_count = 1;
	  tmpNRead = mJackTrip->getUniformResampler()->inp_count;
	  ++countC;
  }

  //cout << "nRead"  << nRead << endl;

  //DeInterleave
    for(int i=0; i<mJackTrip->getBufferSizeInSamples(); i++)
      for(uint16_t j=0; j<mJackTrip->getNumChans(); j++){
    	  outputSampleDeInterleaved[j*mJackTrip->getPeerBufferSize()+i]
    	                         = outputSampleInterleaved[j + i*mJackTrip->getNumChans() ];
      }

  // Convert Samples in bytes
  for(int i = 0; i < ((int)mJackTrip->getBufferSizeInSamples())*mJackTrip->getNumChans(); i++)
	  mJackTrip->fromSampleToBitConversion(&outputSampleDeInterleaved[i],&ptrToReadSlot[i*mJackTrip->getAudioBitResolution()/8], mJackTrip->getAudioBitResolution());


  // Always save memory of the last read slot
  std::memset(mLastReadSlot, 0, mJackTrip->getSizeInBytesPerChannel()*mJackTrip->getNumChans()); // set buffer to 0
  std::memcpy(mLastReadSlot, (int8_t*)ptrToReadSlot, mJackTrip->getSizeInBytesPerChannel()*mJackTrip->getNumChans());
  mBufferIsNotFull.wakeAll();

  return 1;
}

int RingBuffer::setupAdaptive(){
  arr = ((mJackTrip->getPeerEstimatedPeriod()*mJackTrip->getRatioDel())/mJackTrip->getEstimatedPeriod());
  mJackTrip->getUniformResampler()->set_rratio(arr);
  return 0;
}

void RingBuffer::info(){
  arr = (mJackTrip->getPeerEstimatedPeriod()*mJackTrip->getRatioDel())/mJackTrip->getEstimatedPeriod();
  if(arrVector->size()>= SIZEVECTOR)
    arrVector->pop_front();
  arrVector->push_back(arr);
  if(readSpaceVector->size()>= SIZEVECTOR)
    readSpaceVector->pop_front();
  readSpaceVector->push_back(readSpace());
  if(writeSpaceVector->size()>= SIZEVECTOR)
    writeSpaceVector->pop_front();
  writeSpaceVector->push_back(writeSpace());
  if(nReadVector->size()>= SIZEVECTOR)
    nReadVector->pop_front();
  nReadVector->push_back(nRead);
  if(countCVector->size()>= SIZEVECTOR)
	  countCVector->pop_front();
  countCVector->push_back(countC);
}


//*******************************************************************************
void RingBuffer::setUnderrunReadSlot(int8_t* ptrToReadSlot)
{
  std::memset(ptrToReadSlot, 0, mJackTrip->getAudioBitResolution()/8*mJackTrip->getBufferSizeInSamples()*mJackTrip->getNumChans());
}


//*******************************************************************************
void RingBuffer::setMemoryInReadSlotWithLastReadSlot(int8_t* ptrToReadSlot)
{
  std::memcpy(ptrToReadSlot,mLastReadSlot,mJackTrip->getAudioBitResolution()/8*mJackTrip->getBufferSizeInSamples()*mJackTrip->getNumChans());
}




//*******************************************************************************
// Under-run happens when there's nothing to read.
void RingBuffer::underrunReset()
{
  xrunTime =  jack_get_time(); 	//jack_frames_to_time (mJackTrip->getClient(), jack_last_frame_time (mJackTrip->getClient()))  ;
  deltaXrun = xrunTime - xrunTime_prev;
  std::cout << mType << ": U: " << xrunTime << " , diff:" << deltaXrun <<std::endl;
#ifdef __DEBUG__
  debugDump();
#endif
  xrunTime_prev =xrunTime;
  //readPointerAdvance(mTotalSize/2);
  //jack_ringbuffer_reset(buffer);
  XRunHappened = 1;
}


//*******************************************************************************
// Over-flow happens when there's no space to write more slots.
void RingBuffer::overflowReset()
{
  xrunTime =  jack_get_time(); //jack_frames_to_time (mJackTrip->getClient(), jack_last_frame_time (mJackTrip->getClient()))  ;
  deltaXrun = xrunTime - xrunTime_prev;
  std::cout << mType << ": O: " << xrunTime << " , diff:" << deltaXrun <<std::endl;
#ifdef __DEBUG__
  debugDump();
#endif
  xrunTime_prev =xrunTime;
  //jack_ringbuffer_reset (buffer);
  readPointerAdvance(mTotalSize/2);
  XRunHappened = 1;
}


//*******************************************************************************
void RingBuffer::debugDump() const
{
  QString strpeerPeriodVector;
  QString strarrVector;
  strarrVector += "arrVector \t\t";
  for (int it = 0; it < arrVector->size(); ++it)
  {
    strarrVector += QString::number(arrVector->value(it));
    strarrVector += "\t";
  }
  QString strreadPositionVector;
  strreadPositionVector += "readSpaceVector \t\t";
  for (int it = 0; it < readSpaceVector->size(); ++it)
  {
    strreadPositionVector += QString::number(readSpaceVector->value(it));
    strreadPositionVector += "\t";
  }
  QString strwritePositionVector;
  strwritePositionVector += "writeSpaceVector \t\t";
  for (int it = 0; it < writeSpaceVector->size(); ++it)
  {
    strwritePositionVector += QString::number(writeSpaceVector->value(it));
    strwritePositionVector += "\t";
  }
  QString strnReadVector;
  strnReadVector += "nReadVector \t\t";
  for (int it = 0; it < nReadVector->size(); ++it)
  {
    strnReadVector += QString::number(nReadVector->value(it));
    strnReadVector += "\t";
  }
  QString strcountCVector;
  strcountCVector += "countCVector \t\t";
  for (int it = 0; it < countCVector->size(); ++it)
  {
    strcountCVector += QString::number(countCVector->value(it));
    strcountCVector += "\t";
  }
  cout << "RingBuffer Type: " << mType << endl;

  cout << strarrVector.toUtf8().constData() << endl;
  cout << strreadPositionVector.toUtf8().constData() << endl;
  cout << strwritePositionVector.toUtf8().constData() << endl;
  cout << strnReadVector.toUtf8().constData() << endl;
  cout << strcountCVector.toUtf8().constData() << endl;

}


int16_t RingBuffer::write(char* data, int16_t dimension)
{
  // get amount of space we can write
  int16_t availableWrite = writeSpace();

  if (availableWrite >= dimension)
  {
    // tell it to write data, keep track of how much was written
	int16_t written = jack_ringbuffer_write( buffer, data , dimension );
    // ensure we wrote an entire event
    if (written != dimension ) {
      setMemoryInReadSlotWithLastReadSlot((int8_t*)data);
      overflowReset();
    }
  }
  else {
	writeNoAdvance( data, dimension);
	writePointerAdvance(dimension);
    overflowReset();
  }
}

int16_t RingBuffer::writeSpace()
{
   // check if there's anything to read
  int availableWrite = jack_ringbuffer_write_space(buffer);
  return availableWrite;
}

void RingBuffer::writePointerAdvance(int16_t dimension)
{
   // check if there's anything to read
	jack_ringbuffer_write_advance(buffer,dimension);
	return;
}


int16_t RingBuffer::read(char* data, int16_t dimension)
{
   // check if there's anything to read
  int16_t availableRead = readSpace();

  if ( availableRead >= dimension )
  {
    // read from the buffer
    int result = jack_ringbuffer_read( buffer, data , dimension );
    if ( result != dimension ) {
      setUnderrunReadSlot((int8_t*)data);
      underrunReset();
      return -1;
    }
  }
  else{
	  jack_ringbuffer_read(buffer, data, availableRead);
	  std::memset(data + availableRead, 0, mJackTrip->getAudioBitResolution()/8*mJackTrip->getBufferSizeInSamples()*mJackTrip->getNumChans() - availableRead);
      underrunReset();
      return -1;
  }

  return 0;
}


int16_t RingBuffer::readNoAdvance(char* data, int16_t dimension)
{
   // check if there's anything to read
  int16_t availableRead = readSpace();

  if ( availableRead >= dimension )
  {
    // create int to read value into
	int16_t tempInt;

    // read from the buffer
	int16_t result = jack_ringbuffer_peek(buffer, data , dimension);

    if ( result != dimension ) {
        setUnderrunReadSlot((int8_t*)data);
        underrunReset();
        return -1;
    }
  }
  else{
      setUnderrunReadSlot((int8_t*)data);
      underrunReset();
      return -1;
  }

  return 0;
}

int16_t RingBuffer::readSpace()
{
   // check if there's anything to read
  int availableRead = jack_ringbuffer_read_space(buffer);
  return availableRead;
}

void RingBuffer::readPointerAdvance(int16_t dimension)
{
   // check if there's anything to read
	jack_ringbuffer_read_advance(buffer,dimension);
	return;
}



int16_t RingBuffer::writeNoAdvance (char *src, size_t cnt)
{
	size_t free_cnt;
	size_t cnt2;
	size_t to_write;
	size_t n1, n2;

	if ((free_cnt = jack_ringbuffer_write_space (buffer)) == 0) {
		return 0;
	}

	to_write = cnt > free_cnt ? free_cnt : cnt;

	cnt2 = buffer->write_ptr + to_write;

	if (cnt2 > buffer->size) {
		n1 = buffer->size - buffer->write_ptr;
		n2 = cnt2 & buffer->size_mask;
	} else {
		n1 = to_write;
		n2 = 0;
	}

	memcpy (&(buffer->buf[buffer->write_ptr]), src, n1);
	volatile size_t tmp_write_ptr = (buffer->write_ptr + n1) & buffer->size_mask;

	if (n2) {
		memcpy (&(buffer->buf[tmp_write_ptr]), src + n1, n2);
	}

	return to_write;
}
