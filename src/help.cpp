#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for the
// user. If no relation is given (relation is NULL), then it lists all
// the relations in the database, along with the width in bytes of the
// relation, the number of attributes in the relation, and the number of
// attributes that are indexed.  If a relation is given, then it lists
// all of the attributes of the relation, as well as its type, length,
// and offset, whether it's indexed or not, and its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rel;
  AttrDesc *attrs;
  int attrCnt;
  
  if (relation.empty()) {
    return UT_Print(RELCATNAME);
  }
  
  status = getInfo(relation, rel);
  if (status!=OK) {
    return status;
  }
  
  status = attrCat->getRelInfo(relation, attrCnt, attrs);
  if (status!=OK) {
    return status;
  }
  
  cout << "Relation name: " << rel.relName << endl
       << "Number of attributes: " << rel.attrCnt << endl;
  
  printf("Attribute Name     Offset     Type    Length\n");
  for (int i = 0; i < attrCnt; i++) {
    Datatype t = (Datatype) attrs[i].attrType;
   
    string typeName;
    switch (t) {
      case INTEGER:
        typeName = "int";
        break;
      case FLOAT:
        typeName = "float";
      default:
        typeName = "string";
        break;
    }
    
    printf("%14s  %9d   %6s  %8d\n",  attrs[i].attrName,  attrs[i].attrOffset,  typeName.c_str(),  attrs[i].attrLen);
  }
  
  free(attrs);
  
  return OK;
}