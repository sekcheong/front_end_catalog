#include "catalog.h"
#include "query.h"

/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */
const Status QU_Delete(const string & relation,
                       const string & attrName,
                       const Operator op,
                       const Datatype type,
                       const char *attrValue) {
  
  Status status = OK;
  RID rid;
  Record rec;
  AttrDesc attrInfo;
  
  const char *filter;

  int valI;
  float valF;
  if (type == INTEGER) {
    valI = atoi(attrValue);
    filter = (char *) &valI;
  }
  else if (type == FLOAT) {
    valF = atof(attrValue);
    filter = (char *) &valF;
  }
  else {
    filter = attrValue;
  }
  
  HeapFileScan hfs = HeapFileScan(relation, status);
  
  status = attrCat->getInfo(relation, attrName, attrInfo);
  if (status != OK) {
    return status;
  }
  
  status = hfs.startScan(attrInfo.attrOffset, attrInfo.attrLen, type, filter, op);
  if (status != OK) {
    return status;
  }
  // Find the record, and delete it.
  while ((hfs.scanNext(rid)) == OK) {
    status = hfs.getRecord(rec);
    if (status != OK) {
      return status;
    }
    
    status = hfs.deleteRecord();
    if (status != OK) {
      return status;
    }
  }
  
  return status;
}