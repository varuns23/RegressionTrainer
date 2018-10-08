import ROOT, sys
from array import array

response   = array( 'f', [ 0. ] )
resolution = array( 'f', [ 0. ] )
e90xECALTRK = array( 'f', [ 0. ] )

outputFile = ROOT.TFile.Open(sys.argv[1] + "_withRegression_merged.root", "RECREATE")
inputFile1 = ROOT.TFile.Open(sys.argv[1] + "_withRegression.root")
inputFile2 = ROOT.TFile.Open(sys.argv[1] + "_withRegression_trkRegression.root")

inputTree1 = inputFile1.Get('een_analyzer/ElectronTree')
inputTree2 = inputFile2.Get('een_analyzer/correction')

outputFile.mkdir('een_analyzer')
outputFile.cd('een_analyzer')
outputTree = inputTree1.CloneTree(0)
outputTree.Branch('response2', response, 'response2/F')
outputTree.Branch('resolution2', resolution, 'resolution2/F')
outputTree.Branch('e90xECALTRK', e90xECALTRK, 'e90xECALTRK/F')

nevent = 0
for ievent in xrange(inputTree1.GetEntries()):

    if ievent % 100000 == 0: print ievent

    inputTree1.GetEntry(ievent)
    inputTree2.GetEntry(ievent)

    response[0]   = inputTree2.response2
    resolution[0] = inputTree2.resolution2

    e90xECALTRK[0] = inputTree2.response2*((inputTree1.rawEnergy+inputTree1.preshowerEnergy)*inputTree1.response*inputTree1.trkMomentum*inputTree1.trkMomentum*inputTree1.trkMomentumRelError*inputTree1.trkMomentumRelError+inputTree1.trkMomentum*(inputTree1.rawEnergy+inputTree1.preshowerEnergy)*(inputTree1.rawEnergy+inputTree1.preshowerEnergy)*inputTree1.resolution*inputTree1.resolution)/((inputTree1.trkMomentum*inputTree1.trkMomentum*inputTree1.trkMomentumRelError*inputTree1.trkMomentumRelError+(inputTree1.rawEnergy+inputTree1.preshowerEnergy)*(inputTree1.rawEnergy+inputTree1.preshowerEnergy)*inputTree1.resolution*inputTree1.resolution))

    outputTree.Fill()
    nevent = nevent + 1
outputTree.Write()
outputFile.Close()
