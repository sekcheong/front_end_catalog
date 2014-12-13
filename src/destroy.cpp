#include "catalog.h"
#include "stdlib.h"

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
  
  // destroy file
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
  Status status;
  AttrDesc *attrs;
  
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  // find attribute info
  int attrCnt;
  status = getRelInfo(relation, attrCnt, attrs);
  if (status!=OK) {
    return status;
  }
  
  // delete entries from catalog
  for (int i = 0; i < attrCnt; i++) {
    status = removeInfo(relation, attrs[i].attrName);
    if (status!=OK) {
      return status;
    }
  }
  
  free(attrs);
  
  return OK;
}