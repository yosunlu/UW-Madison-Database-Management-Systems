drop table if exists Item;
drop table if exists User;
drop table if exists Category;
drop table if exists Bid;

CREATE TABLE Item(
ItemID integer PRIMARY KEY NOT NULL,
Name char(60) NOT NULL,
Currently char(10)  NOT NULL,
Buy_Price char(10),
First_Bid char(10)  NOT NULL,
Number_of_Bids integer  NOT NULL,
Location char(100)  NOT NULL,
Country char(30)  NOT NULL,
Started char(30)  NOT NULL,
Ends char(30)  NOT NULL,
UserID char(50)  NOT NULL,
Description char(15000),
FOREIGN KEY(UserID) references User
);

CREATE TABLE User(
UserID CHAR(50) PRIMARY KEY NOT NULL,
Rating REAL(10) NOT NULL,
Location CHAR(30),
Country CHAR(10)
);

CREATE TABLE Bid(
UserID CHAR(50) NOT NULL,
ItemID integer NOT NULL,
Time char(30) NOT NULL,
Amount char(10) NOT NULL,
PRIMARY KEY (UserID, ItemID, Time),
FOREIGN KEY(UserID) references User,
FOREIGN KEY(ItemID) references Item
);

CREATE TABLE Category(
Name CHAR(40) NOT NULL,
ItemID INTEGER NOT NULL,
PRIMARY KEY(Name, ItemID),
FOREIGN KEY(ItemID) references Item
);