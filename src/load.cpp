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
  int attrCnt = 0;
  
  if (relation.empty() || fileName.empty() || relation == string(RELCATNAME)
      || relation == string(ATTRCATNAME))
    return BADCATPARM;
  
  // open Unix data file
  int fd;
  if ((fd = open(fileName.c_str(), O_RDONLY, 0)) < 0) {
    return UNIXERR;
  }
  
  // get relation data
  if ((status = relCat->getInfo(relation, rd)) != OK) return status;
  
  // get attribute data
  if ((status = attrCat->getRelInfo(rd.relName, attrCnt, attrs)) != OK)
    return status;
  
  // open data file
  InsertFileScan iFile = InsertFileScan(rd.relName, status);
  if (status!=OK) {
    return status;
  }
  
  // compute width of tuple and open index files, if any
  int width = 0;
  for(int i = 0; i < attrCnt; i++) {
    width += attrs[i].attrLen;
  }
  
  // create a record for constructing the tuple
  char *record;
  record = new char [width];
  if (!record) return INSUFMEM;
  
  int nbytes;
  Record rec;
  int records = 0;
  while((nbytes = (int) read(fd, record, width)) == width) {
    RID rid;
    rec.data = record;
    rec.length = width;
    status = iFile.insertRecord(rec, rid);
    if (status!=OK) {
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

