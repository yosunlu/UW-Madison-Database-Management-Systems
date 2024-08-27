# Minirel Database Buffer Manager

## Overview

This project is a part of the CS 564 course, where we implement various components of a miniature database management system (DBMS), called Minirel. This specific stage involves developing a buffer manager, which is a critical component for managing memory in database systems. The buffer manager handles the in-memory pages of data that have been read from disk, applying a page replacement policy to efficiently manage which pages remain in memory.

## Project Structure

The project contains the following key files:

- `buf.h`: Class definitions for the buffer manager.
- `buf.C`: Implementations of the buffer manager methods.
- `bufHash.C`: Implementation of the buffer pool hash table class.
- `db.h`: Class definitions for the DB and File classes.
- `db.C`: Implementations of the DB and File classes.
- `page.h`: Class definition of the page class.
- `page.C`: Implementation of the page class.
- `error.h`: Error codes and error class definition.
- `error.C`: Error class implementation.
- `testbuf.C`: Test driver for the buffer manager.

## Buffer Manager

### Functionality

The buffer manager is responsible for managing the buffer pool, which consists of a fixed number of frames (pages) that hold database pages read from disk. The key functionalities implemented in this project are:

1. **Page Replacement (Clock Algorithm)**:
   - Implements the clock algorithm to manage which pages should be evicted from the buffer pool when a new page needs to be loaded.

2. **Buffer Pool Management**:
   - Handles allocation, reading, and writing of pages between disk and buffer pool.
   - Tracks the state of each buffer pool frame using `BufDesc` class, which includes information like the number of pins, whether the page is dirty, valid, etc.

3. **Hash Table**:
   - Uses a hash table (`BufHashTbl` class) to efficiently map (File, PageNo) pairs to the buffer pool frames that hold them.

### Key Methods

- `BufMgr::allocBuf(int &frame)`: Allocates a free frame using the clock algorithm. Writes a dirty page back to disk if necessary.
- `BufMgr::readPage(File* file, const int PageNo, Page*& page)`: Reads the specified page, first checking if it's already in the buffer pool.
- `BufMgr::unPinPage(File* file, const int PageNo, const bool dirty)`: Unpins the specified page, decreasing its pin count.
- `BufMgr::allocPage(File* file, int& PageNo, Page*& page)`: Allocates a free page and an empty buffer in the buffer pool.