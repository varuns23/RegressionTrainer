python Fit.py --region EB --flag ZS --ntup ../applyRegressionClusters/testingTree_withRegression.root
python Fit.py --region EE --flag ZS --ntup ../applyRegressionClusters/testingTree_withRegression.root
python Fit.py --region EB --flag FULL --ntup ../applyRegressionClusters/testingTree_withRegression.root
python Fit.py --region EE --flag FULL --ntup ../applyRegressionClusters/testingTree_withRegression.root
python Plot.py FitPickles/* --override
cp -rf plotsPY_pfClusters_E* ~/eos/www/pfClustersCalibration/
python Plot.py FitPickles/* --large
cp -rf plotsPY_pfClusters_E* ~/eos/www/pfClustersCalibration/Large
python Plot.py FitPickles/* --small
cp -rf plotsPY_pfClusters_E* ~/eos/www/pfClustersCalibration/Small
makeIndex