#!/usr/bin/python

import sys
import os.path

# This is a tiny script to help you creating a CSV file from a face
# database with a similar hierarchie:
# 
#  philipp@mango:~/facerec/data/at$ tree
#  .
#  |-- README
#  |-- s1
#  |   |-- 1.pgm
#  |   |-- ...
#  |   |-- 10.pgm
#  |-- s2
#  |   |-- 1.pgm
#  |   |-- ...
#  |   |-- 10.pgm
#  ...
#  |-- s40
#  |   |-- 1.pgm
#  |   |-- ...
#  |   |-- 10.pgm
#

if __name__ == "__main__":
    
    if len(sys.argv) != 2:
        print "usage: create_csv <base_path>"
        sys.exit(1)
    
    BASE_PATH=sys.argv[1]
    SEPARATOR=";"
    
    #for dirname, dirnames, filenames in os.walk(BASE_PATH):
    label = -1
    for filename in os.listdir(BASE_PATH):
        label = filename[len("subject"):]
        period_index = label.find(".")
        label = label[:period_index]
        label = int(label)
        abs_path = os.path.join(os.getcwd(), BASE_PATH, filename)
        print "%s%s%d" % (abs_path, SEPARATOR, label)
