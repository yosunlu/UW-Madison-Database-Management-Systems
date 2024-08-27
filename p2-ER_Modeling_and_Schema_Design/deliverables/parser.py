"""
FILE: skeleton_parser.py
------------------
Author: Firas Abuzaid (fabuzaid@stanford.edu)
Author: Perth Charernwattanagul (puch@stanford.edu)
Modified: 04/21/2014

Skeleton parser for CS564 programming project 1. Has useful imports and
functions for parsing, including:

1) Directory handling -- the parser takes a list of eBay json files
and opens each file inside of a loop. You just need to fill in the rest.
2) Dollar value conversions -- the json files store dollar value amounts in
a string like $3,453.23 -- we provide a function to convert it to a string
like XXXXX.xx.
3) Date/time conversions -- the json files store dates/ times in the form
Mon-DD-YY HH:MM:SS -- we wrote a function (transformDttm) that converts to the
for YYYY-MM-DD HH:MM:SS, which will sort chronologically in SQL.

Your job is to implement the parseJson function, which is invoked on each file by
the main function. We create the initial Python dictionary object of items for
you; the rest is up to you!
Happy parsing!
"""

import sys
from json import loads
from re import sub
import time


columnSeparator = "|"
userdict = {}

# Dictionary of months used for date transformation
MONTHS = {
    "Jan": "01",
    "Feb": "02",
    "Mar": "03",
    "Apr": "04",
    "May": "05",
    "Jun": "06",
    "Jul": "07",
    "Aug": "08",
    "Sep": "09",
    "Oct": "10",
    "Nov": "11",
    "Dec": "12",
}

"""
Returns true if a file ends in .json
"""


def isJson(f):
    return len(f) > 5 and f[-5:] == ".json"


"""
Converts month to a number, e.g. 'Dec' to '12'
"""


def transformMonth(mon):
    if mon in MONTHS:
        return MONTHS[mon]
    else:
        return mon


"""
Transforms a timestamp from Mon-DD-YY HH:MM:SS to YYYY-MM-DD HH:MM:SS
"""


def transformDttm(dttm):
    dttm = dttm.strip().split(" ")
    dt = dttm[0].split("-")
    date = "20" + dt[2] + "-"
    date += transformMonth(dt[0]) + "-" + dt[1]
    return date + " " + dttm[1]


"""
Transform a dollar value amount from a string like $3,453.23 to XXXXX.xx
"""


def transformDollar(money):
    if money == None or len(money) == 0:
        return money
    return sub(r"[^\d.]", "", money)


"""
Parses a single json file. Currently, there's a loop that iterates over each
item in the data set. Your job is to extend this functionality to create all
of the necessary SQL tables for your database.
"""


