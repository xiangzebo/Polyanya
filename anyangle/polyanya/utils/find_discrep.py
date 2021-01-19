#!/usr/bin/python3
import csv
import sys

def get_time(d):
    for name in "length realcost".split():
        if name in d:
            return float(d[name])
    raise Exception("can't find time")

def main():
    if len(sys.argv) != 3:
        print("usage:", sys.argv[0], "result1 result2")
        return

    with open(sys.argv[1]) as r1:
        with open(sys.argv[2]) as r2:
            csv1 = csv.DictReader(r1, delimiter=";")
            csv2 = csv.DictReader(r2, delimiter=";")
            for line1, line2 in zip(csv1, csv2):
                if abs(get_time(line1) - get_time(line2)) > 1e-8:
                    print(line1["index"])

if __name__ == "__main__":
    main()
