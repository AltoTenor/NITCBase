#include "Algebra.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

// will return if a string can be parsed as a floating point number
bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

/// @brief Performs primary checks and inserts using BlockAccess::Insert
/// @param relName 
/// @param nAttrs 
/// @param record 
/// @return Status Code
int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){

    // Cant insert into RELCAT or ATTRCAT
    if ( strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0 ){
      return E_NOTPERMITTED;
    }

    int relId = OpenRelTable::getRelId(relName);

    // if relation is not open in open relation table, return E_RELNOTOPEN
    if ( relId == E_RELNOTOPEN) {
      return E_RELNOTOPEN;
    }

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    if ( nAttrs != relCatEntry.numAttrs ){
      return E_NATTRMISMATCH;
    }

    Attribute recordValues[relCatEntry.numAttrs];

    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    // iterate through 0 to nAttrs-1: (let i be the iterator)
    for (int i = 0; i < nAttrs; i++ ){
      AttrCatEntry attrCatEntry;
      AttrCacheTable::getAttrCatEntry(relId, i, &attrCatEntry);

      int type = attrCatEntry.attrType;
      if (type == NUMBER){
        // if the char array record[i] can be converted to a number
        // (check this using isNumber() function)
        if ( isNumber(record[i]) ){
          recordValues[i].nVal = atof(record[i]);
        }
        else return E_ATTRTYPEMISMATCH;
      }
      else if (type == STRING){
        strcpy(recordValues[i].sVal, record[i]);
      }
    }

    int retVal = BlockAccess::insert(relId, recordValues);
    return retVal;
}

