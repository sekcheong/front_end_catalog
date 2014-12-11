#ifndef DB_H
#define DB_H

#include <sys/types.h>
#include <functional>
#include "error.h"
#include <string.h>
#include <assert.h>
using namespace std;

// define if debug output wanted

//#define DEBUGIO
//#define DEBUGFREE

// forward class definition for db
class DB;

// class definition for open files
class File {
  
  friend class DB;
  friend class OpenFileHashTbl;
  
public:
  // allocate a new page
  Status allocatePage(int& pageNo);
  
  // release space for a page
  const Status disposePage(const int pageNo);
  
  // read page from file
  const Status readPage(const int pageNo,
                        Page* pagePtr) const;

  // write page to file
  const Status writePage(const int pageNo,
                         const Page* pagePtr);
  
  // returns pageNo of first page
  const Status getFirstPage(int& pageNo) const;
  
  bool operator == (const File & other) const {
    return fileName == other.fileName;
  }
  
private:
  
  // initialize
  File(const string &fname);
  
   // deallocate file object
  ~File();
  
  static const Status create(const string &fileName);
  static const Status destroy(const string &fileName);
  
  const Status open();
  const Status close();
  
  // internal file read
  const Status intread(const int pageNo,
                       Page* pagePtr) const;
  
  // internal file write
  const Status intwrite(const int pageNo,
                        const Page* pagePtr);
  
#ifdef DEBUGFREE
  void listFree();    // list free pages
#endif
  
  string fileName;    // The name of the file
  int    openCnt;     // # times file has been opened
  int    unixFile;    // unix file stream for file
};

class BufMgr;
extern BufMgr* bufMgr;

// declarations for hash table of open files
struct fileHashBucket {
  string          fname;    // name of the file
  File*           file;     // pointer to file object
  fileHashBucket* next;	    // next node in the hash table
  
};

// hash table to keep track of open files
class OpenFileHashTbl {
public:
  OpenFileHashTbl();
  ~OpenFileHashTbl(); // destructor
  
  // returns OK if no error occured, HASHTBLERROR if an error occurred
  Status insert(const string fileName, File* file);
  
  // see if fileName is already in hash table.  If so a pointer to the file
  // object is returned.
  // returns OK if found. else returns HASHNOTFOUND
  Status find(const string fileName, File*& file);
  
  // returns OK if fileName was found.  Else return HASHTBLERROR
  Status erase(const string fileName);
  
private:
  int HTSIZE;
  fileHashBucket**  ht; // actual hash table
  int	 hash(string fileName);  // returns value between 0 and HTSIZE-1
};



class DB {
public:
  DB();      // initialize open file table
  ~DB();     // clean up any remaining open files
  
  // create a new file
  const Status createFile(const string & fileName) ;
  
  // destroy a file, release all space
  const Status destroyFile(const string & fileName) ;
  
  // open a file
  const Status openFile(const string & fileName, File* & file);
  
  // close a file
  const Status closeFile(File* file);
  
private:
  OpenFileHashTbl openFiles;    // list of open files
};


// structure of DB (header) page

typedef struct {
  int nextFree;                // page # of next page on free list
  int firstPage;               // page # of first page in file
  int numPages;                // total # of pages in file
} DBPage;

#endif