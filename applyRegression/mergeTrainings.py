#!/bin/env python

import ROOT, sys, os

scaleDir = sys.argv[1]
resolutionDir = sys.argv[2]

for files in os.listdir(scaleDir):
    
    if not files.endswith('.root'): continue
    if not os.path.exists(resolutionDir + '/' + files): continue

    newFile = ROOT.TFile(files, 'RECREATE')

    scaleFile = ROOT.TFile(scaleDir + '/' + files, 'READ')
    
    EBCorrection = scaleFile.Get('EBCorrection')
    EECorrection = scaleFile.Get('EECorrection')
    
    newFile.cd()
    if EBCorrection != None: newFile.WriteObject(EBCorrection, 'EBCorrection')
    if EECorrection != None: newFile.WriteObject(EECorrection, 'EECorrection')
    
    scaleFile.Close()

    resolutionFile = ROOT.TFile(resolutionDir + '/' + files, 'READ')
    
    EBUncertainty = resolutionFile.Get('EBUncertainty')
    EEUncertainty = resolutionFile.Get('EEUncertainty')
    
    newFile.cd()
    if EBUncertainty != None: newFile.WriteObject(EBUncertainty, 'EBUncertainty')
    if EEUncertainty != None: newFile.WriteObject(EEUncertainty, 'EEUncertainty')
    
    resolutionFile.Close()
    newFile.Close()