/// @brief Used to select all the records that satisfy a condition
/// @param srcRel the source relation we want to select from
/// @param targetRel the relation we want to select into
/// @param attr the attribute that the condition is checking
/// @param op the operator of the condition
/// @param strVal the value that we want to compare against (represented as a string)
/// @return Status Code
int Algebra::select(  char srcRel[ATTR_SIZE], 
                      char targetRel[ATTR_SIZE], 
                      char attr[ATTR_SIZE], 
                      int op, 
                      char strVal[ATTR_SIZE]) {

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  // get the attr-cat entry for attr
  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
  if ( ret != SUCCESS ) {
    return E_ATTRNOTEXIST;
  }

  /*** Convert strVal (string) to an attribute of data type NUMBER or STRING ***/
  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {       // the isNumber() function is implemented below
      attrVal.nVal = atof(strVal);
    } else {
      return E_ATTRTYPEMISMATCH;
    }
  } else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }



  /*** Creating and opening the target relation ***/
  // Prepare arguments for createRel() in the following way:
  // get RelcatEntry of srcRel using RelCacheTable::getRelCatEntry()
  RelCatEntry srcRelCatEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &srcRelCatEntry);
  int src_nAttrs =  srcRelCatEntry.numAttrs;

  char attr_names[src_nAttrs][ATTR_SIZE];

  int attr_types[src_nAttrs];

  for ( int i = 0; i < src_nAttrs; i ++ ){
    AttrCatEntry srcAttrCatEntry;
    AttrCacheTable::getAttrCatEntry(srcRelId, i, &srcAttrCatEntry);
    strcpy(attr_names[i], srcAttrCatEntry.attrName);
    attr_types[i] = srcAttrCatEntry.attrType;
  }

  // Create the relation for target relation
  ret = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
  if ( ret != SUCCESS ) return ret;

  // Open the newly created target relation
  int tarRelId = OpenRelTable::openRel(targetRel);

  // If opening fails, delete the target relation
  if ( tarRelId < 0 ) {
    Schema::deleteRel(targetRel);
    return tarRelId;
  }

  /*** Selecting and inserting records into the target relation ***/
  /* Before calling the search function, reset the search to start from the
      first using RelCacheTable::resetSearchIndex() */
  Attribute record[src_nAttrs];

  /*
      The BlockAccess::search() function can either do a linearSearch or
      a B+ tree search. Hence, reset the search index of the relation in the
      relation cache using RelCacheTable::resetSearchIndex().
      Also, reset the search index in the attribute cache for the select
      condition attribute with name given by the argument `attr`. Use
      AttrCacheTable::resetSearchIndex().
      Both these calls are necessary to ensure that search begins from the
      first record.
  */
  //-----------------------B+TREE-------------------------------------------------------------------
  RelCacheTable::resetSearchIndex(srcRelId);
  AttrCacheTable::resetSearchIndex(srcRelId, attr);

  // read every record that satisfies the condition by repeatedly calling
  // BlockAccess::search() until there are no more records to be read

  while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS ) {

    int ret = BlockAccess::insert(tarRelId, record);
    if ( ret != SUCCESS ){
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  
  // Close the relation 
  Schema::closeRel(targetRel);
  return SUCCESS;
}

/// @brief Project all columns into another relation (Copy relation basically)
/// @param srcRel string corresponding to Source Relation Name
/// @param targetRel string corresponding to Target new Relation Name
/// @return Status Code
int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

  int ret;

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if ( srcRelId == E_RELNOTOPEN ) return E_RELNOTOPEN;

  RelCatEntry srcRelCatEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &srcRelCatEntry);

  int numAttrs = srcRelCatEntry.numAttrs;

  // attrNames and attrTypes will be used to store the attribute names
  // and types of the source relation respectively
  char attrNames[numAttrs][ATTR_SIZE];
  int attrTypes[numAttrs];

  /*iterate through every attribute of the source relation :
      - get the AttributeCat entry of the attribute with offset.
        (using AttrCacheTable::getAttrCatEntry())
      - fill the arrays `attrNames` and `attrTypes` that we declared earlier
        with the data about each attribute
  */
  for (int i = 0; i < numAttrs ; i++ ){
    AttrCatEntry srcAttrCatEntry;
    AttrCacheTable::getAttrCatEntry(srcRelId, i, &srcAttrCatEntry);
    strcpy(attrNames[i], srcAttrCatEntry.attrName);
    attrTypes[i] = srcAttrCatEntry.attrType;
  }


  /*** Creating and opening the target relation ***/

  ret = Schema::createRel(targetRel, numAttrs, attrNames, attrTypes);
  if ( ret != SUCCESS ) return ret;

  int tarRelId = OpenRelTable::openRel(targetRel);

  if ( tarRelId < 0 ) {
    Schema::deleteRel(targetRel);
    return tarRelId;
  }

  /*** Inserting projected records into the target relation ***/

  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[numAttrs];


  while (BlockAccess::project(srcRelId, record) == SUCCESS )
  {
    ret = BlockAccess::insert(tarRelId, record);

    if (ret != SUCCESS) {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  Schema::closeRel(targetRel);
  return SUCCESS;
}

/// @brief Project only specified columns into another relation
/// @param srcRel string containing source relation name
/// @param targetRel string containing target relation name
/// @param tar_nAttrs number of attributes in the target relation
/// @param tar_Attrs array of attributes required in the target relation
/// @return Status Code
int Algebra::project( char srcRel[ATTR_SIZE], 
                      char targetRel[ATTR_SIZE], 
                      int tar_nAttrs, 
                      char tar_Attrs[][ATTR_SIZE] ) {
  
  int ret;

  int srcRelId = OpenRelTable::getRelId(srcRel);
  if ( srcRelId == E_RELNOTOPEN ) return E_RELNOTOPEN;

  RelCatEntry srcRelCatEntry;
  RelCacheTable::getRelCatEntry(srcRelId, &srcRelCatEntry);

  int src_nAttrs = srcRelCatEntry.numAttrs;

  // where i-th entry will store the offset in a record of srcRel for the
  // i-th attribute in the target relation.
  int attr_offset[tar_nAttrs];  
  // where i-th entry will store the type of the i-th attribute in the
  // target relation.
  int attr_types[tar_nAttrs];


  /*** Checking if attributes of target are present in the source relation
       and storing its offsets and types ***/
  for (int i = 0; i < tar_nAttrs; i ++ ){
    AttrCatEntry tarAttrCatEntry;
    ret = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &tarAttrCatEntry);
    if ( ret != SUCCESS ) return E_ATTRNOTEXIST;
    attr_offset[i] = tarAttrCatEntry.offset;
    attr_types[i] = tarAttrCatEntry.attrType;
  }

  /*** Creating and opening the target relation ***/
  ret = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
  if ( ret != SUCCESS ) return ret;

  // Try to open and delete relation if open fails
  int tarRelId = OpenRelTable::openRel(targetRel);

  if ( tarRelId < 0 ) {
    Schema::deleteRel(targetRel);
    return tarRelId;
  }


  /*** Inserting projected records into the target relation ***/

  RelCacheTable::resetSearchIndex(srcRelId);
  Attribute record[src_nAttrs];

  while (BlockAccess::project(srcRelId, record) == SUCCESS ){
    // the variable `record` will contain the next record
    Attribute proj_record[tar_nAttrs];

    for (int i = 0; i < tar_nAttrs; i++){
      proj_record[i] = record[attr_offset[i]];
      // if ( attr_types[i] == NUMBER ) proj_record[i].nVal = record[attr_offset[i]].nVal;
      // else strcpy(proj_record[i].sVal, record[attr_offset[i]].sVal);
    }

    ret = BlockAccess::insert(tarRelId, proj_record);

    if (ret != SUCCESS) {
      Schema::closeRel(targetRel);
      Schema::deleteRel(targetRel);
      return ret;
    }
  }
  Schema::closeRel(targetRel);
  return SUCCESS;
}

