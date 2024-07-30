#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <string>

//-----------------------------------------------------------------
// Exercise - Stage 2 - update Student record
// ------------------

int main(int argc, char *argv[]) {
  Disk disk_run;

  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  HeadInfo relCatHeader;
  // load the headers of both the blocks into relCatHeader and attrCatHeader - Done
  relCatBuffer.getHeader(&relCatHeader);


  for (int i = 0; i < relCatHeader.numEntries ; i++ ) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    // Loop through all possible attrCat Blocks
    int blockNum = ATTRCAT_BLOCK;
    while( true ) {

      RecBuffer attrCatBuffer(blockNum);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j = 0; j < attrCatHeader.numEntries; j++ ) {

        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[RELCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        if ( strcmp( attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, 
                      relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0 &&
             strcmp(relCatRecord[RELCAT_REL_NAME_INDEX].sVal, "Students") == 0 ){

          const char *attrType = 
            attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

          // printf(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal);
          if ( strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch") == 0 ){
            unsigned char buffer[BLOCK_SIZE];
            // read the block at this.blockNum into a buffer
            Disk::readBlock(buffer,ATTRCAT_BLOCK);
            int slotCount = attrCatHeader.numSlots;
            int attrCount = attrCatHeader.numAttrs;
            int recordSize = attrCount * ATTR_SIZE;
            unsigned char *slotPointer = buffer+32+slotCount+(recordSize*j)+16;
            char new_class[]="Class";
            
            memcpy(slotPointer,new_class,16);
            Disk::writeBlock(buffer,ATTRCAT_BLOCK);
          }
        }
        
      }
      // printf("%d\n", attrCatHeader.rblock);
      if ( attrCatHeader.rblock == -1 ) break;
      else blockNum = attrCatHeader.rblock;
    }
  }

  for (int i = 0; i < relCatHeader.numEntries ; i++ ) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    // Loop through all possible attrCat Blocks
    int blockNum = ATTRCAT_BLOCK;
    while( true ) {

      RecBuffer attrCatBuffer(blockNum);
      HeadInfo attrCatHeader;
      attrCatBuffer.getHeader(&attrCatHeader);

      for (int j = 0; j < attrCatHeader.numEntries; j++ ) {

        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[RELCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        if ( strcmp( attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, 
                      relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0 ){

          const char *attrType = 
            attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal , attrType);
        }
        
      }
      // printf("%d\n", attrCatHeader.rblock);
      if ( attrCatHeader.rblock == -1 ) break;
      else blockNum = attrCatHeader.rblock;
    }

    printf("\n");
  }


  return 0;
}




//-----------------------------------------------------------------
// Exercise - Stage 2 - Multiple Block Access
// ------------------

// int main(int argc, char *argv[]) {
//   Disk disk_run;

//   // create objects for the relation catalog and attribute catalog
//   RecBuffer relCatBuffer(RELCAT_BLOCK);

//   HeadInfo relCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader - Done
//   relCatBuffer.getHeader(&relCatHeader);


//   for (int i = 0; i < relCatHeader.numEntries ; i++ ) {

//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

//     // Loop through all possible attrCat Blocks
//     int blockNum = ATTRCAT_BLOCK;
//     while( true ) {

//       RecBuffer attrCatBuffer(blockNum);
//       HeadInfo attrCatHeader;
//       attrCatBuffer.getHeader(&attrCatHeader);

//       for (int j = 0; j < attrCatHeader.numEntries; j++ ) {

//         // declare attrCatRecord and load the attribute catalog entry into it
//         Attribute attrCatRecord[RELCAT_NO_ATTRS];
//         attrCatBuffer.getRecord(attrCatRecord, j);

//         if ( strcmp( attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, 
//                       relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0 ){

//           const char *attrType = 
//             attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

//           printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal , attrType);
//         }
        
//       }
//       // printf("%d\n", attrCatHeader.rblock);
//       if ( attrCatHeader.rblock == -1 ) break;
//       else blockNum = attrCatHeader.rblock;
//     }

//     printf("\n");
//   }

//   return 0;
// }




//-----------------------------------------------------------------
// Stage 2
// ------------------

// int main(int argc, char *argv[]) {
//   Disk disk_run;

//   // create objects for the relation catalog and attribute catalog
//   RecBuffer relCatBuffer(RELCAT_BLOCK);
//   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   HeadInfo relCatHeader;
//   HeadInfo attrCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader - Done
//   relCatBuffer.getHeader(&relCatHeader);
//   attrCatBuffer.getHeader(&attrCatHeader);


//   for (int i = 0; i < relCatHeader.numEntries ; i++ ) {

//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

//     for (int j = 0; j < attrCatHeader.numEntries; j++ ) {

//       // declare attrCatRecord and load the attribute catalog entry into it
//       Attribute attrCatRecord[RELCAT_NO_ATTRS];
//       attrCatBuffer.getRecord(attrCatRecord, j);

//       if ( strcmp( attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, 
//                     relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0 ){

//         const char *attrType = 
//           attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

//         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal , attrType);
//       }
      
//     }
//     printf("\n");
//   }

//   return 0;
// }
