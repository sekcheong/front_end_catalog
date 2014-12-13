#include <unistd.h>
#include <fcntl.h>
#include "catalog.h"
#include "utility.h"


//
// Loads a file of (binary) tuples from a standard file into the relation.
// Any indices on the relation are updated appropriately.
//
// Returns:
// 	OK on success
// 	an error code otherwise
//

const Status UT_Load(const string & relation, const string & fileName)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;
  
  if (relation.empty() || fileName.empty() || relation == string(RELCATNAME)
      || relation == string(ATTRCATNAME))
    return BADCATPARM;
  
  // open Unix data file
  
  int fd;
  if ((fd = open(fileName.c_str(), O_RDONLY, 0)) < 0) {
    return UNIXERR;
  }
  
  // get relation data
  status = relCat->getInfo(relation, rd);
  if (status!=OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling relCat->getInfo()" << endl;
    return status;
  }
  
  // get attribute data
  status = attrCat->getRelInfo(rd.relName, attrCnt, attrs);
  if (status!=OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling attrCat->getRelInfo()" << endl;
    return status;
  }
  
  // open data file
  InsertFileScan iFile = InsertFileScan(rd.relName, status);
  if (status != OK) {
    cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling InsertFileScan()" << endl;
    return status;
  }
  
  int records = 0;
  int width = 0;
  for(int i = 0; i < attrCnt; i++) {
    width = width + attrs[i].attrLen;
  }
  
  // create a record for constructing the tuple
  char *record;
  if (!(record = new char [width])) return INSUFMEM;
  
  int len;
  Record rec;
  while((len = (int)read(fd, record, width)) == width) {
    RID rid;
    rec.data = record;
    rec.length = width;
    status = iFile.insertRecord(rec, rid);
    if (status!=OK) {
      cerr << __FILE__ << "(" << __LINE__ << ") " << "Error calling iFile.insertRecord()" << endl;
      return status;
    }
    records++;
  }
  
  cout << "Number of records inserted: " << records << endl;
  
  // close heap file and data file
  if (close(fd) < 0) return UNIXERR;
  
  delete [] record;
  free(attrs);
  
  return OK;
}