/// @brief Join two source relations into one target relation
/// 
/// Should always the format SELECT * FROM S1 JOIN S2 INTO Tar where S1.x=S2.y;
/// @param srcRelation1 S1 -> string of first source relation 
/// @param srcRelation2 S2 -> string of second source relation 
/// @param targetRelation Tar -> string of target relation
/// @param attribute1 x -> join attribute of first source relation
/// @param attribute2 y -> join attribute of second source relation
/// @return Status Code
int Algebra::join(  char srcRelation1[ATTR_SIZE], 
                    char srcRelation2[ATTR_SIZE], 
                    char targetRelation[ATTR_SIZE], 
                    char attribute1[ATTR_SIZE], 
                    char attribute2[ATTR_SIZE] ){

  int ret;
  // get the srcRelation1's rel-id using OpenRelTable::getRelId() method
  int srcRelId1 = OpenRelTable::getRelId(srcRelation1);
  // get the srcRelation2's rel-id using OpenRelTable::getRelId() method
  int srcRelId2 = OpenRelTable::getRelId(srcRelation2);

  if ( srcRelId1<0 || srcRelId2<0 ) return E_RELNOTOPEN;

  AttrCatEntry attrCatEntry1, attrCatEntry2;
  ret = AttrCacheTable::getAttrCatEntry( srcRelId1, attribute1, &attrCatEntry1 );
  if ( ret != SUCCESS ) return E_ATTRNOTEXIST;


  ret = AttrCacheTable::getAttrCatEntry( srcRelId2, attribute2, &attrCatEntry2 );
  if ( ret != SUCCESS ) return E_ATTRNOTEXIST;

  if ( attrCatEntry1.attrType != attrCatEntry2.attrType ) return E_ATTRTYPEMISMATCH;

  // iterate through all the attributes in both the source relations and check if
  // there are any other pair of attributes other than join attributes
  // (i.e. attribute1 and attribute2) with duplicate names in srcRelation1 and
  // srcRelation2 (use AttrCacheTable::getAttrCatEntry())
  // If yes, return E_DUPLICATEATTR

  RelCatEntry relCatEntry1;
  RelCatEntry relCatEntry2;

  RelCacheTable::getRelCatEntry(srcRelId1, &relCatEntry1);
  RelCacheTable::getRelCatEntry(srcRelId2, &relCatEntry2);

  int numOfAttributes1 = relCatEntry1.numAttrs;
  int numOfAttributes2 = relCatEntry2.numAttrs;

  // Check that there are no duplicate entries 
  AttrCatEntry t1,t2;
  for ( int i=0; i < numOfAttributes1; i++ ){
    
    ret = AttrCacheTable::getAttrCatEntry(srcRelId1, i, &t1);
    if ( ret != SUCCESS ) return ret;
    
    for ( int j = 0; j < numOfAttributes2; j++ ){
      
      ret = AttrCacheTable::getAttrCatEntry(srcRelId2, j, &t2);
      if ( ret != SUCCESS ) return ret;

      if ( strcmp(t1.attrName, t2.attrName ) == 0 ){
        // If the join attributes are same then continue 
        if ( strcmp(t1.attrName, attribute1) == 0 ) continue;
        // Else its a problem
        return E_DUPLICATEATTR;
      }
    }
  }



  // if rel2 does not have an index on attr2
  if ( attrCatEntry2.rootBlock == -1 ){
    ret = BPlusTree::bPlusCreate(srcRelId2, attribute2);
    if ( ret != SUCCESS ) return ret;
  }

  // Number of attributes in target relation
  int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;

  // declare the following arrays to store the details of the target relation
  char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
  int targetRelAttrTypes[numOfAttributesInTarget];

  // iterate through all the attributes in both the source relations and
  // update targetRelAttrNames[],targetRelAttrTypes[] arrays excluding attribute2
  // in srcRelation2 (use AttrCacheTable::getAttrCatEntry())
  int j = 0;
  for (int i = 0; i < numOfAttributes1; i++ ){
    AttrCatEntry tempEntry;
    AttrCacheTable::getAttrCatEntry(srcRelId1, i, &tempEntry);
    strcpy(targetRelAttrNames[j], tempEntry.attrName);
    targetRelAttrTypes[j] = tempEntry.attrType;
    j++;
  }
  for (int i = 0; i < numOfAttributes2; i++ ){
    if ( i != attrCatEntry2.offset ) {
      AttrCatEntry tempEntry;
      AttrCacheTable::getAttrCatEntry(srcRelId2, i, &tempEntry);
      strcpy(targetRelAttrNames[j], tempEntry.attrName);
      targetRelAttrTypes[j] = tempEntry.attrType;
      j++;
    }
  }

  // creating the target relation
  ret = Schema::createRel(  targetRelation, 
                            numOfAttributesInTarget, 
                            targetRelAttrNames, 
                            targetRelAttrTypes );
  if ( ret != SUCCESS ) return ret;

  // Open the targetRelation
  int targetRelId = OpenRelTable::openRel(targetRelation);

  // if openRel() fails (No free entries left in the Open Relation Table)
  if ( targetRelId < 0 ){
    Schema::deleteRel(targetRelation);
    return targetRelId;
  }

  Attribute record1[numOfAttributes1];
  Attribute record2[numOfAttributes2];
  Attribute targetRecord[numOfAttributesInTarget];

  RelCacheTable::resetSearchIndex(srcRelId1);
  AttrCacheTable::resetSearchIndex(srcRelId1, attribute1);

  // this loop is to get every record of the srcRelation1 one by one
  while (BlockAccess::project(srcRelId1, record1) == SUCCESS) {

    RelCacheTable::resetSearchIndex(srcRelId2);
    AttrCacheTable::resetSearchIndex(srcRelId2, attribute2);

    // record1.attribute1 = record2.attribute2 (i.e. Equi-Join condition)
    while (BlockAccess::search( srcRelId2, record2, attribute2, record1[attrCatEntry1.offset], EQ ) == SUCCESS ) {

        // copy srcRelation1's and srcRelation2's attribute values(except
        // for attribute2 in rel2) from record1 and record2 to targetRecord
        int j = 0;
        for (int i = 0; i < numOfAttributes1; i++){
          targetRecord[j++] = record1[i];
        }
        for (int i = 0; i < numOfAttributes2; i++){
          if ( i != attrCatEntry2.offset ) targetRecord[j++] = record2[i];
        }

        ret = BlockAccess::insert(targetRelId, targetRecord);

        if( ret == E_DISKFULL ) {
          OpenRelTable::closeRel(targetRelId);
          Schema::deleteRel(targetRelation);
          return E_DISKFULL;
        }
    }
  }
  OpenRelTable::closeRel(targetRelId);
  return SUCCESS;
}
