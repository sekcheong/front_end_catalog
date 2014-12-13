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
                       const char *attrValue)
{
  
  
  Status status = OK;
  HeapFileScan hfs = HeapFileScan(relation, status);
  
  if (status!=OK) {
    return status;
  }
  
  RID rid;
  Record rec;
  AttrDesc attrInfo;
  
  const char *filter;
  if (type == INTEGER) {
    int val = atoi(attrValue);
    filter = (char *) &val;
  }
  else if (type == FLOAT) {
    float val = atof(attrValue);
    filter = (char *) &val;
  }
  else {
    filter = attrValue;
  }
  
  status = attrCat->getInfo(relation, attrName, attrInfo);
  if (status != OK) {
    return status;
  }
  
  status = hfs.startScan(attrInfo.attrOffset, attrInfo.attrLen,
                         type, filter, op);
  if (status != OK) {
    return status;
  }
  
  // find the record to delete
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
  
  return OK;
}