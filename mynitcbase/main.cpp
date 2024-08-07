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

//-----------------------------------------------------------------
// Exercise - Stage 1
// ------------------
// int main(int argc, char *argv[]) {
//   Disk disk_run;

//   unsigned char buffer2[BLOCK_SIZE];
//   Disk::readBlock(buffer2, 0);
//   for (auto x:buffer2) printf("%d ",x);
//   return 0;
// }

//-----------------------------------------------------------------

// Stage 1 
// ------------------
// int main(int argc, char *argv[]) {
//   Disk disk_run;

//   unsigned char buffer[BLOCK_SIZE];
//   Disk::readBlock(buffer, 7000);
//   char message[] = "hello";
//   memcpy(buffer + 20, message, 6);
//   Disk::writeBlock(buffer, 7000);

//   unsigned char buffer2[BLOCK_SIZE];
//   char message2[6];
//   Disk::readBlock(buffer2, 7000);
//   memcpy(message2, buffer2 + 20, 6);
//   std::cout << message2;

//   return 0;
// }
//-----------------------------------------------------------------