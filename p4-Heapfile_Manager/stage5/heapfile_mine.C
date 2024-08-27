#include "heapfile.h"
#include "error.h"

// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // try to open the file. This should return an error
    status = db.openFile(fileName, file);
    if (status != OK)
    {
		// file doesn't exist. First create it and allocate
		// an empty header page and data page.

        // init the file pointer
        status = db.createFile(fileName);
        ASSERT(status == OK);
        status = db.openFile(fileName, file);
        ASSERT(status == OK);
        
        // allocate an empty header page in the file
        status = bufMgr->allocPage(file, hdrPageNo, newPage);
        ASSERT(status == OK);
        hdrPage = (FileHdrPage*) newPage;
		
        // allocate an empty data page in the file
        status = bufMgr->allocPage(file, newPageNo, newPage);
        ASSERT(status == OK)
        newPage->init(newPageNo);

        // init attributes of the header page
        strncpy(hdrPage->fileName, fileName.c_str(), MAXNAMESIZE);
        hdrPage->fileName[MAXNAMESIZE - 1] = '\0';
        hdrPage->firstPage = newPageNo;
        hdrPage->lastPage = newPageNo;
        hdrPage->pageCnt = 1;
        hdrPage->recCnt = 0;		

        // Clean up (Unpin both pages, close the file)
        status = bufMgr->unPinPage(file, hdrPageNo, true);
        ASSERT(status == OK);
        status = bufMgr->unPinPage(file, newPageNo, true);
        ASSERT(status == OK);
        status = db.closeFile(file);
        ASSERT(status == OK);

        return OK;

    }
    return (FILEEXISTS);
}

// routine to destroy a heapfile
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;

    cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
        // get the page number (within the file) of the first page, and assign it to headerPageNo 
		status = filePtr->getFirstPage(headerPageNo);
        ASSERT(status == OK);

		// read the header page into the buffer
        // pagePtr returns the pointer to a page in bufPool (now the header page)
        status = bufMgr->readPage(filePtr, headerPageNo, pagePtr);
        ASSERT(status == OK);

        // init the headerPage
        // headerPage is a protected pointer to a FileHdrPage struct
        // assign pagePtr (a pointer to the page in the bufool) to headerPage
		// headerPage = (FileHdrPage)* pagePtr; ????
        headerPage = reinterpret_cast<FileHdrPage*>(pagePtr);
        hdrDirtyFlag = false;
        
        // read the first data page into the buffer
        // pagePtr returns the pointer to a page in bufPool (now the first data page)
        curPageNo = headerPage->lastPage;
        status = bufMgr->readPage(filePtr, curPageNo, curPage);
        ASSERT(status == OK);

        curDirtyFlag = false;

        returnStatus = OK;
        curRec = NULLRID;

        return;
    }
    else
    {
    	cerr << "open of heap file failed\n";
		returnStatus = status;
		return;
    }
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of date page\n";
    }
	
	 // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
	
	// status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
	// if (status != OK) cerr << "error in flushFile call\n";
	// before close the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;
    Page* pagePtr;
    // case 1: current page is not the record's page
    if((curPageNo != rid.pageNo) || curPage == NULL){
        // unpin the current page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag); // why?
        if (status != OK) return status;

        // read the record's page
        status = bufMgr->readPage(filePtr, rid.pageNo, curPage);
        if (status != OK) return status;

        // update the heapfile object
        curPageNo = rid.pageNo;
    }
    // case 2: current page is the record's page
    status = curPage->getRecord(rid, rec);
    if (status != OK) return status;
    
    // update HeapFile object
    curRec = rid;
    return status;
    // cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;
   
}

HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_, 
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        (type_ == INTEGER && length_ != sizeof(int)
         || type_ == FLOAT && length_ != sizeof(float)) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }
    return OK;
}

HeapFileScan::~HeapFileScan()
{
    endScan();
}

const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}

const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}


const Status HeapFileScan::scanNext(RID& outRid)
{
    Status 	status = OK;
    RID		nextRid;
    RID		tmpRid;
    int 	nextPageNo;
    Record      rec;


    // check if curPage is null; if yes start from the first page
    if(curPage == NULL){
        curPageNo = headerPage->firstPage;
        status = bufMgr->readPage(filePtr, curPageNo, curPage);
        if(status != OK) return status;

        status = curPage->firstRecord(tmpRid);
        if(status != OK) return status;

        nextRid = tmpRid; // nextRid here is the first RID to be checked
    }else{
        nextRid = curRec;
    }

    while(1){
        // loop within page
        do{
            tmpRid = nextRid;
            status = curPage->getRecord(tmpRid, rec); // rec: pointer to the recored tmpRid is reffering to
            if(status != OK) return status;
            
            if( matchRec(rec) == true){
                outRid = tmpRid;
                curRec = outRid;
                return status;
            }
        } while ((curPage->nextRecord(tmpRid, nextRid)) != ENDOFPAGE);
        
        // reached ENDOFPAGE of the last page
        if(curPageNo == headerPage->lastPage){
            return FILEEOF;
        }

        // Before moving to next page, do cleanup
        // Unpin current page
        bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);

        // read the next page; first get next page's number
        status = curPage->getNextPage(nextPageNo);
        if(status != OK) return status;
        
        // actually read the page into buffer 
        curPageNo = nextPageNo;
        status = bufMgr->readPage(filePtr, curPageNo, curPage);
        if(status != OK) return status;

        // start from the first RID of the page
        status = curPage->firstRecord(nextRid);
        if(status != OK) return status;
    }
}


// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}

const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}

InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}

// Insert a record into the file
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    Page*   lastPage;
    int		newPageNo;
    Status	status;
    // RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    // set curpage to last page is curpage is null
    if((curPage == NULL)){

        status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
        if(status != OK) return status;
        
        // bookkeeping the header page
        curPageNo = headerPage->lastPage;
    }
    
    // allocate new page if the current page is full, then insert
    if(curPage->insertRecord(rec, outRid) == NOSPACE){

    // allocate new page and set it to last page
        // read the last page
        status = bufMgr->readPage(filePtr, headerPage->lastPage, lastPage);
        if(status != OK) return status;

        // allocate a page in the buffer
        status = bufMgr->allocPage(filePtr, newPageNo, newPage);
        if(status != OK) return status;

        newPage->init(newPageNo);
        lastPage->setNextPage(newPageNo);

        // update the headerPage's attributes that were effected by adding a new page
        headerPage->lastPage = newPageNo;
        headerPage->pageCnt++; 

    // unpin old current page (now still at the full page), and insert to new curPage
        // unpin the current page 
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if(status != OK) return status;

        // bookkeeping the header page
        curPage = newPage;
        curPageNo = newPageNo;

        status = curPage->insertRecord(rec, outRid);    
    }

    ASSERT(status == OK);
    headerPage->recCnt++;
    hdrDirtyFlag = true;
    curDirtyFlag = true;
    curRec = outRid;

    return status;
}


