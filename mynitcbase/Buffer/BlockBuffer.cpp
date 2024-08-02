#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>



BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
  
  // Use loadBlockAndGetBufferPtr to retrieve pointer to buffer
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // populate the numEntries, numAttrs and numSlots fields in *head
  memset(head, 0, sizeof(struct HeadInfo));

  memcpy(&head->blockType,  bufferPtr + 0,  4);
  memcpy(&head->pblock,     bufferPtr + 4,  4);
  memcpy(&head->lblock,     bufferPtr + 8,  4);
  memcpy(&head->rblock,     bufferPtr + 12, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs,   bufferPtr + 20, 4);
  memcpy(&head->numSlots,   bufferPtr + 24, 4);
  memcpy(&head->reserved,   bufferPtr + 28, 4);
  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  
  struct HeadInfo head;
  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // Use loadBlockAndGetBufferPtr to retrieve pointer to buffer
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr + HEADER_SIZE + slotCount + (recordSize * slotNum);

  // load the record into the rec data structure - Done
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;
}


int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int retval = loadBlockAndGetBufferPtr(&bufferPtr);
    if ( retval != SUCCESS ) {
      return retval;
    }

    HeadInfo head;
    BlockBuffer::getHeader(&head);

    int numOfAttr = head.numAttrs;
    int numOfSlots = head.numSlots;

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    if ( slotNum < 0 || slotNum >= numOfSlots ) return E_OUTOFBOUND;

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    int recordSize = ATTR_SIZE * numOfAttr;
    memcpy(bufferPtr + HEADER_SIZE + numOfSlots + (slotNum*recordSize), rec, recordSize );
    // update dirty bit using setDirtyBit()

    int response = StaticBuffer::setDirtyBit(this->blockNum);

    if (response != SUCCESS){
      printf("Error in setting dirty bit.\n");
      exit(1);
    }

    return SUCCESS;
}


/* 
  Used to get the slotmap from a record block
  NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  this->getHeader(&head);

  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  for (int i = 0; i < slotCount; i ++ ){
    slotMap[i] = slotMapInBuffer[i];
  }

  return SUCCESS;
}


/*
  Used to load a block to the buffer and get a pointer to it.
  NOTE: this function expects the caller to allocate memory for the argument
*/
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if (bufferNum != E_BLOCKNOTINBUFFER) {
    // If buffer present, increment all others
    for (int i = 0; i < BUFFER_CAPACITY; i ++ ){
      if ( StaticBuffer::metainfo[i].free == false ){
        StaticBuffer::metainfo[i].timeStamp++;
      }
    }
    // Set current to zero 
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    
  }
  else{
    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) return E_OUTOFBOUND;
    // Do readblock if required block is not present in buffer
    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
  // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
  *buffPtr = StaticBuffer::blocks[bufferNum];
  // printf("debug2: %u\n",buffPtr);
  return SUCCESS;
}


// Used to perform operations and comparisons in SQL queries
int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    double diff;
    if (attrType == STRING)
        diff = strcmp(attr1.sVal, attr2.sVal);
    else
        diff = attr1.nVal - attr2.nVal;

    if (diff > 0) return 1;
    else if (diff < 0) return -1;
    else return 0;
}