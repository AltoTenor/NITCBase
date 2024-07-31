#include "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

StaticBuffer::StaticBuffer() {

  // initialise all buffers as free
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++ ) {
    metainfo[bufferIndex].free = true;
  }

}

// yet to perform write back
StaticBuffer::~StaticBuffer() {}


int StaticBuffer::getFreeBuffer(int blockNum) {
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // Get free buffer index 
  int allocatedBuffer;
  for (allocatedBuffer = 0; allocatedBuffer < BUFFER_CAPACITY; allocatedBuffer++ ){
    if ( metainfo[allocatedBuffer].free == true ) break;
  }

  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;

  return allocatedBuffer;
}


// Retrieve index of Block stored in buffer already
int StaticBuffer::getBufferNum( int blockNum ){
  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  int bufferIndex;
  for (bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++ ){
    if ( metainfo[bufferIndex].blockNum == blockNum ) break;
  }

  if ( bufferIndex == BUFFER_CAPACITY ) return E_BLOCKNOTINBUFFER;
  else return bufferIndex;
}