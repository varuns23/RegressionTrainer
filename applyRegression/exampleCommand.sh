# TRAINING_NTUP="/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_training.root"
# TESTING_NTUP="/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing.root"
# TESTING_SAMPLE_NTUP="/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing_sample.root"

# RESULT="../savedResults/UsedForOct28Talk/Config_Oct20_electron_EE_ECALonly_results.root"

# python applyRegressionWrapper.py \
#     --config ../python/Config_Oct20_electron_EE_ECALonly.config \
#     --result "$RESULT" \
#     --ntup   "$TESTING_SAMPLE_NTUP" \
#     --batch

# python applyRegressionWrapper.py \
#     --config ../python/Config_Oct20_electron_EE_ECALonly.config \
#     --result "$RESULT" \
#     --ntup   "$TRAINING_NTUP" \
#     --keep \
#     --batch

# python applyRegressionWrapper.py \
#     --config ../python/Config_Oct20_electron_EE_ECALonly.config \
#     --result "$RESULT" \
#     --ntup   "$TESTING_NTUP" \
#     --keep \
#     --batch


# Remade with run and lumi numers included

TRAINING_NTUP="/afs/cern.ch/work/r/rcoelhol/public/CMSSW_8_0_12/src/NTuples/Ntup_10Nov_ElectronPhoton.root"
TESTING_NTUP="/afs/cern.ch/work/r/rcoelhol/public/CMSSW_8_0_12/src/NTuples/Ntup_10Nov_ElectronPhoton.root"
TESTING_SAMPLE_NTUP="/afs/cern.ch/work/r/rcoelhol/public/CMSSW_8_0_12/src/NTuples/Ntup_10Nov_ElectronPhoton.root"

RESULT="../Config_Nov26_electron_EB_ECALonly_NONE_results.root"

python applyRegressionWrapper.py \
    --config ../python/Config_Nov26_electron_EB_ECALonly_NONE.config \
    --result "$RESULT" \
    --ntup   "$TESTING_SAMPLE_NTUP" \
    --out "Ntup_10Nov_ElectronPhoton_apply.root"
    --batch