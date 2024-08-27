#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{
// part 6
// Example: delete from stars where stars.soapid = 1;

Status status;
// Construct a heapFileScan object using the relation name
HeapFileScan* hfs = new HeapFileScan(relation, status);
if (status != OK) { return status; }

// cout << "attrName:" << attrName << endl;
// cout << "attrValue:" << attrValue << endl;


if(attrName.empty()){
	// Handle cases when attrName is NULL
	status = hfs->startScan(0, 0, STRING, NULL, EQ);
	if (status != OK) { return status; }
} else {
	 // get AttrDesc structure for the filter attribute
    AttrDesc attrDesc;
    status = attrCat->getInfo(relation, attrName, attrDesc);
    if (status != OK) { return status; }

	// cout << "attrDesc.attrOffset:" << attrDesc.attrOffset << endl;
	// cout << "attrDesc.attrLen:" << attrDesc.attrLen << endl;
	// cout << "attrValue:" << attrValue << endl;

	int intVal;
	float floatVal;
	char* val;
	// Handle each value type (checked by the attrType member)
	switch(type)
	{
	case INTEGER:
		intVal = atoi(attrValue);
		val = (char*) &intVal;
		break;
	case FLOAT:
		floatVal = atof(attrValue);
		val = (char*) &floatVal;
		break;
	case STRING:
		val = strdup(attrValue);
		break;
	}
	// start the scan
	status = hfs->startScan(attrDesc.attrOffset, attrDesc.attrLen, type, val, op);
	if (status != OK) { return status; }
}

RID scanRID;
// scanNext() will return OK if there are unscanned records
// if OK, keep scanNext()
while (hfs->scanNext(scanRID) == OK)
{
	// delete the record
	hfs->deleteRecord();
}

hfs->endScan();
delete(hfs);

return OK;
}


