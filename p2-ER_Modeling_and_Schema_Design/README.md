# ER Modelling and Schema Design

## Overview

This project is part of a database course, designed to parse JSON files containing auction data and load them into a SQLite database. The main goal is to transform semi-structured data from eBay auctions into a structured relational database format. This project involves designing a relational schema, parsing the JSON data, and populating the database with the parsed data.

## Project Structure

The project consists of the following key files:

- `design.pdf`: Contains the Entity-Relationship (ER) diagram and relational schema definitions used for designing the database structure.
- `parser.py`: The main Python script responsible for parsing JSON files and generating SQLite load files.
- `create.sql`: SQL script to create tables in the SQLite database according to the defined schema.
- `load.txt`: Commands for bulk-loading data from `.dat` files into the SQLite database.
- `runParser.sh`: A shell script to automate the data parsing and loading process.
- `query1.sql` to `query7.sql`: SQL queries to test the correctness and efficiency of the database.

## Relational Schema

The relational schema is designed based on the ER diagram provided in `design.pdf`. It consists of the following tables:

1. **Item**:
   - `ItemID`: integer, Primary Key
   - `Name`: string, NOT NULL
   - `Currently`: string, NOT NULL
   - `Buy_Price`: string
   - `First_Bid`: string, NOT NULL
   - `Number_of_Bids`: integer, NOT NULL
   - `Location`: string, NOT NULL
   - `Country`: string, NOT NULL
   - `Started`: string, NOT NULL
   - `Ends`: string, NOT NULL
   - `Description`: string

2. **User**:
   - `UserID`: string, Primary Key, NOT NULL
   - `Rating`: real, NOT NULL
   - `Location`: string
   - `Country`: string

3. **Category**:
   - `Name`: string, Primary Key, NOT NULL
   - `ItemID`: integer, Primary Key, Foreign Key referencing `Item.ItemID`, NOT NULL

4. **Bid**:
   - `UserID`: string, Primary Key, Foreign Key referencing `User.UserID`, NOT NULL
   - `ItemID`: integer, Primary Key, Foreign Key referencing `Item.ItemID`, NOT NULL
   - `Time`: string, Primary Key, NOT NULL
   - `Amount`: string, NOT NULL

## Data Parsing and Loading Process

### Parsing JSON Data

The main script, `skeleton_parser.py`, is responsible for reading JSON files containing eBay auction data and converting them into `.dat` files that can be bulk-loaded into SQLite. The parser performs the following key tasks:

- **Date/Time Transformation**: Converts dates from the format `Mon-DD-YY HH:MM:SS` to `YYYY-MM-DD HH:MM:SS`.
- **Dollar Value Transformation**: Converts dollar amounts from strings like `$3,453.23` to `3453.23`.
- **File Handling**: Opens and processes each JSON file, writing the parsed data into corresponding `.dat` files (`category.dat`, `user.dat`, `bid.dat`, `item.dat`).

### Loading Data into SQLite

After parsing, the data is loaded into the SQLite database using the `create.sql` and `load.txt` scripts:

1. **Table Creation**: The `create.sql` script drops any existing tables and creates new ones according to the schema.
2. **Bulk Loading**: The `load.txt` script bulk-loads data from the `.dat` files into the corresponding tables in the SQLite database.

### Testing and Queries

The project includes seven SQL queries (`query1.sql` to `query7.sql`) that test the correctness of the data loaded into the database. These queries are designed to verify the integrity and efficiency of the database.