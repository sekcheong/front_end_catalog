#include "catalog.h"


const Status RelCatalog::createRel(const string & relation,
                                   const int attrCnt,
                                   const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad;
  
  if (relation.empty() || attrCnt < 1) {
    return BADCATPARM;
  }
  
  if (relation.length() >= sizeof rd.relName) {
    return NAMETOOLONG;
  }
  
  // make sure the relation doesn't already exist
  status = getInfo(relation, rd);
  if (status == OK) {
    return RELEXISTS;
  }
  
  if (status != RELNOTFOUND) {
    return status;
  }
  
  // check duplicated attribute name
  int tupleWidth = attrList[0].attrLen;
  if (attrCnt > 1) {
    for (int i = 1; i < attrCnt; i++) {
      tupleWidth += attrList[i].attrLen;
      for (int j = 0; j < i; j++) {
        if (strcmp(attrList[i].attrName, attrList[j].attrName) == 0) {
          return DUPLATTR;
        }
      }
    }
  }
  
  // make sure the relation will fit on a page
  if (tupleWidth > PAGESIZE) {
    return ATTRTOOLONG;
  }
  
  // insert relation info
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;
  if ((status = addInfo(rd)) != OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling addInfo()" << endl;
    return status;
  }
  
  // insert attribute info
  strcpy(ad.relName, relation.c_str());
  int offset = 0;
  for(int i = 0; i < attrCnt; i++) {
    if (strlen(attrList[i].attrName) >= sizeof ad.attrName)
      return NAMETOOLONG;
    strcpy(ad.attrName, attrList[i].attrName);
    ad.attrOffset = offset;
    ad.attrType = attrList[i].attrType;
    ad.attrLen = attrList[i].attrLen;
    if ((status = attrCat->addInfo(ad)) != OK) {
      cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling attrCat->addInfo()" << endl;
      return status;
    }
    offset += ad.attrLen;
  }
  
  //create the heap file to store the relation
  status = createHeapFile (relation);
  if (status != OK) return status;
  return OK;
}
--------------------