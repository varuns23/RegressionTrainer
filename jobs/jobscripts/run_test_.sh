cd /afs/cern.ch/work/v/varuns/EGMRecoComm/TrainingAOD_92/CMSSW_9_2_3/src
eval `scramv1 runtime -sh`
cd RegressionTrainer
#--------------------------------------------------
echo "START OF RUN FOR test_.config"
#--------------------------------------------------
./regression.exe /afs/cern.ch/work/v/varuns/EGMRecoComm/TrainingAOD_92/CMSSW_9_2_3/src/RegressionTrainer/python/test_.config
#--------------------------------------------------
echo "END OF RUN FOR test_.config"
#--------------------------------------------------
