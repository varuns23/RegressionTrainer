#!/bin/sh
source /cvmfs/cms.cern.ch/cmsset_default.sh 
cd /cvmfs/cms.cern.ch/slc6_amd64_gcc630/cms/cmssw/CMSSW_9_4_1 
scramv1 runtime -sh
cd ${_CONDOR_SCRATCH_DIR}
./regression.exe Config_cluster_EE_ecal.config
gfal-copy -f -p Config_cluster_EE_ecal_results.root gsiftp://cms-lvs-gridftp.hep.wisc.edu:2811//hdfs/store/user/varuns/EgammaRegression/94X/output/Config_cluster_EE_ecal_results.root
#cp Config_cluster_EE_ecal_results.root /nfs_scratch/varuns/testCondor/Config_cluster_EE_ecal_results.root
