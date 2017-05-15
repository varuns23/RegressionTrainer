#!/usr/bin/env python
"""
Thomas:
"""

########################################
# Imports
########################################

import os


########################################
# Main
########################################

base_sh = """#$ -o /mnt/t3nfs01/data01/shome/tklijnsm/EGM/CMSSW_8_0_4/src/RegressionTraining/Plotting/jobs/std
#$ -e /mnt/t3nfs01/data01/shome/tklijnsm/EGM/CMSSW_8_0_4/src/RegressionTraining/Plotting/jobs/std

source /mnt/t3nfs01/data01/swshare/psit3/etc/profile.d/cms_ui_env.sh
source $VO_CMS_SW_DIR/cmsset_default.sh

cd /mnt/t3nfs01/data01/shome/tklijnsm/EGM/CMSSW_8_0_4/src/
eval `scramv1 runtime -sh`

cmsenv

cd RegressionTraining/Plotting
{0}
"""


def main():

    cmds = [
        # 'python Fit.py Config_Jul31_electron_EB_ECALonly_results.root --region EB',
        # 'python Fit.py Config_Jul31_electron_EB_ECALTRK_results.root --region EB',
        # 'python Fit.py Config_Jul31_electron_EE_ECALonly_results.root --region EE',
        # 'python Fit.py Config_Jul31_electron_EE_ECALTRK_results.root --region EE',
        # 'python Fit.py Config_Jul31_photon_EB_ECALonly_results.root --region EB',
        # 'python Fit.py Config_Jul31_photon_EE_ECALonly_results.root --region EE',
        'python Fit.py Config_Aug09_electron_EB_ECALonly_results.root --region EB',
        'python Fit.py Config_Aug09_electron_EB_ECALTRK_results.root --region EB',
        'python Fit.py Config_Aug09_electron_EE_ECALonly_results.root --region EE',
        'python Fit.py Config_Aug09_electron_EE_ECALTRK_results.root --region EE',
        'python Fit.py Config_Aug09_photon_EB_ECALonly_results.root --region EB',
        'python Fit.py Config_Aug09_photon_EE_ECALonly_results.root --region EE',
        ]

    for i_cmd, cmd in enumerate(cmds):

        sh_file = 'job{0}.sh'.format(i_cmd)

        with open( sh_file, 'w' ) as sh_fp:
            sh_fp.write( base_sh.format(cmd) )

        os.system( 'chmod 777 ' + sh_file  )
        os.system( 'qsub -q short.q ' + sh_file )

########################################
# End of Main
########################################
if __name__ == "__main__":
    main()