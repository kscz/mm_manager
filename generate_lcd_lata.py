#!/usr/bin/env python3
# www.github.com/hharte/mm_manager
#
# (c) 2020-2022, Howard M. Harte
#
# This script generates LCD tables for the Nortel Millennium Payphone.
# It generates the LCD tables by looking up information from
# localcallingguide.com to determine the LATA, given an NPA-NXX.
# This LATA is used and each exchange in the LATA is checked to
# determine which NPA-NXX are local.  All of these NPA-NXX are stored
# in a dictionary, and are considered Intra-LATA Toll.  Finally, the
# exchange of the specified NPA-NXX is used to determine which calls
# are actually local to the payphone.
#
# The first time this script is executed for a given LATA, the LATA
# data is downloaded and stored as a .csv.  Subsequent runs for the
# same LATA will use this cached data, unless it is older than a
# specified number of days.
#
# Tables 136-155 (0x88-0x9b)
#
# The LCD Table is an array of 202 bytes.  The first two bytes contain
# the NPA followed by 'e', for example, NPA 408 would be represented as
# 0x40 0x8e.
#
# The remaining 200 bytes represent two bits for each NXX in the range
# of 200-999.
#
# These two bits encode a value as follows:
# 0 - Local Rate
# 1 - ?
# 2 - Intra-LATA
# 3 - Invalid (ie, the N11, the NPA itself.)
#
# usage: generate_lcd_lata.py [-h] [--debug DEBUG] [--country COUNTRY]
#                        [--npa NPA] [--nxx NXX]
#
# For example, to generate LCD tables for the San Jose, California, USA rate center:
# ./generate_lcd.py --npa 408 --nxx 202
#
# Canada is not supported yet, because all of Canada is LATA 888

import argparse
import array
import csv
import requests
import sys
import time
import xmltodict

from os import path

def is_file_older_than_x_days(file, days=1):
    file_time = path.getmtime(file)
    # Check against 24 hours
    if (time.time() - file_time) / 3600 > 24*days:
        return True
    else:
        return False

parser = argparse.ArgumentParser()
parser.add_argument("--npa", help="Terminal's NPA", required=True)
parser.add_argument("--nxx", help="Terminal's NXX", required=True)
parser.add_argument("--debug", help="display debug output")
parser.add_argument("--age", help="age in days of LATA data before refresh", default=14)

args = parser.parse_args()

print("LCD Table Generator for the Nortel Millennium Payphone")
print("(c) 2020-2022, Howard M. Harte\n")

my_npa = args.npa
my_nxx = args.nxx
lata_age_max = int(args.age)    # Maximum age of cached LATA data in days

print("Generating LCD tables for " + str(my_npa) + "-" + str(my_nxx))

r = requests.get("https://localcallingguide.com/xmlprefix.php?npa=" + str(my_npa) + "&nxx=" + str(my_nxx)).text
data = xmltodict.parse(r)['root']['prefixdata']

local_exch = data['exch']
lata = data['lata']
latacsvname = "lata-" + lata + ".csv"

if lata == 888:
    print("Error: Canada is not supported.")
    sys.exit()

print("Exchange: " + local_exch + " LATA: " + lata)

npa_dict = {}
npanxx_dict = {}

if path.exists(latacsvname) and is_file_older_than_x_days(latacsvname, days=lata_age_max) == False:

    print(latacsvname + " exists and is not older than " + str(lata_age_max) + " days, skipping generation")

    with open(latacsvname, mode='r') as latafile:
        reader = csv.reader(latafile)
        next(reader, None)  # Skip CSV header
        for rows in reader:
            npanxx = rows[0] + "-" + rows[1]
            npa_dict[rows[0]] = 0
            npanxx_dict[npanxx] = 2

else:
    r = requests.get("https://localcallingguide.com/xmlrc.php?lata=" + lata).text
    data = xmltodict.parse(r)['root']

    for cur in data['rcdata']:

        exch = str(cur['exch'])
        see_exch = str(cur['see-exch'])

        if(see_exch != 'None'):
            if args.debug: print("Skipping: " + exch)
            continue

        print(" Parsing: " + exch)

        r2 = requests.get("https://localcallingguide.com/xmllocalexch.php?exch=" + exch).text
        data2 = xmltodict.parse(r2)['root']['lca-data']

        for cur2 in data2['prefix']:
            try:
                npa = str(cur2['npa'])
                npanxx = npa + "-" + str(cur2['nxx'])
                npa_dict[npa] = 0
                npanxx_dict[npanxx] = 2
            except:
                print("Error parsing " + exch)

    with open(latacsvname, 'w', newline='') as g:
        csvwriter2 = csv.writer(g)
        csvwriter2.writerow(['NPA','NXX'])

        for i in sorted (npanxx_dict):
            csvwriter2.writerow(i.split('-'))

print("Adding NPA-NXX for local exchange...")

r2 = requests.get("https://localcallingguide.com/xmllocalexch.php?exch=" + local_exch).text
data2 = xmltodict.parse(r2)['root']['lca-data']

for cur2 in data2['prefix']:
    try:
        npa = str(cur2['npa'])
        npanxx = npa + "-" + str(cur2['nxx'])
        npa_dict[npa] = 0
        npanxx_dict[npanxx] = 0
    except:
        print("Error parsing " + exch)

lcd_npas = list()

for i in sorted (npa_dict):
    if(i == my_npa):
        lcd_npas.insert(0, i)
    else:
        lcd_npas.append(i)

if args.debug: print("NPAs for these rate centers: " + str(lcd_npas))

