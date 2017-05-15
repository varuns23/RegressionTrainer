#!/usr/bin/env python
"""
Thomas:
"""

########################################
# Imports
########################################

import os
import argparse
import pickle

# Change directory to location of this source file
execDir = os.path.dirname( os.path.abspath(__file__) )
os.chdir( execDir )

import sys
sys.path.append('src')

from SlicePlot import SlicePlot

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

ROOT.gROOT.ProcessLine("gErrorIgnoreLevel = kError;")
ROOT.RooMsgService.instance().setSilentMode(True)

ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Eval )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Generation )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Minimization )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Plotting )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Fitting )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Integration )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.LinkStateMgmt )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Caching )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Optimization )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.ObjectHandling )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.InputArguments )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Tracing )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Contents )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.DataHandling )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.NumIntegration )
ROOT.RooMsgService.instance().getStream(1).removeTopic( ROOT.RooFit.Eval )

ROOT.gSystem.Load("libHiggsAnalysisGBRLikelihood")
ROOT.gROOT.LoadMacro( os.getcwd() + "/src/LoadDataset.C" )

# Paths to regression results
result_path = os.getcwd()


########################################
# Main
########################################

def Fit():

    ########################################
    # Command line options
    ########################################

    parser = argparse.ArgumentParser()
    parser.add_argument( '--region', type=str, default='TODO', choices=[ 'EB', 'EE', 'TODO' ] )
    parser.add_argument( '--ntup', type=str, help='Pass path to an Ntuple')
    parser.add_argument( '--ptbins', metavar='N', type=float, nargs='+',
                         help='supply a list of global pt bins (there is a default list in the code)' )
    args = parser.parse_args()


    ########################################
    # Settings
    ########################################

    # Region needs to be determined carefully because the right workspace needs to be loaded
    if args.region == 'EB':
        region = 'EB'
        dobarrel = True
    elif args.region == 'EE':
        region = 'EE'
        dobarrel = False

    plotdir = 'plotsPY_pfClusters_{0}'.format( region )

    tree_name = 'PfTree'

    pline()
    print 'Summary of input data for Fit.py:'
    print '    region:    ' + region
    print '    plotdir:   ' + plotdir
    print '    ntuple:    ' + args.ntup
    print '    ntup tree: ' + tree_name


    ########################################
    # FITTING PROCEDURE
    ########################################

    # ======================================
    # Get the workspace and set the variables

    pline()

    clusrawE  = ROOT.RooRealVar("clusrawE", "clusrawE", 0.)
    cluscorrE = ROOT.RooRealVar("cluscorrE", "cluscorrE", 0.)
    clusPt    = ROOT.RooRealVar("clusPt", "clusPt", 0.)
    clusEta   = ROOT.RooRealVar("clusEta", "clusEta", 0.)
    clusPhi   = ROOT.RooRealVar("clusPhi", "clusPhi", 0.)
    clusSize  = ROOT.RooRealVar("clusSize", "clusSize", 0.)
    clusPS1   = ROOT.RooRealVar("clusPS1", "clusPS1", 0.)
    clusPS2   = ROOT.RooRealVar("clusPS2", "clusPS2", 0.)
    clusFlag  = ROOT.RooRealVar("clusFlag", "clusFlag", 0.)
    nvtx      = ROOT.RooRealVar("nvtx", "nvtx", 0.)
    genEnergy = ROOT.RooRealVar("genEnergy", "genEnergy", 0.)
    genPt     = ROOT.RooRealVar("genPt", "genPt", 0.)
    genEta    = ROOT.RooRealVar("genEta", "genEta", 0.)
    genPhi    = ROOT.RooRealVar("genPhi", "genPhi", 0.)
    e91X      = ROOT.RooRealVar("e91X", "e91X", 0.)
    
    # ======================================
    # Set ranges where reasonably possible

    clusrawE.setRange(0., 300.)
    cluscorrE.setRange(0., 300.)
    genEnergy.setRange(0., 300.)

    clusPt.setRange(0., 100.)
    genPt.setRange(0., 100.)

    clusPhi.setRange(-ROOT.TMath.Pi(), ROOT.TMath.Pi())
    genPhi.setRange(-ROOT.TMath.Pi(), ROOT.TMath.Pi())

    clusSize.setRange(0., 25.)
    nvtx.setRange(0., 60.)

    if dobarrel:
        clusEta.setRange(-1.5, 1.5)
        genEta.setRange(-1.5, 1.5)
    else:
        clusEta.setRange(-3.,3.)
        genEta.setRange(-3,3)


    # ======================================
    # Define which vars to use

    Vars = [
        clusrawE,
        cluscorrE,
        clusPt,
        clusEta,
        clusPhi,
        clusSize,
        clusPS1,
        clusPS2,
        clusFlag,
        nvtx,
        genEnergy,
        genPt,
        genEta,
        genPhi,
        e91X
        ]

    VarsArgList = ROOT.RooArgList()
    for Var in Vars: VarsArgList.add(Var)


    # ======================================
    # Create the dataset

    print 'Getting dataset (using the macro)'
    eventcut = ''
    hdata = ROOT.LoadDataset( eventcut, dobarrel, args.ntup, 'een_analyzer', tree_name, VarsArgList )
    print '  Using {0} entries'.format( hdata.numEntries() )


    ########################################
    # Add columns to dataset for E_raw,cor,cor74 over E_true
    ########################################

    # NOTE: BRACKETS AROUND THE FORMULA ARE EXTREMELY IMPORTANT
    #       There is no error message, but the results are interpreted totally different without the brackets!
    #       Or it is the RooArgLists that can't be passed in the defition of the RooFormula directly

    nBinningHistVars = 2000

    rawArgList = ROOT.RooArgList( clusrawE, genEnergy )    
    rawformula = ROOT.RooFormulaVar( 'rawformula', 'raw', '(@0/@1)', rawArgList )
    rawvar = hdata.addColumn(rawformula)
    rawvar.setRange( 0., 2. )
    rawvar.setBins(nBinningHistVars)

    ecor74ArgList = ROOT.RooArgList( cluscorrE, genEnergy )
    ecor74formula = ROOT.RooFormulaVar( 'ecor74formula', 'corr. (74X)', '(@0/@1)', ecor74ArgList )
    ecor74var = hdata.addColumn(ecor74formula)
    ecor74var.setRange( 0., 2. )
    ecor74var.setBins(nBinningHistVars)

    ecor91ArgList = ROOT.RooArgList( e91X, genEnergy )
    ecor91formula = ROOT.RooFormulaVar( 'ecor91formula', 'corr. (91X)', '(@0/@1)', ecor91ArgList )
    ecor91var = hdata.addColumn(ecor91formula)
    ecor91var.setRange( 0., 2. )
    ecor91var.setBins(nBinningHistVars)

    
    ########################################
    # Make the fits
    ########################################

    pline()
    print 'Start fitting\n'

    if not args.ptbins:
        # Default pt bins
        globalPt_bounds = [
            0.25,
            4.5,
            18.,
            100.
            ]
    else:
        globalPt_bounds = args.ptbins


    allPt_bounds = [0.25,100.]
    allPt_bounds += [ 0.25  + 0.425*i for i in xrange(10) ]
    allPt_bounds += [ 4.5   + 1.35*i  for i in xrange(10) ]
    allPt_bounds += [ 100.0 + 8.2*i   for i in xrange(10) ]
    
    for i_globalPtBin in xrange(len(globalPt_bounds)-1):

        min_globalPt = globalPt_bounds[i_globalPtBin]
        max_globalPt = globalPt_bounds[i_globalPtBin+1]

        print '    Reducing total dataset to genPt between {0} and {1}'.format( min_globalPt, max_globalPt )
        hdata_globalPtBin = hdata.reduce( 'genPt>{0}&&genPt<{1}'.format( min_globalPt, max_globalPt ) )
        print '        Number of entries in this genPt selection: ' + str(hdata_globalPtBin.numEntries())

        histogramVariables = [
            rawvar,
            ecor74var,
            ecor91var,
            ]
        
        # ======================================
        # genPt plot

        # Get the finer genPt bounds inside this global bin
        localPt_bounds = allPt_bounds[ allPt_bounds.index(min_globalPt) : allPt_bounds.index(max_globalPt) + 1 ]

        # genPt_name = 'GENPT{0}-{1}'.format( int(min_globalPt), int(max_globalPt) )
        genPt_name = 'GENPT-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        genPt_sliceplot = SlicePlot(
            name     = genPt_name,
            longname = region + '_' + genPt_name,
            plotdir  = plotdir
            )
        genPt_sliceplot.SetDataset( hdata_globalPtBin )
        genPt_sliceplot.SetHistVars( histogramVariables )
        genPt_sliceplot.SetSliceVar(
            genPt,
            localPt_bounds,
            'p_{t, gen}'
            )
        genPt_sliceplot.FitSlices()


        # ======================================
        # genEta plot

        if dobarrel:
            genEta_bounds = [ 0.0 + 0.0375*i for i in xrange(41) ]
        else:
            genEta_bounds = [ 1.4 + 0.0275*i for i in xrange(41) ]

        genEta_name = 'GENETA-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        genEta_sliceplot = SlicePlot(
            name     = genEta_name,
            longname = region + '_' + genEta_name,
            plotdir  = plotdir
            )
        genEta_sliceplot.SetDataset( hdata_globalPtBin )
        genEta_sliceplot.SetHistVars(histogramVariables)
        genEta_sliceplot.SetSliceVar(
            genEta,
            genEta_bounds,
            '#eta_{gen}'
            )
        genEta_sliceplot.FitSlices()


########################################
# Functions
########################################

def pline(s='='):
    print '\n' + s*70


########################################
# End of Main
########################################
if __name__ == "__main__":
    Fit()
