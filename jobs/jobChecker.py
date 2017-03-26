#!/usr/bin/env python

"""
Thomas:
"""

########################################
# Imports
########################################

import os
import shutil
import argparse
import sys

from glob import glob


if 'lxplus' in os.environ['HOSTNAME']:
    onLxplus = True
    onPsi = False
elif 't3' in os.environ['HOSTNAME']:
    onLxplus = False
    onPsi = True
else:
    print 'Could not determine platform, guessing lxplus'
    onLxplus = True
    onPsi = False


########################################
# Main
########################################

parser = argparse.ArgumentParser()
parser.add_argument( '-o', action='store_true', help='also show .o files')
args = parser.parse_args()


def main():

    pwd = os.path.dirname(os.path.realpath(sys.argv[0]))

    jobscriptDir = os.path.join( pwd, 'jobscripts' )
    stdDir       = os.path.join( pwd, 'std' )


    # ======================================
    # Checking qstat

    if onPsi:
        pline()
        print 'Doing qstat: \n'
        os.system( 'qstat' )

    elif onLxplus:
        pline()
        print 'Doing bjobs: \n'
        os.system( 'bjobs' )


    # ======================================
    # Checking the error outputs

    if onPsi:
        eFiles = glob( stdDir + '/*.e*' )
    elif onLxplus:
        eFiles = glob( 'LSFJOB_*/LSFJOB' )


    for eFile in eFiles:

        pline()
        print 'Contents of ' + os.path.basename( eFile ) + ':'
        print

        with open( eFile, 'r' ) as eFP:
            eLines = eFP.readlines()
            PrintNLines( eLines, 50, mode='head' )


    # ======================================
    # Checking the error outputs

    if args.o:

        if onPsi:
            oFiles = glob( stdDir + '/*.o*' )
        elif onLxplus:
            oFiles = glob( 'LSFJOB_*/STDOUT' )


        for oFile in oFiles:

            print '\n' + '-'*70
            print 'Contents of ' + os.path.basename( oFile ) + ':'
            print

            with open( oFile, 'r' ) as oFP:
                oLines = oFP.readlines()
                PrintNLines( oLines, 60 )




def pline( symbol='-', N=70 ):
    print '\n' + symbol*N


def PrintNLines( lines, N, mode='headtail' ):
    nLines = len(lines)

    # Simply print all lines
    if nLines <= N :
        print ''.join(lines)
        return


    if mode == 'head':
        print 'Printing head of file'
        print ''.join(lines[:N])
        print '<----------------- Rest of file cut off ----------------->'
        return

    elif mode == 'tail':
        print 'Printing tail of file'
        print '<----------------- Beginning of file cut off ----------------->'
        print ''.join(lines[nLines-N:])
        return

    elif mode == 'headtail' or mode == 'tailhead':
        halfN = int(N/2)
        print 'Printing head and tail of file'
        print ''.join(lines[:halfN])
        print '<----------------- Middle of file left out ----------------->'
        print ''.join(lines[nLines-halfN:])
        return












########################################
# End of Main
########################################
if __name__ == "__main__":
    main()