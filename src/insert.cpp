#include "catalog.h"
#include "query.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */
const Status QU_Insert(const string & relation,
                       const int attrCnt,
                       const attrInfo attrList[])
{
  Status status = OK;
  Record record;
  RID rid;
  int attrLen = 0;
  int len = 0;
  RelDesc relinfo;
  AttrDesc* attrinfo;
  
  InsertFileScan ifs = InsertFileScan(relation, status);
  if (status != OK) {
    return status;
  }
  
  // Get the information and relation information about the given relation
  status = relCat->getInfo(relation, relinfo);
  if (status != OK) {
    return status;
  }
  
  status = attrCat->getRelInfo(relation, attrLen, attrinfo);
  if (status != OK) {
    return status;
  }
  
  // Find matching attributes to get the length of the records
  for (int i = 0; i < attrLen; i++) {
    for (int j = 0; j < attrCnt; j++) {
      if (strcmp(attrinfo[i].attrName, attrList[j].attrName) == 0) {
        len = len + attrinfo[i].attrLen;
      }
    }
  }
  
  record.data = (char*) malloc(len);
  record.length = len;
		
  // Find each attrValue, get the value at a string, and put it into data.
  for (int i = 0; i < attrCnt; i++) {
    for (int j = 0; j < attrLen; j++) {
      if (strcmp(attrList[i].attrName, attrinfo[j].attrName) == 0) {
        int type = attrinfo[j].attrType;
        if (type == INTEGER) {
          int v = atoi((char*)attrList[i].attrValue);
          memcpy((char*)record.data + attrinfo[j].attrOffset, &v, attrinfo[j].attrLen);
        }
        else if (type == FLOAT)
        {
          float v = atof((char*)attrList[i].attrValue);
          memcpy((char*)record.data + attrinfo[j].attrOffset, &v, attrinfo[j].attrLen);
        }
        else {
          memcpy((char*)record.data + attrinfo[j].attrOffset, (char*)attrList[i].attrValue, attrinfo[j].attrLen - 1);
        }
      }
    }
  }
  return ifs.insertRecord(record, rid);
}