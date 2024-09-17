#include "Frontend.h"

#include <cstring>
#include <iostream>
#include <bits/stdc++.h>
int Frontend::create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE], int type_attrs[]) {
  return Schema::createRel(relname, no_attrs, attributes, type_attrs);
}

int Frontend::drop_table(char relname[ATTR_SIZE]) {
  return Schema::deleteRel(relname);
}

int Frontend::open_table(char relname[ATTR_SIZE]) {
  return Schema::openRel(relname);
}

int Frontend::close_table(char relname[ATTR_SIZE]) {
  return Schema::closeRel(relname);
}

int Frontend::alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE]) {
  return Schema::renameRel(relname_from, relname_to);
}

int Frontend::alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[ATTR_SIZE],
                                        char attrname_to[ATTR_SIZE]) {
  return Schema::renameAttr(relname, attrname_from, attrname_to);
}

int Frontend::create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  return Schema::createIndex(relname, attrname);
}

int Frontend::drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  return Schema::dropIndex(relname, attrname);
}

int Frontend::insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE]) {
  return Algebra::insert(relname, attr_count, attr_values);
}

int Frontend::select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE]) {
  return Algebra::project(relname_source, relname_target);
}

int Frontend::select_attrlist_from_table(char relname_source[ATTR_SIZE],
                                         char relname_target[ATTR_SIZE],
                                         int attr_count,
                                         char attr_list[][ATTR_SIZE]) {
  return Algebra::project(relname_source, relname_target, attr_count, attr_list);
}

int Frontend::select_from_table_where(char relname_source[ATTR_SIZE],
                                      char relname_target[ATTR_SIZE],
                                      char attribute[ATTR_SIZE],
                                      int op, char value[ATTR_SIZE]) {
  return Algebra::select(relname_source, relname_target, attribute, op, value);
}

int Frontend::select_attrlist_from_table_where(
  char relname_source[ATTR_SIZE], 
  char relname_target[ATTR_SIZE],
  int attr_count, 
  char attr_list[][ATTR_SIZE],
  char attribute[ATTR_SIZE], 
  int op, 
  char value[ATTR_SIZE]) {

  int ret = Algebra::select(relname_source, (char*)TEMP, attribute, op, value);
  if ( ret != SUCCESS ) return ret;

  int tempRelId = OpenRelTable::openRel((char*)TEMP);  
  if ( tempRelId < 0 ){ 
    Schema::deleteRel((char*)TEMP);
    return tempRelId;
  }

  ret = Algebra::project((char*)TEMP, relname_target, attr_count, attr_list);

  OpenRelTable::closeRel(tempRelId);  
  ret = Schema::deleteRel((char*)TEMP);
  return ret;
}

int Frontend::select_from_join_where(
  char relname_source_one[ATTR_SIZE], 
  char relname_source_two[ATTR_SIZE],
  char relname_target[ATTR_SIZE],
  char join_attr_one[ATTR_SIZE], 
  char join_attr_two[ATTR_SIZE]) {
  
  return Algebra::join(  relname_source_one, 
                            relname_source_two, 
                            relname_target, 
                            join_attr_one, 
                            join_attr_two );
}

int Frontend::select_attrlist_from_join_where(
  char relname_source_one[ATTR_SIZE], 
  char relname_source_two[ATTR_SIZE],
  char relname_target[ATTR_SIZE], 
  char join_attr_one[ATTR_SIZE],
  char join_attr_two[ATTR_SIZE], 
  int attr_count, 
  char attr_list[][ATTR_SIZE]) {

  
  // TEMP results from the join of the two source relation (and hence it
  // contains all attributes of the source relations except the join attribute
  // of the second source relation)
  int ret = Algebra::join(  relname_source_one,   
                            relname_source_two , 
                            (char*)TEMP, 
                            join_attr_one, 
                            join_attr_two );

  if ( ret != SUCCESS ) return ret;

  int relId = OpenRelTable::openRel((char*)TEMP);
  if ( relId < 0 ){ 
    Schema::deleteRel((char*)TEMP);
    return ret;
  }
  
  // (The final target relation contains only those attributes mentioned in attr_list)
  ret = Algebra::project((char*)TEMP, relname_target, attr_count, attr_list); 
  if ( ret != SUCCESS ) return ret;

  ret = OpenRelTable::closeRel(relId);
  if ( ret != SUCCESS ) return ret;
  ret = Schema::deleteRel((char*)TEMP);
  if ( ret != SUCCESS ) return ret;

  return SUCCESS;
}

int Frontend::custom_function(int argc, char argv[][ATTR_SIZE]) {
  // argc gives the size of the argv array
  // argv stores every token delimited by space and comma

  // implement whatever you desire
  if ( argc == 2 ){
    int relId = OpenRelTable::getRelId(argv[0]);
    AttrCatEntry attrCatBuf;
    int ret = AttrCacheTable::getAttrCatEntry(relId, argv[1], &attrCatBuf);
    // printf("%s %s %d\n",argv[0], argv[1], ret);
    int rootBlock = attrCatBuf.rootBlock;
    if ( rootBlock == -1 ) printf("No index\n");

    
    std::queue <std::pair<int,int>> q;
    int curlvl = -1;
    q.push({rootBlock,0});
    while(!q.empty()){
      int blockNum = q.front().first;
      int lvl = q.front().second;
      if ( lvl != curlvl ){
        curlvl = lvl;
        printf(" \nLevel %d\n ", lvl );
      }
      q.pop();
      int type = StaticBuffer::getStaticBlockType(blockNum);
      if ( type == IND_INTERNAL ){
        IndInternal internalBlk(blockNum);
        HeadInfo internalHeader;
        internalBlk.getHeader(&internalHeader);
        int numEntries = internalHeader.numEntries;
        // printf("T %d\n",internalHeader.numEntries);

        InternalEntry entry;
        internalBlk.getEntry(&entry, 0);
        q.push({ entry.lChild, lvl+1 });

        for (int i=0;i<numEntries;i++){
          internalBlk.getEntry(&entry, i);
          printf("%s ",entry.attrVal.sVal);
          q.push({ entry.rChild, lvl+1 });
        }
      }
      else{
        IndLeaf leafBlk(blockNum);
        HeadInfo leafHeader;
        leafBlk.getHeader(&leafHeader);
        int numEntries = leafHeader.numEntries;
        Index entry;
        for (int i=0;i<numEntries;i++){
          leafBlk.getEntry(&entry, i);
          printf("%s ",entry.attrVal.sVal);
        }
      }

    }
  }
  return SUCCESS;
}