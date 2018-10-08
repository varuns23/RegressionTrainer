In order to use this code do:

```bash
cmsrel CMSSW_9_4_1
cd CMSSW_9_4_1/src
cmsenv
git clone git@github.com:varuns23/HiggsAnalysis.git
scram b -j 4
git clone -b 94X-egm git@github.com:varuns23/RegressionTrainer.git
cd RegressionTrainer
make -j 4
```
