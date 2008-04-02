//circularbuffer.h
#ifndef INCLUDED_CIRCULARBUFFER
#define INCLUDED_CIRCULARBUFFER

#include <qthread.h>
//#include <qsemaphore.h>//***JPC Port to qt4*****************
#include <QSemaphore>
/**
 * @brief Provides a circular buffer that can be written to and read
 * from asynchronously.
 *
 * The CircularBuffer is an array of \b numChunks chunks of memory,
 * each of which is of size \b chunkSize bytes.  Chunks can be read
 * and written asynchronously by multiple threads.
 *
 * If \b block is true, calling read and write will block until
 * the requested operation can be legally executed.
 *
 * If \b block is false, calling read on an empty buffer, will
 * return a buffer of 0s and print a buffer underrun error message.
 * Calling write on a full buffer will clear the entire CircularBuffer,
 * and print a buffer overrun error message.
 */

class CircularBuffer {
    int          writePosition;   /**< Pointer to the position to which to write the
                                   * next audio buffer. */
    int          readPosition;    /**< Pointer to the position of the next audio
                                   * buffer to be read. */
    bool         blockOnEmpty;    /**< If read is called on an empty buffer, will it block
                                     * until it has a chunk to return, or will it return a
                                     * chunk of zeros? */
    int          numBufChunks;       /**< Capacity of the CircularBuffer. */
    int          numChunksFree;   /**< Number of chunks that could be written before the
                                   * CircularBuffer would be overflow. */
    int          chunkSize;       /**< The size of one chunk. */
    void        *cbBuffer;        /**< Pointer to the circular buffer memory. */
 int *seqBuffer;
	private:   
 //   Semaphore   *writeSemaphore;  /**< Locks write access. */
    QSemaphore  *readSemaphore;   /**< Locks read access. */
    QSemaphore  *dataLock;        /**< Locks access to state variables. */
public:
 bool         isOpen;          /**< Is the CircularBuffer ready to be read/written? */
     
    bool isEmpty();
    bool isFull();
    void prepareReaderWriter();
    void fillChunksWithZeros(int from, int to);
    void underrunReset();
    void overflowReset();
    
    
    bool         aborted;
    
  public:

    CircularBuffer(int chunkSize,int numChunks,bool blockOnEmpty, int maxSeq);
    CircularBuffer(CircularBuffer *copyMe);
    
    ~CircularBuffer();

        /** @brief Write a chunk into the circularbuffer. */
    int write(const void* writeChunk);
int
writeRedundant (const void *writeChunk, int z, int seq);
  int lastSeq;
  int maxSeq;
  bool secondTry;
        /** @brief Read a chunk from the circularbuffer. */
    int read(void* readChunk);

        /** @brief Abort current read/write. */
    void abortReadOrWrite();

    void debugDump();
        
    bool isAborted() { return  aborted; }

    void reset();
};

#endif
