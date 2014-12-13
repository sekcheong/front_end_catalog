#include "catalog.h"


const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status   status;
  RelDesc  rd;
  AttrDesc ad;

  if (relation.empty() || attrCnt < 1) {
    return BADCATPARM;
  }

  if (relation.length() >= sizeof rd.relName) {
    return NAMETOOLONG;
  }

  // make sure the relation we are going to cerate doesn't already exist
  status = getInfo(relation, rd);
  if (status == OK) {
    return RELEXISTS;
  }
  else if (status != RELNOTFOUND) {
    return status;
  }
  
  // make sure attribute names are unique
  int tupleWidth = attrList[0].attrLen;
  if (attrCnt > 1) {
    for (int i = 1; i < attrCnt; i++) {
      tupleWidth += attrList[i].attrLen;
      for (int j = 0; j < i; j++)
        if (strcmp(attrList[i].attrName, attrList[j].attrName) == 0)
          return DUPLATTR;
    }
  }
  
  // make sure the tuple fits within a page
  if (tupleWidth > PAGESIZE)  {
    return ATTRTOOLONG;
  }
  
  // add the relation information
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;
  status = addInfo(rd);
  if (status != OK) {
    return status;
  }
  
  // add the attribute information
  strcpy(ad.relName, relation.c_str());
  int offset = 0;
  for (int i = 0; i < attrCnt; i++) {
    if (strlen(attrList[i].attrName) >= sizeof ad.attrName) {
      return NAMETOOLONG;
    }
    strcpy(ad.attrName, attrList[i].attrName);
    ad.attrOffset = offset;
    ad.attrType = attrList[i].attrType;
    ad.attrLen = attrList[i].attrLen;
    status = status = attrCat->addInfo(ad);
    if (status != OK) {
      return status;
    }
    offset += ad.attrLen;
  }
  
  // create the heap file that stores the relation
  status = createHeapFile (relation);
  if (status != OK) return status;
  return OK;
}