

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

from glob import glob
from time import sleep

from sys import exit


########################################
# Main
########################################

parser = argparse.ArgumentParser()

# Arguments related to the training
parser.add_argument( '--config', required=True,  type=str, help='The config file with which the result was trained' )
parser.add_argument( '--result', required=True,  type=str, help='The result (a root file ending with _result.root)' )
parser.add_argument( '--ntup'  , required=True,  type=str, help='Path to the ntuple on which the training result is to be ran' )
parser.add_argument( '--out'   , default='AUTO', type=str, help='Name of the output tuple' )
parser.add_argument( '--region', default='AUTO', type=str, help='Detector region', choices=['EB','EE','AUTO'] )

# Arguments related to the batch
parser.add_argument( '-b', '--batch', action='store_true', help='executes the command as a job on the batch')
parser.add_argument( '--test', action='store_true', help='Does not submit the job, but creates the .sh file and prints')
parser.add_argument( '-q', '--queue',
    type=str, choices=['short', 'all', 'long', '8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw' ],
    default='8nh', help='which queue to submit to')
parser.add_argument( '-n', '--normalmemory', action='store_true', help='By default more memory is requested; this option disables that')
parser.add_argument( '-k', '--keep', action='store_true', help='Does not clean the output and jobscript directories')

args = parser.parse_args()


# Figure out the platform
host = os.environ['HOSTNAME']
if host == 't3ui17':
    onPsi = True
    onLxplus = False
elif 'lxplus' in host:
    onPsi = False
    onLxplus = True


# Figure out from which path was executed and where the module is
pwd = os.path.abspath( os.getcwd() )

modulePath = os.path.abspath( os.path.dirname(__file__) )
if not pwd == modulePath:
    print 'Execute this script from ' + modulePath
    print pwd
    print modulePath
    exit()

jobscriptDir = os.path.join( pwd, 'jobscripts' )
stdDir       = os.path.join( pwd, 'std' )
LSFJOBdirs   = glob('LSFJOB_*')


def main():

    if onLxplus and args.queue in ['short', 'all', 'long' ]:
        print 'Queue {0} does not exist on {1}; Please pick from {2}'.format(
            args.queue, host, ', '.join(['8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw'])    )
        return
    elif onPsi and args.queue in ['8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw']:
        print 'Queue {0} does not exist on {1}; Please pick from {2}'.format(
            args.queue, host, ', '.join(['short', 'all', 'long' ])    )
        return


    # Clean old directories
    if not args.keep and args.batch:
        print 'Cleaning up old directories'
        if os.path.isdir(jobscriptDir): shutil.rmtree( jobscriptDir )
        
        if onPsi:
            if os.path.isdir(stdDir): shutil.rmtree( stdDir )
        elif onLxplus: 
            for LSFJOBdir in LSFJOBdirs: shutil.rmtree(LSFJOBdir)

    # Make directories if necessary
    if args.batch:
        if not os.path.isdir(jobscriptDir): os.makedirs( jobscriptDir )
        if onPsi and not os.path.isdir(stdDir): os.makedirs( stdDir )


    ########################################
    # Create the command
    ########################################

    if args.out == 'AUTO':

        # Determine what kind of ntuple was passed
        if 'testing_sample' in args.ntup:
            ntupType = 'testing_sample'
        elif 'testing' in args.ntup:
            ntupType = 'testing'
        elif 'training' in args.ntup:
            ntupType = 'training'
        else:
            print 'Not recognizing the ntuple ' + args.ntup
            ntupType = 'unknown'

        args.out = os.path.basename(args.result).replace('.root','') + '_' + ntupType + '_ptWeight.root'


    if args.region == 'AUTO':
        if 'EB' in args.result:
            args.region = 'EB'
        elif 'EE' in args.result:
            args.region = 'EE'

    if args.region == 'EB':
        applyRegressionRegionFlag = 'b'
    elif args.region == 'EE':
        applyRegressionRegionFlag = 'e'


    cmd = './applyRegression.exe -p {0} -{4} {1} -t {2} -o {3}'.format(
        args.config, args.result, args.ntup, args.out, applyRegressionRegionFlag )


    if args.test:
        print 'TEST MODE. command to be run is: ' + cmd
        if args.batch: Make_jobscript( cmd, jobscriptDir, stdDir )
    else:
        if args.batch:
            Make_jobscript( cmd, jobscriptDir, stdDir )
        else:
            print 'cmd = ' + cmd
            os.system( cmd )
            # print 'disabled'



########################################
# Functions
########################################

def Make_jobscript( cmd, jobscriptDir, stdDir ):

    # ======================================
    # Creating the sh file

    sh_file = jobscriptDir + '/run_' + ( os.path.basename(args.out) ).replace( '.root', '.sh' )

    sh_fp = open( sh_file, 'w' )
    p = lambda text: sh_fp.write( text + '\n' )

    # Setup

    if onPsi: 
        p( '#$ -o ' + stdDir )
        p( '#$ -e ' + stdDir )
        p( 'source /swshare/psit3/etc/profile.d/cms_ui_env.sh' )
        p( 'source $VO_CMS_SW_DIR/cmsset_default.sh' )


    # Going into right directory
    p( 'cd {0}/src'.format( os.path.abspath( os.environ['CMSSW_BASE'] ) ) )
    p( 'eval `scramv1 runtime -sh`' )
    p( 'cd RegressionTraining/applyRegression' )

    p( '#' + '-'*50 )
    p( 'echo "START OF JOB"' )
    p( '#' + '-'*50 )

    p( cmd )

    p( '#' + '-'*50 )
    p( 'echo "END OF JOB"' )
    p( '#' + '-'*50 )

    sh_fp.close()


    # ======================================
    # Setting permissions and submitting

    os.system( 'chmod 777 ' + sh_file  )


    # ------------------
    # Parsing the command

    if onPsi:

        # qsub part of command
        subCmd = 'qsub -q {0}.q '.format(args.queue)

        # Request more memory by default, but don't if argument is passed
        if not args.normalmemory:
            subCmd += '-l h_vmem=5g '

        # Pass the sh file
        subCmd += os.path.relpath(sh_file)

    elif onLxplus:

        subCmd = 'bsub -q {0} -J {1} < '.format(args.queue, sh_file.rsplit('/',1)[-1].replace('.sh','') ) + os.path.relpath(sh_file)


    # ------------------
    # Submission

    if args.test:
        print 'TEST MODE. Would now run: ' + subCmd
        print '          which executes: ' + cmd
    else:
        os.system( subCmd )
        # print 'Script disabled'



########################################
# End of Main
########################################
if __name__ == "__main__":
    main()