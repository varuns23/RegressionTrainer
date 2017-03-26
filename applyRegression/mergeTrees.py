import ROOT, sys
from array import array

response     = array( 'f', [ 0. ] )
resolution   = array( 'f', [ 0. ] )
e80xECALonly = array( 'f', [ 0. ] )
eOverP = array( 'f', [ 0. ] )

outputFile = ROOT.TFile.Open("/eos/user/r/rcoelhol/80X_NTuples/"+sys.argv[1]+"_withRegression.root", "RECREATE")
inputFile1 = ROOT.TFile.Open("/eos/user/r/rcoelhol/80X_NTuples/"+sys.argv[1]+".root")
inputFile2 = ROOT.TFile.Open(sys.argv[1]+"_application.root")

inputTree1 = inputFile1.Get('een_analyzer/ElectronTree')
inputTree2 = inputFile2.Get('een_analyzer/correction')

outputFile.mkdir('een_analyzer')
outputFile.cd('een_analyzer')
outputTree = inputTree1.CloneTree(0)
outputTree.Branch('eOverP', eOverP, 'eOverP/F')
outputTree.Branch('response', response, 'response/F')
outputTree.Branch('resolution', resolution, 'resolution/F')
outputTree.Branch('e80xECALonly', e80xECALonly, 'e80xECALonly/F')

for ievent in xrange(inputTree1.GetEntries()):
    inputTree1.GetEntry(ievent)
    inputTree2.GetEntry(ievent)

    response[0]   = inputTree2.response
    resolution[0] = inputTree2.resolution
    eOverP[0] = inputTree2.eOverP
    e80xECALonly[0] = inputTree2.response*(inputTree2.scRawEnergy+inputTree2.scPreshowerEnergy)
    outputTree.Fill()

outputTree.Write()
outputFile.Close()
