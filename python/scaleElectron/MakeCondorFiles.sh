#!/bin/sh

# 1- regression.exe; 2-input config; 3-outputfileName

export CMSSW_RELEASE_BASE=/cvmfs/cms.cern.ch/slc6_amd64_gcc630/cms/cmssw/CMSSW_9_4_1

cat>Job_${3}.sh<<EOF
#!/bin/sh
source /cvmfs/cms.cern.ch/cmsset_default.sh 
cd $CMSSW_RELEASE_BASE 
scramv1 runtime -sh
cd \${_CONDOR_SCRATCH_DIR}
./${1} ${2}
gfal-copy -f -p ${3}_results.root gsiftp://cms-lvs-gridftp.hep.wisc.edu:2811//hdfs/store/user/varuns/EgammaRegression/94X/output/${3}_results.root
#cp ${3}_results.root /nfs_scratch/varuns/testCondor/${3}_results.root
EOF

chmod 775 Job_${3}.sh

cat>condor_${3}<<EOF
x509userproxy        = /tmp/x509up_u4549
use_x509userproxy    = True
universe             = vanilla
Executable           = Job_${3}.sh
Notification         = never
WhenToTransferOutput = On_Exit
ShouldTransferFiles  = yes
Requirements = (TARGET.UidDomain == "hep.wisc.edu" && TARGET.HAS_CMS_HDFS)
on_exit_remove       = (ExitBySignal == FALSE && (ExitCode == 0 || ExitCode == 42 || NumJobStarts>3))
+IsFastQueueJob      = True
getenv               = true
request_memory       = 4096
request_disk         = 4096000
Transfer_Input_Files = ${1}, ${2}
output               = /nfs_scratch/varuns/testCondor/\$(Cluster)_\$(Process)_${3}.out
error                = /nfs_scratch/varuns/testCondor/\$(Cluster)_\$(Process)_${3}.err
Log                  = /nfs_scratch/varuns/testCondor/\$(Cluster)_\$(Process)_${3}.log
Queue
EOF

condor_submit condor_${3}
