/*
Group member:

HUAI-DAN CHANG
908 466 1785

YUSHAN LU
908 476 1288

JHIHFAN LIN
908 493 4349
*/

#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"

#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;

    bufTable = new BufDesc[bufs];
    memset(bufTable, 0, bufs * sizeof(BufDesc));
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}


BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}

/*
Allocates a free frame using the clock algorithm; 
if necessary, writes a dirty page back to disk. 

Parameter:
- frame: reference to the # of frame to be allocated

Return:
- frame number of the newly allocated frame to the caller via the frame parameter
- OK if no errors occured
- BUFFEREXCEEDED if all buffer frames are pinned
- UNIXERR if the call to the I/O layer returned an error when a dirty page was being written to disk
*/

const Status BufMgr::allocBuf(int & frame) 
{
    int count = 0;
    while(count < numBufs*2-1){

        advanceClock();
        BufDesc* curBuf = &bufTable[clockHand];
        
        if (!(curBuf->valid)){ // The frame is not valid (empty) break and return
            frame = clockHand; 
            return OK;
        }
        
        if(curBuf->refbit == true){ // skip the frame if the refbit is set
            curBuf->refbit = false;
            count++;
            continue;
         }

        if(curBuf->pinCnt != 0){ // skip the frame if the page is pinned 
            count++;
            continue;
         }
        
        // the frame can be used; if dirty, write to disk
        if(curBuf->dirty == true){
            Status status = curBuf->file->writePage(curBuf->pageNo, &bufPool[clockHand]);                      
            if(status != OK)
                return UNIXERR;
         }
        
        // remove the appropriate entry from the hash table
        hashTable->remove(curBuf->file, curBuf->pageNo);
        frame = clockHand; // return the frame number
        return OK;
    }

    return BUFFEREXCEEDED; 
}
	

/*
Reads the specified page. 
First checks whether the page is already in the buffer pool.
If not, call allocbuf(). If yes, sets the appropriate refbit and increments the pinCnt for the page

Parameter:
- file: the file that the page will read from
- PageNo: the # of the page to be read
- page: reference to the pointer of the page in the buffer pool

Return:
- a pointer to the frame containing the page via the page parameter
- OK if no errors occured
- UNIXERR if a Unix error occurred
- BUFFEREXCEEDED if all buffer frames are pinned
- HASHTBLERROR if a hash table error occurred
*/
const Status BufMgr::readPage(File* file, const int pageNo, Page*& page)
{

    int frameNo = 0;
    // see if it is in the buffer pool
    Status status = hashTable->lookup(file, pageNo, frameNo);
    
    // Case 2: The page is in the buffer pool.
    // set the appropriate refbit
    if(status == OK){
        bufTable[frameNo].refbit = true;
        bufTable[frameNo].pinCnt++;
    }

    else if(status == HASHNOTFOUND){
        // Case 1: The page is not in the buffer pool.
        if((status = allocBuf(frameNo)) != OK) // allocate a frame for the page
            return status; 
        
        if((status = file->readPage(pageNo, &bufPool[frameNo])) != OK)  // read the page from the disk into the buffer pool frame
            return status;
            
        if((status = hashTable->insert(file, pageNo, frameNo)) != OK) // // insert the page into the hashtable
            return status;
        
        bufTable[frameNo].Set(file, pageNo); // invoke Set() to leave the pinCnt for the page to 1
    }
    
    // return a pointer to the frame containing the page via the page parameter.
    page = &bufPool[frameNo];
    return OK;
}

/*
Unpins the specified page
Parameter:
- file: the file that the page will be removed from
- PageNo: the # of the page to be removed
- dirty: whether the page is dirty

Return:
- OK if no errors occured
- HASHNOTFOUND if the page is not in the buffer pool hash table
- PAGENOTPINNED if the pin count is already 0
*/
const Status BufMgr::unPinPage(File* file, const int PageNo, const bool dirty) 
{
    int frameNo = 0;
    Status status = hashTable->lookup(file, PageNo, frameNo);
    
    if(status == OK){
        
        if (dirty)  // if the specified page is dirty, set the dirty bit
            bufTable[frameNo].dirty = dirty;

        if(bufTable[frameNo].pinCnt == 0) // if the pin count of the buffer is not 0, return error
            return PAGENOTPINNED;
        
        bufTable[frameNo].pinCnt --; // decrement the pincount
    }

    return status;
}

/*
Allocates a free page to the file and an empty buffer in the bufferpool;
Parameter:
- file: the file that the free page will be added to
- PageNo: reference to the # of page to be added
- page: reference to the pointer of the page in the buffer pool

Return:
- page number of the newly allocated page to the caller via the pageNo parameter
- a pointer to the buffer frame allocated for the page via the page parameter
- OK if no errors occured
- UNIXERR if a Unix error occurred
- BUFFEREXCEEDED if all buffer frames are pinned 
- HASHTBLERROR if a hash table error occurred

*/
const Status BufMgr::allocPage(File* file, int& PageNo, Page*& page) 
{
    
    Status status;
    if((status = file->allocatePage(PageNo)) != OK) // allocate an empty page in the specified file 
        return status;
    
    int frameNo = 0;
    if((status = allocBuf(frameNo)) != OK) // allocBuf() is called to obtain a buffer pool frame
        return status;
    
    // an entry is inserted into the hash table and Set() is invoked on the frame to set it up properly
    if((status = hashTable->insert(file, PageNo, frameNo)) != OK)
        return status;    

    bufTable[frameNo].Set(file, PageNo);
    page = &bufPool[frameNo];
    
    return OK;
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


