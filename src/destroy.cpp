#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;
  
  if (relation.empty() ||
      relation == string(RELCATNAME) ||
      relation == string(ATTRCATNAME))
    return BADCATPARM;
  
  status = attrCat->dropRelation(relation);
  if (status!=OK) {
    return status;
  }
  
  status = removeInfo(relation);
  if (status!=OK) {
    return status;
  }
  
  status = destroyHeapFile(relation);
  if (status!=OK) {
    return status;
  }
  return OK;
}

//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  Status status;
  AttrDesc *attrs;
  int attrCnt, i;
  
  status = getRelInfo(relation, attrCnt, attrs);
  if (status!=OK) {
    return status;
  }
  
  for(i = 0; i < attrCnt; i++) {
    status = removeInfo(relation, attrs[i].attrName);
    if (status!=OK) {
      return status;
    }
  }
  
  free(attrs);
  return OK;
}


