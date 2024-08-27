# Minirel Database Query and Update Operators

## Overview

This project is part of the CS 564 course and focuses on implementing query and update operations for the Minirel database system. The operations you will implement include selection, projection, insertion, and deletion using a simplified SQL-like syntax. The project involves processing commands such as `SELECT`, `INSERT`, and `DELETE` on database relations.

## Project Structure

The project consists of the following key files that you need to modify:

- `select.C`: Implements the `QU_Select` function for handling `SELECT` queries.
- `insert.C`: Implements the `QU_Insert` function for handling `INSERT` operations.
- `delete.C`: Implements the `QU_Delete` function for handling `DELETE` operations.

### Additional Files (Not to be Modified)
- `join.C`: Contains the implementation of the `QU_Join` function, but you are not required to modify this file.
- `catalog.h`, `query.h`, `catalog.C`, and `query.C`: These files contain supporting functions and class definitions necessary for the query and update operations.

## SQL DML Commands

The Minirel system implements a simplified version of SQL. The syntax for the Data Manipulation Language (DML) commands is as follows:

### Select Command

```sql
SELECT <TARGET_LIST> [into relname] FROM <TABLE_LIST> [WHERE <QUAL>];
```

- TARGET_LIST: Specifies the list of attributes to project.
- TABLE_LIST: Specifies the relation(s) from which to select.
- QUAL: Specifies the selection condition, which can be either a selection or a join.

### Insert Command
```sql
INSERT INTO relname (attr1, attr2, ..., attrN) VALUES (val1, val2, ..., valN);
```
- Inserts the specified values into the specified attributes of a relation.

### Delete Command
```sql
DELETE FROM relname [WHERE <SELECTION>];
```
- Deletes tuples that satisfy the specified selection condition.


## Implementation Details
### QU_Select

- Purpose: Handles the SELECT operation by performing a selection and/or projection on the specified relation and storing the result in a new relation.
- Process:
    - If a qualification (i.e., WHERE clause) is provided, a filtered HeapFileScan is used to retrieve the matching tuples.
    - The projection is performed on-the-fly as each result tuple is being appended to the result relation.
    - The result of the selection is stored in a relation specified by the result parameter.

### QU_Insert

- Purpose: Handles the INSERT operation by inserting a tuple with the given attribute values into the specified relation.
- Process:
    - The attributes are reordered based on the relation's schema before insertion to ensure the values are placed in the correct order.
    - The function rejects any insertion if a value is not provided for all attributes, as Minirel does not support NULL values.

### QU_Delete

- Purpose: Handles the DELETE operation by deleting all tuples in the specified relation that satisfy the predicate provided.
- Process:
    - A filtered HeapFileScan is used to locate the qualifying tuples based on the given condition (e.g., attrName, op, and attrValue).
    - The qualifying tuples are then deleted from the relation.