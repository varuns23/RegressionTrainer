

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


########################################
# Main
########################################

parser = argparse.ArgumentParser()
parser.add_argument( '--test', action='store_true', help='Does not submit the job, but creates the .sh file and prints')
parser.add_argument( '-q', '--queue',
    type=str, choices=['short', 'all', 'long', '8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw' ],
    default='1nh', help='which queue to submit to')
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



def main():

    if onLxplus and args.queue in ['short', 'all', 'long' ]:
        print 'Queue {0} does not exist on {1}; Please pick from {2}'.format(
            args.queue, host, ', '.join(['8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw'])    )
        return
    elif onPsi and args.queue in ['8nm', '1nh', '8nh', '1nd', '2nd', '1nw', '2nw']:
        print 'Queue {0} does not exist on {1}; Please pick from {2}'.format(
            args.queue, host, ', '.join(['short', 'all', 'long' ])    )
        return


    pwd = os.getcwd()

    jobscriptDir = os.path.join( pwd, 'jobscripts' )
    stdDir       = os.path.join( pwd, 'std' )
    LSFJOBdirs   = glob('LSFJOB_*')

    # Clean old directories
    if not args.keep:
        print 'Cleaning up old directories'
        if os.path.isdir(jobscriptDir): shutil.rmtree( jobscriptDir )
        
        if onPsi:
            if os.path.isdir(stdDir): shutil.rmtree( stdDir )
        elif onLxplus: 
            for LSFJOBdir in LSFJOBdirs: shutil.rmtree(LSFJOBdir)

    # Make directories if necessary
    if not os.path.isdir(jobscriptDir): os.makedirs( jobscriptDir )
    if onPsi and not os.path.isdir(stdDir): os.makedirs( stdDir )
            

    ########################################
    # Arrange input to Fit.py
    ########################################

    # EB and EE ECALonly training
    # result = '../savedResults/UsedForOct28Talk/Config_Oct20_electron_EB_ECALonly_results.root'
    # result = '../savedResults/UsedForOct28Talk/Config_Oct20_electron_EE_ECALonly_results.root'
    
    # EB TRK training
    # result = '../savedResults/UsedForOct28Talk/Config_Oct25_electron_EB_TRKstep2_FastOptions_results.root'
    # result = '../savedResults/UsedForOct28Talk/Config_Oct25_electron_EB_TRKstep2_SlowOptions_results.root'

    # EE TRK training
    # result = '../savedResults/UsedForOct28Talk/Config_Oct26_electron_EE_TRKstep2_FastOptions_results.root'
    # result = '../savedResults/UsedForOct28Talk/Config_Oct26_electron_EE_TRKstep2_results.root'

    # globalPtBins = [
    #     0.,
    #     100.,
    #     500.,
    #     2500.,
    #     6500.,
    #     ]

    # ntup = '../applyRegression/Config_Oct20_electron_EB_ECALonly_results_testing_ptWeight.root'
    # ntup = '../applyRegression/Config_Oct20_electron_EB_ECALonly_results_testing_sample_ptWeight.root'
    
    # ntup = '../applyRegression/Config_Oct20_electron_EE_ECALonly_results_testing_ptWeight.root'
    # ntup = '../applyRegression/Config_Oct20_electron_EE_ECALonly_results_testing_sample_ptWeight.root'

    fitjobs = [

        # Format:
        # (
        #     'path/to/result.root',
        #     list of pt bins, eg: [0., 100., 500., 2500., 6500., ],
        #     'path/tp/ntup.root',
        #     True/False regarding whether result is a "second regression" result
        #     ),

        ### ELECTRONS

        # # EB fitjob
        # (
        #     '../savedResults/UsedForOct28Talk/Config_Oct25_electron_EB_TRKstep2_FastOptions_results.root',
        #     [0., 100., 500., 2500., 6500., ],
        #     '../applyRegression/Config_Oct20_electron_EB_ECALonly_results_testing_ptWeight.root',
        #     True
        #     ),


        # # EE fitjob
        # (
        #     '../savedResults/UsedForOct28Talk/Config_Oct26_electron_EE_TRKstep2_FastOptions_results.root',
        #     [0., 100., 500., 2500., 6500., ],
        #     '../applyRegression/Config_Oct20_electron_EE_ECALonly_results_testing_ptWeight.root',
        #     True
        #     ),


        ### PHOTONS

        # EB fitjob
        (
            '../savedResults/UsedForOct28Talk/Config_Oct26_photon_EB_ECALonly_NI3000_results.root',
            [0., 100., 500., 2500., 6500., ],
            '/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing.root',
            False
            ),

        # EE fitjob
        (
            # '../savedResults/UsedForOct28Talk/Config_Oct26_photon_EB_ECALonly_NI3000_results.root',
            # '../Config_Oct27_photon_EE_ECALonly_NI3000_Fast_results.root',
            '../Config_Oct27_photon_EE_ECALonly_NI3000_results.root',
            [0., 100., 500., 2500., 6500., ],
            '/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing.root',
            False
            ),

        ]


    ########################################
    # Make command and submit
    ########################################

    for result, globalPtBins, ntup, isSecondRegression in fitjobs:

        for iPtBound in xrange( len(globalPtBins)-1 ):

            ptMin = int(globalPtBins[iPtBound])
            ptMax = int(globalPtBins[iPtBound+1])

            cmd = 'python Fit.py {0} --ntup {1} --ptbins {2} {3}'.format(
                result, ntup, ptMin, ptMax )

            if isSecondRegression:
                cmd += ' --second-regression'

            shFileName = os.path.basename(result).replace('.root','') + '_PT{0:04d}-{1:04d}.sh'.format( int(ptMin), int(ptMax) )

            Make_jobscript( cmd, shFileName, jobscriptDir, stdDir )

            if iPtBound != len(globalPtBins)-2 and not args.test:
                nSleep = 1
                print 'Sleeping {0} seconds to prevent jobs from interfering with one another'.format(nSleep)
                sleep(nSleep)



########################################
# Functions
########################################

def Make_jobscript( fitcmd, resultName, jobscriptDir, stdDir ):

    # ======================================
    # Creating the sh file

    sh_file = jobscriptDir + '/run_' + (os.path.basename(resultName)).replace( '.root', '.sh' )

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
    p( 'cd RegressionTraining/Plotting' )

    p( '#' + '-'*50 )
    p( 'echo "START OF FITTING"' )
    p( '#' + '-'*50 )

    p( fitcmd )

    p( '#' + '-'*50 )
    p( 'echo "END OF FITTING"' )
    p( '#' + '-'*50 )

    sh_fp.close()


    # ======================================
    # Setting permissions and submitting

    os.system( 'chmod 777 ' + sh_file  )


    # ------------------
    # Parsing the command

    if onPsi:

        # qsub part of command
        cmd = 'qsub -q {0}.q '.format(args.queue)

        # Request more memory by default, but don't if argument is passed
        if not args.normalmemory:
            cmd += '-l h_vmem=5g '

        # Pass the sh file
        cmd += os.path.relpath(sh_file)

    elif onLxplus:

        cmd = 'bsub -q {0} -J {1} < '.format(args.queue, sh_file.rsplit('/',1)[-1].replace('.sh','') ) + os.path.relpath(sh_file)


    # ------------------
    # Submission

    if args.test:
        print 'TEST MODE. Would now run: ' + cmd
        print '          which executes: ' + fitcmd
    else:
        os.system( cmd )
        # print 'Script disabled'



########################################
# End of Main
########################################
if __name__ == "__main__":
    main()