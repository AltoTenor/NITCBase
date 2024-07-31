#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <string>

//-----------------------------------------------------------------
// Stage 3
// ------------------

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;


  for (int i = 0; i < 2 ; i ++ ){
    RelCatEntry relCatBuf;
    int response = RelCacheTable::getRelCatEntry(i, &relCatBuf);
    if (response != SUCCESS)
    {
      printf("Relation Catalogue Entry Not found.");
      exit(1);
    }
    printf("Relation: %s\n", relCatBuf.relName);
    for (int j=0 ; j < relCatBuf.numAttrs; j ++ ){
      AttrCatEntry attrCatBuf;
      response = AttrCacheTable::getAttrCatEntry(i, j, &attrCatBuf);
      if (response != SUCCESS)
      {
        printf("Relation Catalogue Entry Not found.");
        exit(1);
      }
      printf("  %s: %s\n", attrCatBuf.attrName, attrCatBuf.attrType==NUMBER?"NUM":"STR");
    }

  }
  /*
  for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)

      get the relation catalog entry using RelCacheTable::getRelCatEntry()
      printf("Relation: %s\n", relname);

      for j = 0 to numAttrs of the relation - 1
          get the attribute catalog entry for (rel-id i, attribute offset j)
           in attrCatEntry using AttrCacheTable::getAttrCatEntry()

          printf("  %s: %s\n", attrName, attrType);
  */

  return 0;































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

