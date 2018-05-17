#!/usr/bin/python

# Copyright 2017 Tony Kuo
# This program is distributed under the terms of the GNU General Public License

# Utility Script
# Compiles a consensus classification based on the results of readclassify, for tetraploid.
# Classification is determined by the reference with the highest probability.
# ex)
#   eagle -t 8 -a chrA/refsort.bam -r A.fa -v AB.vcf --omega=1e-40 --mvh --isc --splice --verbose 1> A.vs.B.txt 2> A.vs.B.readinfo.txt
#   eagle-rc -a chrA/refsort.bam --listonly -o A.vs.B A.vs.B.txt A.vs.B.readinfo.txt > A.vs.B.list
#
#   eagle -t 8 -a chrB/refsort.bam -r B.fa -v BA.vcf --omega=1e-40 --mvh --isc --splice --verbose 1> B.vs.A.txt 2> B.vs.A.readinfo.txt
#   eagle-rc -a chrB/refsort.bam --listonly -o B.vs.A B.vs.A.txt B.vs.A.readinfo.txt > B.vs.A.list
#
#   python ref2_consensus.py -o sample -A A.vs.B.list -B B.vs.A.list
#   eagle-rc -a chrA/refsort.bam --refonly --readlist -o sample.chrA sample.chrA.list
#   eagle-rc -a chrB/refsort.bam --refonly --readlist -o sample.chrB sample.chrB.list

from __future__ import print_function
import argparse
import re, os, sys
import numpy as np
from datetime import datetime
from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE,SIG_DFL) 

def readFile(fn, entry):
    with open(fn, 'r') as fh:
        for line in fh:
            if re.match('^#', line): continue

            t = line.strip().split('\t')
            key = "{}\t{}".format(t[0], t[7])
            pos = "{}\t{}".format(t[2], t[3])
            entry[key] = (pos, float(t[4]), np.logaddexp(np.logaddexp(float(t[4]), float(t[5])), float(t[6]))) # total = prgu + prgv + pout
    fh.close
    print("Read:\t{}\t{}".format(fn, datetime.now()), file=sys.stderr)
    return(entry)

def writeTable(chrA, chrB, unique_reads, out_prefix):
    fhA = open(out_prefix + '.chrA.list', 'w')
    fhB = open(out_prefix + '.chrB.list', 'w')
    fh = [fhA, fhB]

    threshold = np.log(0.95)
    for key in chrA:
        if key not in chrB: continue

        pos = [chrA[key][0], chrB[key][0]]
        x = [chrA[key][1], chrB[key][1]] # numerator
        y = [chrA[key][2], chrB[key][2]] # denominator

        p = [x[0] - y[0], x[1] - y[1]]
        i = max(range(len(p)), key=p.__getitem__)
        d = [p[i] - p[j] for j in range(len(p)) if i != j]

        if p[i] >= threshold and min(d) >= np.log(0.01): c = "REF"
        else: c = "UNK"
        t = key.strip().split('\t')
        print("{}\t{}\t{}\t{}\t{}\t-\t{}\t-".format(t[0], c, pos[i], x[i], y[i], t[1]), file=fh[i])

    if unique_reads:
        for key in chrA:
            if key in chrB: continue
            if chrA[key][1] - chrA[key][2] >= threshold: c = "REF"
            else: c = "UNK"
            t = key.strip().split('\t')
            print("{}\t{}\t{}\t{}\t{}\t-\t{}\t-".format(t[0], c, chrA[key][0], chrA[key][1], chrA[key][2], t[1]), file=fhA)

        for key in chrB:
            if key in chrA: continue
            if chrB[key][1] - chrB[key][2] >= threshold: c = "REF"
            else: c = "UNK"
            t = key.strip().split('\t')
            print("{}\t{}\t{}\t{}\t{}\t-\t{}\t-".format(t[0], c, chrB[key][0], chrB[key][1], chrB[key][2], t[1]), file=fhB)

    fhA.close()
    fhB.close()
    print("Done:\t{}".format(datetime.now()), file=sys.stderr)

def main():
    #python ref2consensus.py -o $F.ref -A $F.A.vs.B.list -B $F.B.vs.A.list
    parser = argparse.ArgumentParser(description='Determine read classification REF = A, B.  Classification is determined by log likelihood ratio')
    parser.add_argument('-A', nargs='+', required=True, help='2 list files: from readclassify with A as reference followed by mirror consensus')
    parser.add_argument('-B', nargs='+', required=True, help='2 list files: from readclassify with B as reference followed by mirror consensus')
    parser.add_argument('-o', type=str, required=True, help='output file prefix')
    parser.add_argument('-u', action='store_true', help='include reads that map uniquely to one reference genome')
    args = parser.parse_args()
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()

    print("Start:\t{0}".format(datetime.now()), file=sys.stderr)
    chrA = {}
    chrA = readFile(args.A[0], chrA)
    chrB = {}
    chrB = readFile(args.B[0], chrB)
    writeTable(chrA, chrB, args.u, args.o)

if __name__ == '__main__':
    main()
    #os._exit(1)#!/usr/bin/python

