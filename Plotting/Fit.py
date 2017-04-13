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
    parser.add_argument( '--particle', type=str, default='TODO', choices=[ 'electron', 'photon', 'TODO' ] )
    parser.add_argument( '--region', type=str, default='TODO', choices=[ 'EB', 'EE', 'TODO' ] )
    parser.add_argument( '--testrun', action='store_true', help='selects only a few events for testing purposes')

    parser.add_argument( '-s', '--second-regression', action='store_true', help='Use this flag if the sample is trained twice')
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


    particle = args.particle
    ecaltrk = False
    ecaltrkstr = ''
        

    plotdir = 'plotsPY_{0}_{1}'.format( particle, region )
    if ecaltrk: plotdir += '_ECALTRK'
    if args.second_regression: plotdir += '_secondRegression'

    tree_name = particle.capitalize() + 'Tree'
    #    if args.second_regression: tree_name = 'correction'

    pline()
    print 'Summary of input data for Fit.py:'
    print '    particle:  ' + particle
    print '    region:    ' + region
    print '    ecaltrk:   ' + str(ecaltrk)
    print '    plotdir:   ' + plotdir
    print '    ntuple:    ' + args.ntup
    print '    ntup tree: ' + tree_name


    hugeTrkErrorStudy = False
    if hugeTrkErrorStudy:
        print 'Warning! A specific study is activated!'


    ########################################
    # FITTING PROCEDURE
    ########################################

    # ======================================
    # Get the workspace and set the variables

    pline()

    rawEnergy         = ROOT.RooRealVar( "rawEnergy",          "rawEnergy", 0.)
    preshowerEnergy   = ROOT.RooRealVar( "preshowerEnergy",    "preshowerEnergy", 0.)
    r9                = ROOT.RooRealVar( "r9[0]",                 "r9[0]", 0.)
    rhoValue          = ROOT.RooRealVar( "rhoValue",           "rhoValue", 0.)
    genEta            = ROOT.RooRealVar( "genEta",               "genEta", 0. )
    genE              = ROOT.RooRealVar( "genEnergy",            "genEnergy", 0. )
    genPt             = ROOT.RooRealVar( "genPt",                "genPt", 0. )    
    cor74E            = ROOT.RooRealVar( "corrEnergy74X",        "corrEnergy74X", 0. )
    cor74Eerror       = ROOT.RooRealVar( "corrEnergy74XError",   "corrEnergy74XError", 0. )
    trkEta            = ROOT.RooRealVar( "trkEta",               "trkEta", 0.)
    trkPhi            = ROOT.RooRealVar( "trkPhi",               "trkPhi", 0.)
    fbrem             = ROOT.RooRealVar( "fbrem",                "fbrem", 0.)
    response          = ROOT.RooRealVar( "response",             "response", 0.)
    resolution        = ROOT.RooRealVar( "resolution",           "resolution", 0.)
    response2         = ROOT.RooRealVar( "response2",             "response2", 0.)
    resolution2       = ROOT.RooRealVar( "resolution2",           "resolution2", 0.)

    trkMomentum         = ROOT.RooRealVar( "trkMomentum",         "trkMomentum", 0. )
    trkMomentumRelError = ROOT.RooRealVar( "trkMomentumRelError", "trkMomentumRelError", 0. )
    ecalDrivenVar       = ROOT.RooRealVar( "eleEcalDrivenSeed",         "eleEcalDrivenSeed", 0. )


    # ======================================
    # Set ranges where reasonably possible

    rhoValue.setRange(    0., 60. );
    r9.setRange(    0., 1.2 );
    genE.setRange(  0., 10000. );
    genPt.setRange( 0., 10000. );

    if dobarrel:
        genEta.setRange( -1.5, 1.5 )
    else:
        genEta.setRange( -3, 3 )


    # ======================================
    # Define which vars to use

    Vars = [
        rawEnergy,
        preshowerEnergy,
        rhoValue,
        genEta,
        genE,
        genPt,
        cor74E,
        cor74Eerror,
        response,
        resolution,
        ]

    if ecaltrk:
        Vars.extend([
            trkMomentum,
            trkMomentumRelError,
            trkEta,
            trkPhi,
            fbrem
            ])

    if args.second_regression:
        # Use different variables in this case
        Vars = [
            rawEnergy,
            preshowerEnergy,
            r9,
            rhoValue,
            genEta,
            genE,
            genPt,
            cor74E,
            response,
            resolution,
            response2,
            resolution2,

            # Used to compute the 74X Ep combination result
            trkMomentum,
            trkMomentumRelError,

            ecalDrivenVar,
            ]


    VarsArgList = ROOT.RooArgList()
    for Var in Vars: VarsArgList.add(Var)


    # ======================================
    # Create the dataset

    eventcut = ''
    if args.testrun: eventcut = "eventNumber%20==1||eventNumber%20==0"

    print 'Getting dataset (using the macro)'
    hdata = ROOT.LoadDataset( eventcut, dobarrel, args.ntup, 'een_analyzer', tree_name, VarsArgList )
    print '  Using {0} entries'.format( hdata.numEntries() )


    ########################################
    # Add columns to dataset for E_raw,cor,cor74 over E_true
    ########################################

    # NOTE: BRACKETS AROUND THE FORMULA ARE EXTREMELY IMPORTANT
    #       There is no error message, but the results are interpreted totally different without the brackets!
    #       Or it is the RooArgLists that can't be passed in the defition of the RooFormula directly

    nBinningHistVars = 2000

    rawArgList = ROOT.RooArgList( rawEnergy, preshowerEnergy, genE )
    rawformula = ROOT.RooFormulaVar( 'rawformula', 'raw', '((@0+@1)/@2)', rawArgList )
    rawvar = hdata.addColumn(rawformula)
    rawvar.setRange( 0., 2. )
    rawvar.setBins(nBinningHistVars)

    ecor74ArgList = ROOT.RooArgList( cor74E, genE )
    ecor74formula = ROOT.RooFormulaVar( 'ecor74formula', 'corr. (ECAL 74X)', '(@0/@1)', ecor74ArgList )
    ecor74var = hdata.addColumn(ecor74formula)
    ecor74var.setRange( 0., 2. )
    ecor74var.setBins(nBinningHistVars)

    if not args.second_regression:
        # ecor uses target
        ecorArgList = ROOT.RooArgList( response, rawEnergy, preshowerEnergy, genE )
        ecorformula = ROOT.RooFormulaVar(
            'ecorformula', 'corr. (ECAL)',
            '(response * ((rawEnergy+preshowerEnergy)/genEnergy))',
            ecorArgList
            )

        ecorvar = hdata.addColumn(ecorformula)
        ecorvar.setRange( 0., 2. )
        ecorvar.setBins(nBinningHistVars)

    elif args.second_regression:
        # distinguish between corrected ECAL and corrected 2step
        
        ecalCorrArgList = ROOT.RooArgList( response, rawEnergy, preshowerEnergy, genE )
        ecalCorrFormula = ROOT.RooFormulaVar(
            'ecalCorrFormula', 'corr. (ECAL)',
            '(@0 * ((@1+@2)/@3))',
            ecalCorrArgList
            )
        ecalCorrVar = hdata.addColumn(ecalCorrFormula)
        ecalCorrVar.setRange( 0., 2. )
        ecalCorrVar.setBins(nBinningHistVars)

        trkCorrArgList = ROOT.RooArgList( response2, genE, trkMomentum, trkMomentumRelError, rawEnergy, preshowerEnergy, resolution, response )
        trkCorrFormula = ROOT.RooFormulaVar(
            'trkCorrFormula', 'E/p comb.',
            '(@0 / (@1 * (@2*@2*@3*@3 + (@4+@5)*(@4+@5)*@6*@6) / ( (@4+@5)*@7*@2*@2*@3*@3 + @2*(@4+@5)*(@4+@5)*@6*@6 )))',
            trkCorrArgList
            )

        trkCorrVar = hdata.addColumn(trkCorrFormula)
        trkCorrVar.setRange( 0., 2. )
        trkCorrVar.setBins(nBinningHistVars)

        Ep74CorrArgList = ROOT.RooArgList( trkMomentum, trkEta, genE )
        Ep74CorrFormula = ROOT.RooFormulaVar(
            'Ep74CorrFormula', 'corr. (TRK)',
            '((@0*TMath::CosH(@1))/@2)',
            Ep74CorrArgList
            )

        Ep74CorrVar = hdata.addColumn(Ep74CorrFormula)
        Ep74CorrVar.setRange( 0., 2. )
        Ep74CorrVar.setBins(nBinningHistVars)



    ########################################
    # Make the fits
    ########################################

    pline()
    print 'Start fitting\n'

    if not args.ptbins:
        # Default pt bins
        globalPt_bounds = [
            0.,
            20.,
            30.,
            40.,
            50.,
            60., 
            80.,
            100.,
            125.,
            150.,
            175.,
            200.,
            250.,
            275.,
            325.,
            375.,
            425.,
            475.,
            1000.,
            2500.,
            6500.,
            ]
    else:
        globalPt_bounds = args.ptbins


    allPt_bounds = [0.,5.]
    allPt_bounds += [ 6 + i for i in xrange(55) ]
    allPt_bounds += [ 62 + 2*i for i in xrange(20) ]
    allPt_bounds += [ 105 + 5*i for i in xrange(45) ]
    allPt_bounds += [ 335 + 10*i for i in xrange(15) ]
    allPt_bounds += [ 550 + 75*i for i in xrange(7) ]
    allPt_bounds += [ 1250 + 250*i for i in xrange(6) ]
    allPt_bounds += [ 3500 + 1000*i for i in xrange(4) ]
    
    print globalPt_bounds
    print allPt_bounds

    for i_globalPtBin in xrange(len(globalPt_bounds)-1):

        min_globalPt = globalPt_bounds[i_globalPtBin]
        max_globalPt = globalPt_bounds[i_globalPtBin+1]

        print '    Reducing total dataset to genPt between {0} and {1}'.format( min_globalPt, max_globalPt )
        hdata_globalPtBin = hdata.reduce( 'genPt>{0}&&genPt<{1}'.format( min_globalPt, max_globalPt ) )
        print '        Number of entries in this genPt selection: ' + str(hdata_globalPtBin.numEntries())

        if not args.second_regression:
            histogramVariables = [
                rawvar,
                ecor74var,
                ecorvar,
                ]
        elif args.second_regression:
            histogramVariables = [
                rawvar,
                ecor74var,
                ecalCorrVar,
                trkCorrVar,
                Ep74CorrVar,
                ]

        # ======================================
        # genPt plot

        # Get the finer genPt bounds inside this global bin
        localPt_bounds = allPt_bounds[ allPt_bounds.index(min_globalPt) : allPt_bounds.index(max_globalPt) + 1 ]

        # genPt_name = 'GENPT{0}-{1}'.format( int(min_globalPt), int(max_globalPt) )
        genPt_name = 'GENPT-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        genPt_sliceplot = SlicePlot(
            name     = genPt_name,
            longname = particle + region + ecaltrkstr + '_' + genPt_name,
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
            genEta_bounds = [ 
                0.0, 0.220, 0.440, 0.660, 0.789, 1.20, 1.479, 1.50
                ]
        else:
            genEta_bounds = [ 
                1.4, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.5
                ]

        genEta_name = 'GENETA-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        genEta_sliceplot = SlicePlot(
            name     = genEta_name,
            longname = particle + region + ecaltrkstr + '_' + genEta_name,
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



        # ======================================
        # rho plot

        rhoValueVar = rhoValue

        rhoValue_bounds = [ 
            0., 5., 10., 15., 20., 25., 30., 35., 40., 45., 50., 55., 60.
            ]

        rhoValue_name = 'RHOVALUE-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        rhoValue_sliceplot = SlicePlot(
            name     = rhoValue_name,
            longname = particle + region + ecaltrkstr + '_' + rhoValue_name,
            plotdir  = plotdir
            )
        rhoValue_sliceplot.SetDataset( hdata_globalPtBin )
        rhoValue_sliceplot.SetHistVars(histogramVariables)
        rhoValue_sliceplot.SetSliceVar(
            rhoValueVar,
            rhoValue_bounds,
            'rhoValue'
            )
        rhoValue_sliceplot.FitSlices()









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
