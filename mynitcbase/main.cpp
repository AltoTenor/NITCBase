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


  for (int i = 0; i <= 2 ; i ++ ){
    RelCatEntry relCatBuf;
    int response = RelCacheTable::getRelCatEntry(i, &relCatBuf);
    if (response != SUCCESS)
    {
      printf("Relation Catalogue Entry Not found.\n");
      exit(1);
    }
    printf("Relation: %s\n", relCatBuf.relName);
    for (int j=0 ; j < relCatBuf.numAttrs; j ++ ){
      AttrCatEntry attrCatBuf;
      response = AttrCacheTable::getAttrCatEntry(i, j, &attrCatBuf);
      if (response != SUCCESS)
      {
        printf("Attribute Catalogue Entry Not found.\n");
        exit(1);
      }
      printf("  %s: %s\n", attrCatBuf.attrName, attrCatBuf.attrType==NUMBER?"NUM":"STR");
    }

  }
  return 0;
}

