#include "catalog.h"
#include "stdlib.h"

RelCatalog::RelCatalog(Status &status) :
HeapFile(RELCATNAME, status)
{
}


const Status RelCatalog::getInfo(const string & relation, RelDesc &record)
{
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  Status status;
  Record rec;
  RID rid;
  
  HeapFileScan hfs = HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING, relation.c_str(), EQ);
  if (status!= OK){
    return status;
  }
  
  status = hfs.scanNext(rid);
  if (status == FILEEOF) status = RELNOTFOUND;
  else if (status == OK) {
    if ((status = hfs.getRecord(rec)) != OK) return status;
    assert(sizeof(RelDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
  }
  
  return status;
}


const Status RelCatalog::addInfo(RelDesc & record)
{
  RID rid;
  Status status;
  
  InsertFileScan ifs = InsertFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  int len = (int)strlen(record.relName);
  memset(&record.relName[len], 0, sizeof(record.relName) - len);
  Record rec;
  
  rec.data = &record;
  rec.length = sizeof(RelDesc);
  status = ifs.insertRecord(rec, rid);
  return status;
}

const Status RelCatalog::removeInfo(const string &relation)
{
  
  RID rid;
  Status status;
  HeapFileScan hfs = HeapFileScan(RELCATNAME, status);
  if (status != OK) return status;
  
  if (relation.empty()) return BADCATPARM;
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING, relation.c_str(), EQ);
  if (!status) return status;
  
  status = hfs.scanNext(rid);
  if (status == FILEEOF) status = RELNOTFOUND;
  if (status == OK) status = hfs.deleteRecord();
  
  if (status == NORECORDS) status = OK;
  return status;
}


RelCatalog::~RelCatalog()
{
}


AttrCatalog::AttrCatalog(Status &status) :
HeapFile(ATTRCATNAME, status)
{
}


const Status AttrCatalog::getInfo(const string & relation,
                                  const string & attrName,
                                  AttrDesc &record)
{
  
  Status status;
  RID rid;
  Record rec;
  
  if (relation.empty() || attrName.empty()) return BADCATPARM;
  
  HeapFileScan hfs = HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  status = hfs.startScan(0, (int)relation.length() + 1, STRING, relation.c_str(), EQ);
  if (status!=OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling ifs.startScan()" << endl;
    return status;
  }
  
  while((status = hfs.scanNext(rid)) == OK) {
    status = hfs.getRecord(rec);
    if (status != OK) return status;
    assert(sizeof(AttrDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
    if (string(record.attrName) == attrName) {
      break;
    }
  }
  
  if (status == FILEEOF) {
    status = ATTRNOTFOUND;
  }
  
  return status;
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID rid;
  
  Status status;
  InsertFileScan ifs = InsertFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  int len = (int) strlen(record.relName);
  memset(&record.relName[len], 0, sizeof(record.relName) - len);
  len = (int) strlen(record.attrName);
  memset(&record.attrName[len], 0, sizeof(record.attrName) - len);
  
  Record rec;
  rec.data = &record;
  rec.length = sizeof(AttrDesc);
  //cout << "insert record into attCat of size " << rec.length << endl;
  status = ifs.insertRecord(rec, rid);
  if (status != OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling ifs.insertRecord()" << endl;
  }
  return status;
}


const Status AttrCatalog::removeInfo(const string &relation,
                                     const string &attrName)
{
  Status status;
  Record rec;
  RID rid;
  AttrDesc record;
  
  if (relation.empty() || attrName.empty()) return BADCATPARM;
  
  HeapFileScan hfs = HeapFileScan(ATTRCATNAME, status);
  if (status != OK) {
    return status;
  }
  
  status = hfs.startScan(0, (int)relation.length() + 1, STRING, relation.c_str(), EQ);
  if (status!=OK) {
    return status;
  }
  
  while((status = hfs.scanNext(rid)) == OK) {
    if ((status = hfs.getRecord(rec)) != OK) return status;
    
    assert(sizeof(AttrDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
    if (string(record.attrName) ==  attrName) break;
  }
  if (status == FILEEOF) status = RELNOTFOUND;
  if (status == OK) {
    status = hfs.deleteRecord();
  }
  if (status == NORECORDS) return OK;
  return status;
}


const Status AttrCatalog::getRelInfo(const string &relation,
                                     int &attrCnt,
                                     AttrDesc *&attrs)
{
  Status status;
  RID rid;
  Record rec;
  
  if (relation.empty()) return BADCATPARM;
  
  HeapFileScan hfs = HeapFileScan(ATTRCATNAME, status);
  if (status != OK) return status;
  
  status = hfs.startScan(0, (int)relation.length() + 1, STRING, relation.c_str(), EQ);
  if (status!=OK) {
    return status;
  }
  
  attrCnt = 0;
  while((status = hfs.scanNext(rid)) == OK) {
    if ((status = hfs.getRecord(rec)) != OK) return status;
    
    assert(sizeof(AttrDesc) == rec.length);
    attrCnt++;
    if (attrCnt == 1) {
      if (!(attrs = (AttrDesc*)malloc(sizeof(AttrDesc)))) {
        return INSUFMEM;
      }
    }
    else {
      if (!(attrs = (AttrDesc*)realloc(attrs, attrCnt * sizeof(AttrDesc)))) {
        return INSUFMEM;
      }
    }
    memcpy(&attrs[attrCnt - 1], rec.data, rec.length);
  }
  
  if (status == FILEEOF) {
    if (attrCnt == 0) status = RELNOTFOUND;
    else status = OK;
  }
  
  return status;
}


AttrCatalog::~AttrCatalog()
{
  
}
