#include "stdio.h"
#include "stdlib.h"
#include "catalog.h"
#include "query.h"

// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
	// Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;
	Status status;

	// go through the projection list and look up each in the 
    // attr cat to get an AttrDesc structure (for offset, length, etc)
	int reclen = 0;
    AttrDesc attrDescArray[projCnt];
    for (int i = 0; i < projCnt; i++)
    {
       status = attrCat->getInfo(projNames[i].relName,
                                         projNames[i].attrName,
                                         attrDescArray[i]);
       if (status != OK) { return status; }
	   reclen += attrDescArray[i].attrLen;
    }

	AttrDesc attrDesc;
	if(attr != NULL){
		// turn attrInfo into AttrDesc
    	status = attrCat->getInfo(projNames[0].relName, attr->attrName, attrDesc);
		if (status != OK) { return status; }
		status = ScanSelect(result, projCnt, attrDescArray, &attrDesc, op, attrValue, reclen);
	} else {
		// If attr is NULL, an unconditional scan of the input table should be performed.
		status = ScanSelect(result, projCnt, attrDescArray, nullptr, op, attrValue, reclen);
	}

	if (status != OK) { return status; }

	return OK;
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
	// Construct a heapFileScan object using the relation name
	HeapFileScan* hfs = new HeapFileScan(projNames[0].relName, status);
	if (status != OK) { return status; }
	
	int intVal;
	float floatVal;
	char* val;
	if (attrDesc == NULL) {
		// If attrDesc is NULL, an unconditional scan of the input table should be performed.
		status = hfs->startScan(0 , 0, STRING, NULL, EQ);
	} else {
		// Handle each value type (checked by the attrType member)
		switch((Datatype) attrDesc->attrType)
		{
		case INTEGER:
			intVal = atoi(filter);
			val = (char*) &intVal;
			break;
		case FLOAT:
			floatVal = atof(filter);
			val = (char*) &floatVal;
			break;
		case STRING:
			val = strdup(filter);
			break;
		}

		// start the scan with attrOffset, attrLen and attrType stored in the attrDesc
		// val is from filter and already be converted to the proper type based on the attrType
		status = hfs->startScan(attrDesc->attrOffset, attrDesc->attrLen, (Datatype)attrDesc->attrType,
									val, op);
	} 

	if (status != OK) { return status; }


	// open the relation table to be inserted
	InsertFileScan insertRel(result, status);
	if (status != OK) { return status; }


	RID scanRID;
	// scanNext() will return OK if there are unscanned records
	// if OK, keep scanNext()
	while (hfs->scanNext(scanRID) == OK) {
		// get the record that satisfies the predicate
		Record scanRec;
		status = hfs->getRecord(scanRec);
		if (status != OK) { return status; }

		// record to be inserted
		char outputData[reclen];
		Record outputRec;
		outputRec.data = (void *) outputData;
		outputRec.length = reclen;

		int outputOffset = 0;
		for(int j = 0; j < projCnt; j++){
			// copy each attribute in the record into outputRec
			memcpy(outputData + outputOffset, (char *)scanRec.data + projNames[j].attrOffset, projNames[j].attrLen);
			outputOffset += projNames[j].attrLen;
		}

		// add the new record to the output relation
		RID outRID;
		status = insertRel.insertRecord(outputRec, outRID);
		if (status != OK) { return status; }
	}

	hfs->endScan();
	delete(hfs);

	return OK;
}
