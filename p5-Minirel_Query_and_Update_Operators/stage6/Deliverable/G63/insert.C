#include "catalog.h"
#include "query.h"


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
// part 6
// <INSERT> ::= INSERT INTO relname (attr1, attr2,... attrN) VALUES (val1, val2,... valN)
// Example: insert into stars (real_name,soapid, starid, plays) values ("Bonarrigo, Laura", 3, 101, "Cassie");

Status status;
// get the attributes in order and their descriptions
AttrDesc* attrDescArr;
int attrCount = 0;
status = attrCat->getRelInfo(relation, attrCount, attrDescArr);
if (status != OK) { return status; }

// cout << "attrCount:" << attrCount << endl;
int reclen = 0;
for(int i = 0; i < attrCount; i++) {
	reclen += attrDescArr[i].attrLen;
}

// open the relation table
InsertFileScan insertRel(relation, status);
if (status != OK) { return status; }

// create output record for insertion
char outputData[reclen];
Record outputRec;
outputRec.data = (void *) outputData;
outputRec.length = reclen;

int outputOffset = 0;
for(int i = 0; i < attrCnt; i++){
	bool ifMatch = false;
	// Since the order of the attributes in attrList[] may not be the same as in the relation, 
	// you might have to rearrange them before insertion.
	for(int j = 0; j < attrCnt; j++){
		// i is the order of the attributes in the relation
		if(0 == strcmp(attrList[j].attrName, attrDescArr[i].attrName)){
			// we have a match, copy data into the output record
			if(attrList[j].attrValue == NULL){
				// If no value is specified for an attribute, you should reject the insertion 
				// as Minirel does not implement NULLs.
				// cout << "attrValue is NULL" << endl;
				return ATTRTYPEMISMATCH;
			}

			if (attrList[j].attrType != attrDescArr[i].attrType)
			{
				// cout << "attrDescArr[i].attrName:" << attrDescArr[i].attrName << endl;
				// cout << "attrDescArr[i].attrType:" << attrDescArr[i].attrType << endl;
				// cout << "attrList[j].attrType:" << attrList[j].attrType << endl;
				// cout << "attrDescArr[i].attrLen:" << attrDescArr[i].attrLen << endl;
				// cout << "attrList[j].attrLen:" << attrList[j].attrLen << endl;
				// cout << "attrList attrType mismatchs attrDescArr attrType" << endl;
				return ATTRTYPEMISMATCH;
			}

			int intVal;
			float floatVal;
			// Handle each value type (checked by the attrType member)
			switch(attrList[j].attrType)
			{
			case INTEGER:
				intVal = atoi((char *)attrList[j].attrValue);
				// copy the attribute value in attrList into the output record
				memcpy(outputData + outputOffset,(void *)&intVal, attrDescArr[i].attrLen);
				break;
			case FLOAT:
				floatVal = atof((char *)attrList[j].attrValue);
				memcpy(outputData + outputOffset, (void *)&floatVal, attrDescArr[i].attrLen);
				break;
			case STRING:
				memcpy(outputData + outputOffset, (char *)attrList[j].attrValue, attrDescArr[i].attrLen);
				break;
			}

			outputOffset += attrDescArr[i].attrLen;
			ifMatch = true;
		}
	}

	// missing some attributes in the insertion sql
	if(!ifMatch){
		return ATTRTYPEMISMATCH;
	}
}

// add the new record to the output relation
RID outRID;
status = insertRel.insertRecord(outputRec, outRID);
ASSERT(status == OK);

return OK;

}

