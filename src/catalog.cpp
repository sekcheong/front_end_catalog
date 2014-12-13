#include "catalog.h"


RelCatalog::RelCatalog(Status &status) :
HeapFile(RELCATNAME, status)
{
  // nothing should be needed here
}

//Open a scan on the relcat relation by invoking the startScan()
//method on itself
const Status RelCatalog::getInfo(const string &relation, RelDesc &record)
{
  
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  Status status;
  Record rec;
  RID    rid;
  
  HeapFileScan hfs = HeapFileScan(RELCATNAME, status);
  if (status != OK) {
    return status;
  }
  
  status = hfs.startScan(0, ((int) relation.length() + 1), STRING,
                         relation.c_str(), EQ);
  if (status!= OK) {
    return status;
  }
  
  status = hfs.scanNext(rid);
  if (status == OK) {
    status = hfs.getRecord(rec);
    if (status!= OK) {
      return status;
    }
    assert(sizeof(RelDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
  }
  else if (status ==FILEEOF) {
    status = RELNOTFOUND;
  }
  
  return status;
}

//Adds the relation descriptor contained in record to the relcat relation.
//RelDesc represents both the in-memory format and on-disk format of
//a tuple in relcat.
const Status RelCatalog::addInfo(RelDesc &record)
{
  RID             rid;
  Status          status;
  InsertFileScan  ifs = InsertFileScan(RELCATNAME, status);
  
  if (status != OK) {
    return status;
  }
  
  //zeros out the record while preserving the record name
  int len = (int) strlen(record.relName);
  memset(&record.relName[len], 0, sizeof(record.relName) - len);
  
  Record rec;
  rec.data = &record;
  rec.length = sizeof(RelDesc);
  status = ifs.insertRecord(rec, rid);
  return status;
}

const Status RelCatalog::removeInfo(const string & relation)
{
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  Status        status;
  RID           rid;
  HeapFileScan  hfs = HeapFileScan(RELCATNAME, status);
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING,
                         relation.c_str(), EQ);
  if (status!=OK) {
    return status;
  }
  
  status = hfs.scanNext(rid);
  if (status == FILEEOF) {
    status = RELNOTFOUND;
  }
  
  if (status == OK) {
    status = hfs.deleteRecord();
  }
  
  if (status == NORECORDS) {
    return OK;
  }
  
  return status;
}


RelCatalog::~RelCatalog()
{
  // nothing should be needed here
}


AttrCatalog::AttrCatalog(Status &status) :
HeapFile(ATTRCATNAME, status)
{
  // nothing should be needed here
}


const Status AttrCatalog::getInfo(const string & relation,
                                  const string & attrName,
                                  AttrDesc &record)
{
  
  Status        status;
  RID           rid;
  Record        rec;
  HeapFileScan  hfs = HeapFileScan(RELCATNAME, status);
  bool          found = false;
  
  if (relation.empty() || attrName.empty()) {
    return BADCATPARM;
  }
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING,
                         relation.c_str(), EQ);
  if (status!=OK) {
    return status;
  }
  
  while ((status = hfs.scanNext(rid)) == OK) {
    status = hfs.getRecord(rec);
    if (status!=OK) {
      return status;
    }
    assert(sizeof(AttrDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
    if (string(record.attrName) == attrName) {
      found = true;
      break;
    }
  }
  
  if (status == FILEEOF) {
    status = ATTRNOTFOUND;
  }
  
  if (status!=OK) {
    return status;
  }
  
  return status;
}


const Status AttrCatalog::addInfo(AttrDesc & record)
{
  RID             rid;
  Status          status;
  InsertFileScan  ifs = InsertFileScan(RELCATNAME, status);
  
  if (status != OK) {
    return status;
  }
  
  int len = (int) strlen(record.relName);
  memset(&record.relName[len], 0, sizeof( record.relName) - len);
  len = (int) strlen(record.attrName);
  memset(&record.attrName[len], 0, sizeof( record.attrName) - len);
  
  Record rec;
  rec.data = &record;
  rec.length = sizeof(AttrDesc);
  status = ifs.insertRecord(rec, rid);
  return status;
}


const Status AttrCatalog::removeInfo(const string & relation,
                                     const string & attrName)
{
  if (relation.empty() || attrName.empty()) {
    return BADCATPARM;
  }
  
  Status        status;
  Record        rec;
  RID           rid;
  AttrDesc      record;
  HeapFileScan  hfs = HeapFileScan(RELCATNAME, status);
  
  if (status != OK) {
    return status;
  }
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING,
                         relation.c_str(), EQ);
  if (status != OK) {
    return status;
  }
  
  while ((status = hfs.scanNext(rid)) == OK) {
    status = hfs.getRecord(rec);
    if (status!=OK) {
      return status;
    }
    assert(sizeof(AttrDesc) == rec.length);
    memcpy(&record, rec.data, rec.length);
    if (string(record.attrName) ==  attrName) {
      status = hfs.deleteRecord();
      return status;
    }
  }
  
  if (status == FILEEOF) {
    status = RELNOTFOUND;
  }
  else if (status == NORECORDS) {
    status = OK;
  }
  
  return status;
}


const Status AttrCatalog::getRelInfo(const string & relation,
                                     int &attrCnt,
                                     AttrDesc *&attrs)
{
  
  if (relation.empty()) {
    return BADCATPARM;
  }
  
  Status       status;
  RID          rid;
  Record       rec;
  HeapFileScan hfs = HeapFileScan(RELCATNAME, status);
  
  if (status!=OK) {
    return status;
  }
  
  status = hfs.startScan(0, (int) relation.length() + 1, STRING,
                         relation.c_str(), EQ);
  if (status!=OK) {
    return status;
  }
  
  attrCnt = 0;
  while((status = hfs.scanNext(rid)) == OK) {
    status = hfs.getRecord(rec);
    if (status!= OK) {
      return status;
    }
    assert(sizeof(AttrDesc) == rec.length);
    attrCnt++;
    if (attrCnt == 1) {
      if (!(attrs = (AttrDesc*) malloc(sizeof(AttrDesc))))
        return INSUFMEM;
    }
    else {
      if (!(attrs = (AttrDesc*) realloc(attrs, attrCnt * sizeof(AttrDesc))))
        return INSUFMEM;
    }
    memcpy(&attrs[attrCnt - 1], rec.data, rec.length);
  }
  
  if (status == FILEEOF) {
    if (attrCnt == 0) {
      status = RELNOTFOUND;
    }
    else {
      status = OK;
    }
  }
  
  return status;
  
}


AttrCatalog::~AttrCatalog()
{
  // nothing should be needed here
}

