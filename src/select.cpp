#include "catalog.h"
#include "query.h"
#include <stdlib.h>
#include <stdio.h>

//forward declaration
const Status ScanSelect(const string & result,
                        const int projCnt,
                        const AttrDesc projNames[],
                        const AttrDesc *attrDesc,
                        const Operator op,
                        const char *filter,
                        const int reclen);

const Status QU_Select(const string & result,
                       const int projCnt,
                       const attrInfo projNames[],
                       const attrInfo *attr,
                       const Operator op,
                       const char *attrValue)
{
  cout << "Doing QU_Select " << endl;
  
  Status status;
  AttrDesc attrDesc;
  const char *filter;
  
  
  AttrDesc* projs = new AttrDesc[projCnt];
  for (int i = 0; i < projCnt; i++) {
    status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projs[i]);
    if (status != OK) {
      return status;
    }
  }
  
  if (attr != NULL) {
    status = attrCat-> getInfo(attr->relName, attr->attrName, attrDesc);
    if (status != OK) {
      return status;
    }
  }
  
  int reclen = 0;
  for (int i = 0; i < projCnt; i++) {
    reclen = reclen + projs[i].attrLen;
  }
  
  int valI;
  float valF;
  int type = attrDesc.attrType;
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
  
  return ScanSelect(result, projCnt, projs, &attrDesc, op, filter, reclen);
}


const Status ScanSelect(const string & result,
                        const int projCnt,
                        const AttrDesc projNames[],
                        const AttrDesc *attrDesc,
                        const Operator op,
                        const char *filter,
                        const int reclen)
{
  cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;
  
  Status status;
  Record outputRec;
  Record rec;
  RID rid;
  
  // See if the relation is already in the DB
  InsertFileScan resultRel = InsertFileScan(result, status);
  if (status != OK) {
    return status;
  }
  
  outputRec.data = (char*) malloc(reclen);
  outputRec.length = reclen;
  
  // Prepare to scan the heap!
  HeapFileScan hfs = HeapFileScan(projNames->relName, status);
  
  status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype)attrDesc->attrType, filter, op);
  if (status != OK) {
    return status;
  }
  
  // Scan for all the records we need.
  while (hfs.scanNext(rid) == OK) {
    status = hfs.getRecord(rec);
    if (status != OK) {
      return status;
    }
    int offset = 0;
    for (int i = 0; i < projCnt; i++) {
      memcpy((char *)outputRec.data + offset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
      offset += projNames[i].attrLen;
    }
    RID output;
    status = resultRel.insertRecord(outputRec, output);
  }
  return status;
}