def parseJson(json_file):
    with open(json_file, "r") as f:
        start = time.time()
        items = loads(f.read())[
            "Items"
        ]  # creates a Python dictionary of Items for the supplied json file
        if json_file == "items-0.json":
            # create category.dat
            category_file = open("category.dat", "w")
            # create user.dat
            # user_file = open("user.dat", "w")
            # create bid.dat
            bid_file = open("bid.dat", "w")
            # create time.dat
            item_file = open("item.dat", "w")
        else:
            # append category.dat
            category_file = open("category.dat", "a")
            # append user.dat
            # user_file = open("user.dat", "a")
            # append bid.dat
            bid_file = open("bid.dat", "a")
            # append item.dat
            item_file = open("item.dat", "a")

        # descLen = 0;
        # locLen = 0;
        # countryLen = 0;
        # nameLen = 0;
        # catLen = 0;
        # useridLen = 0
        for item in items:
            """
            TODO: traverse the items dictionary to extract information from the
            given `json_file' and generate the necessary .dat files to generate
            the SQL tables based on your relation design
            """
            # Item(ItemID, Name, Currently, Buy_Price, First_Bid, Number_of_Bids, Location, Country, Started, Ends, Description)
            # Buy_Price can be null
            # 1045480969|DISNEY PINOCCHIO COOKIE JAR - FAST SHIPPING!|$32.95|$96.57|$32.95|0|Northwest Suburbs|USA|Dec-07-01 21:03:44|Dec-14-01 21:03:44|desc
            # 1045480969|DISNEY PINOCCHIO COOKIE JAR - FAST SHIPPING!|32.95|96.57|32.95|0|Northwest Suburbs|USA|2001-12-07 21:03:44|2001-12-14 21:03:44|desc
            seller = item["Seller"]
            item_file.write(
                item["ItemID"]
                + columnSeparator
                + ('"' + item["Name"].replace('"', '""') + '"')
                + columnSeparator
                + (transformDollar(item["Currently"]))
                + columnSeparator
                + (
                    transformDollar(item["Buy_Price"])
                    if "Buy_Price" in item
                    else "NULL"
                )
                + columnSeparator
                + (transformDollar(item["First_Bid"]))
                + columnSeparator
                + item["Number_of_Bids"]
                + columnSeparator
                + ('"' + item["Location"].replace('"', '""') + '"')
                + columnSeparator
                + ('"' + item["Country"].replace('"', '""') + '"')
                + columnSeparator
                + (transformDttm(item["Started"]))
                + columnSeparator
                + (transformDttm(item["Ends"]))
                + columnSeparator
                + ('"' + seller["UserID"].replace('"', '""') + '"')
                + columnSeparator
                + (
                    ('"' + item["Description"].replace('"', '""') + '"')
                    if item["Description"] is not None
                    else "NULL"
                )
                + "\n"
            )

            # if item["Description"] is not None :
            #     if len(item["Description"]) > descLen:
            #         descLen = len(item["Description"])
            # if item["Location"] is not None :
            #     if len(item["Location"]) > locLen:
            #         locLen = len(item["Location"])
            # if item["Country"] is not None :
            #     if len(item["Country"]) > countryLen:
            #         countryLen = len(item["Country"])
            # if item["Name"] is not None :
            #     if len(item["Name"]) > nameLen:
            #         nameLen = len(item["Name"])

            # Category(Name, ItemID)
            # Collectibles|1043374545
            cat_list = []
            categories = item["Category"]
            for category in categories:
                if category not in cat_list:
                    category_file.write(
                        ('"' + category.replace('"', '""') + '"')
                        + columnSeparator
                        + item["ItemID"]
                        + "\n"
                    )
                    cat_list.append(category)
                # if category is not None :
                #     if len(category) > catLen:
                #         catLen = len(category)

            # User(UserID, Rating, Location, Country)
            # goldcoastvideo|2919|Los Angeles,CA|USA
            # bids
            bids = item["Bids"]
            if bids is not None:
                for bid in bids:
                    a_bid = bid["Bid"]
                    # bidder
                    bidder = a_bid["Bidder"]
                    # if len(bidder["UserID"]) > useridLen:
                    #     useridLen = len(bidder["UserID"])
                    if bidder["UserID"] not in userdict:
                        userdict[bidder["UserID"]] = {
                            "UserID": bidder["UserID"],
                            "Rating": bidder["Rating"],
                            "Location": bidder["Location"]
                            if "Location" in bidder
                            else "NULL",
                            "Country": bidder["Country"]
                            if "Country" in bidder
                            else "NULL",
                        }
                    else:
                        if "Location" in bidder:
                            userdict[bidder["UserID"]]["Location"] = bidder["Location"]
                        if "Country" in bidder:
                            userdict[bidder["UserID"]]["Country"] = bidder["Country"]

                    # Bid(UserID, ItemID, Time, Amount)
                    # Glen|1045460719|Dec-10-01 11:22:45|$48.38 -> Glen|1045460719|2001-12-10 11:22:45|48.38
                    bid_file.write(
                        ('"' + bidder["UserID"].replace('"', '""') + '"')
                        + columnSeparator
                        + item["ItemID"]
                        + columnSeparator
                        + (transformDttm(a_bid["Time"]))
                        + columnSeparator
                        + (transformDollar(a_bid["Amount"]))
                        + "\n"
                    )

            # if len(seller["UserID"]) > useridLen:
            #     useridLen = len(seller["UserID"])
            if seller["UserID"] not in userdict:
                userdict[seller["UserID"]] = {
                    "UserID": seller["UserID"],
                    "Rating": seller["Rating"],
                    "Location": item["Location"],
                    "Country": item["Country"],
                }
            else:
                userdict[seller["UserID"]]["Location"] = item["Location"]
                userdict[seller["UserID"]]["Country"] = item["Country"]

            pass

        category_file.close()
        bid_file.close()
        item_file.close()
        end = time.time()
        # print("execution time: " + str(end - start) + "s")
        # print("desc len:" + str(descLen))
        # print("loc len:" + str(locLen))
        # print("name len:" + str(nameLen))
        # print("country len:" + str(countryLen))
        # print("cat len:" + str(catLen))
        # print("userid len:" + str(useridLen))


"""
Loops through each json files provided on the command line and passes each file
to the parser
"""


def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, "Usage: python skeleton_json_parser.py <path to json files>"
        sys.exit(1)
    # loops over all .json files in the argument
    for f in argv[1:]:
        if isJson(f):
            parseJson(f)
            print("Success parsing " + f)

    # create user.dat
    user_file = open("user.dat", "w")
    for x in userdict.values():
        user_file.write(
            ('"' + x["UserID"].replace('"', '""') + '"')
            + columnSeparator
            + x["Rating"]
            + columnSeparator
            + ('"' + x["Location"].replace('"', '""') + '"')
            + columnSeparator
            + ('"' + x["Country"].replace('"', '""') + '"')
            + "\n"
        )
    user_file.close()


if __name__ == "__main__":
    main(sys.argv)
