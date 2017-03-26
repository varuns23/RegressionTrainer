#!/bin/bash

if [ $1 -eq 1 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Electron_EB.root -o Ntup_10Nov_Electron_EB_application.root -p ../python/Config_Jan14_electron_EB_ECALonly_LOWPT.config -b ../Config_Jan14_electron_EB_ECALonly_LOWPT_results.root -c ../Config_Jan14_electron_EB_ECALonly_HIGHPT_results.root -e ../Config_Jan14_electron_EE_ECALonly_LOWPT_results.root -f ../Config_Jan14_electron_EE_ECALonly_HIGHPT_results.root 
elif [ $1 -eq 2 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Electron_EE.root -o Ntup_10Nov_Electron_EE_application.root -p ../python/Config_Jan14_electron_EE_ECALonly_LOWPT.config -b ../Config_Jan14_electron_EB_ECALonly_LOWPT_results.root -c ../Config_Jan14_electron_EB_ECALonly_HIGHPT_results.root -e ../Config_Jan14_electron_EE_ECALonly_LOWPT_results.root -f ../Config_Jan14_electron_EE_ECALonly_HIGHPT_results.root 
elif [ $1 -eq 3 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Photon_EB.root -o Ntup_10Nov_Photon_EB_application.root -p ../python/Config_Jan14_photon_EB_ECALonly_LOWPT.config -b ../Config_Jan14_photon_EB_ECALonly_LOWPT_results.root -c ../Config_Jan14_photon_EB_ECALonly_HIGHPT_results.root -e ../Config_Jan14_photon_EE_ECALonly_LOWPT_results.root -f ../Config_Jan14_photon_EE_ECALonly_HIGHPT_results.root 
elif [ $1 -eq 4 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Photon_EE.root -o Ntup_10Nov_Photon_EE_application.root -p ../python/Config_Jan14_photon_EE_ECALonly_LOWPT.config -b ../Config_Jan14_photon_EB_ECALonly_LOWPT_results.root -c ../Config_Jan14_photon_EB_ECALonly_HIGHPT_results.root -e ../Config_Jan14_photon_EE_ECALonly_LOWPT_results.root -f ../Config_Jan14_photon_EE_ECALonly_HIGHPT_results.root 
elif [ $1 -eq 5 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Electron_EB_withRegression.root -o Ntup_10Nov_Electron_EB_application.root -p ../python/Config_Jan14_electron_EB_TRKstep2_LOWPT.config -b ../Config_Jan14_electron_EB_TRKstep2_LOWPT_results.root -c ../Config_Jan14_electron_EB_TRKstep2_HIGHPT_results.root -e ../Config_Jan14_electron_EE_TRKstep2_LOWPT_results.root -f ../Config_Jan14_electron_EE_TRKstep2_HIGHPT_results.root --limhighpt 50
elif [ $1 -eq 6 ]
then
    ./applyRegression.exe -t /eos/user/r/rcoelhol/80X_NTuples/Ntup_10Nov_Electron_EE_withRegression.root -o Ntup_10Nov_Electron_EE_application.root -p ../python/Config_Jan14_electron_EE_TRKstep2_LOWPT.config -b ../Config_Jan14_electron_EB_TRKstep2_LOWPT_results.root -c ../Config_Jan14_electron_EB_TRKstep2_HIGHPT_results.root -e ../Config_Jan14_electron_EE_TRKstep2_LOWPT_results.root -f ../Config_Jan14_electron_EE_TRKstep2_HIGHPT_results.root --limhighpt 50
fi