# Generate Double-Compressed LCD Tables
# Start with table 136 (0x88)
#
# MTR 2.x supports only Double-Compressed LCD tables (16 maximum.)
table = 136

print("Generating MTR 2.x (Double-Compressed) tables:")
# Loop through list of LCD NPAs for which we need to generate tables.
for i in lcd_npas:
    if table > 155:
        print("Error: maximum of 16 LCD tables reached.")
        break

    fname = "mm_table_" + hex(table)[2:4] + ".bin"
    print("    Double-compressed LCD table " + fname + " for NPA " + i + ".")

    npa_h = int(int(i) / 100)
    npa_t = int((int(i) - (npa_h * 100)) / 10)
    npa_o = int((int(i) - (npa_h * 100)) - (npa_t * 10))

    npa_h = npa_h << 4
    npa_h = npa_h | npa_t
    npa_o = npa_o << 4
    npa_o = npa_o | 0x0e

    # Start LCD table array with first 3 digits of NPA followed by 0xe.
    a = array.array('B', [npa_h, npa_o])

    stflag = 0

    for index in range(200, 1000): #, row in allnpa.iterrows():
        cur_npanxx = str(i) + "-" + str(index)

        if cur_npanxx in npanxx_dict:
            flag = npanxx_dict.get(cur_npanxx)
        else:
            flag = 3    # No entry in NPA table, means invalid.

        # Pack into double-compressed LCD byte
        stflag = stflag << 2
        stflag = stflag | flag

        # Every 4th entry, add compressed LCD byte to table.
        if index % 4 == 3:
            a.append(stflag)
            stflag = 0

    # Write LCD table array to file.
    f=open(fname, "wb")
    a.tofile(f)

    # Proceed to next table
    table = table + 1
    if table == 150: table = 154

# Generate Uncompressed / Compressed LCD Tables
# Uncompressed tables start with table 74 (0x4a)
# Compressed tables start with 101 (0x65)
#
# MTR 1.7 only supports uncompressed tables (maximum of 10 NPAs.)
# MTR 1.9 supports uncompressed tables for the first 10 NPAs, and
#         can have an additional 7 compressed tables for a total
#         of 17 NPAs maximum.
table = 74

print("Generating MTR 1.7 (Uncompressed) / 1.9 (Uncompressed, Compressed) tables:")
# Loop through list of LCD NPAs for which we need to generate tables.
for i in lcd_npas:
    if table > 107:
        print("Error: maximum of 17 Uncompressed LCD tables reached.")
        break

    fname = "mm_table_" + hex(table)[2:4] + ".bin"

    term_npa_h = int(int(my_npa) / 100)
    term_npa_t = int((int(my_npa) - (term_npa_h * 100)) / 10)
    term_npa_o = int((int(my_npa) - (term_npa_h * 100)) - (term_npa_t * 10))

    term_nxx_h = int(int(my_nxx) / 100)
    term_nxx_t = int((int(my_nxx) - (term_nxx_h * 100)) / 10)
    term_nxx_o = int((int(my_nxx) - (term_nxx_h * 100)) - (term_nxx_t * 10))

    term_npa_h = term_npa_h << 4
    term_npa_h = term_npa_h | term_npa_t
    term_npa_o = term_npa_o << 4
    term_npa_o = term_npa_o | term_nxx_h
    term_nxx_h = term_nxx_t << 4
    term_nxx_h = term_nxx_h | term_nxx_o

    npa_h = int(int(i) / 100)
    npa_t = int((int(i) - (npa_h * 100)) / 10)
    npa_o = int((int(i) - (npa_h * 100)) - (npa_t * 10))

    npa_h = npa_h << 4
    npa_h = npa_h | npa_t
    npa_o = npa_o << 4
    npa_o = npa_o | 0x0e

    if table > 91:
        # Compressed LCD: Start the table array with first 3 digits of NPA followed by 0xe.
        a = array.array('B', [npa_h, npa_o])
        print("    Compressed   LCD table " + fname + " for NPA " + i)
    else:
        # Uncompressed LCD: Start the table array with the terminal's NPANXX, followed by
        # the first 3 digits of the called NPA, followed by 0xe.
        a = array.array('B', [term_npa_h, term_npa_o, term_nxx_h, npa_h, npa_o])
        print("    Uncompressed LCD table " + fname + " for NPA " + i + ".")

    stflag = 0

# 0 Local
# 1 LMS (future)
# 2 Intra-lata Toll
# 3 Invalid NPA/NXX
# 4 Inter-lata Toll
    for index in range(200, 1000): #, row in allnpa.iterrows():
        cur_npanxx = str(i) + "-" + str(index)

        if cur_npanxx in npanxx_dict:
            flag = npanxx_dict.get(cur_npanxx)
        else:
            flag = 3    # No entry in NPA table, means invalid.

        if table <= 91:
            # Uncompressed table, each entry is one byte.
            a.append(flag)
        else:
            # Pack into Compressed LCD byte
            stflag = stflag << 4
            stflag = stflag | flag

            # Every 2nd entry, add compressed LCD byte to table.
            if index % 2 == 1:
                a.append(stflag)
                stflag = 0

    if table <= 91:
        # Uncompressed tables have 13 byges of padding at the end.
        for index in range(0, 13):
            a.append(0)

    # Write LCD table array to file.
    f=open(fname, "wb")
    a.tofile(f)

    # Proceed to next table
    table = table + 1
    if table == 82: table = 90
    if table == 92: table = 101

if table > 101:
    print("* * * WARNING: "  + str(my_npa) + "-" + str(my_nxx) + " has more than 10 NPAs and cannot be supported by MTR 1.7.")

print("Successfully completed generating LCD tables.")
