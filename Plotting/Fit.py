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

    if os.environ['HOSTNAME'] == 't3ui17':
        defaultNtupPath = os.path.join( '/mnt/t3nfs01/data01/shome/tklijnsm/Samples/RegressionSamples', '22Jul_samples' )
    else:
        defaultNtupPath = '/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples'

    defaultNtup = os.path.join( defaultNtupPath, 'Ntup_Jul22_fullpt_testing_sample.root' )


    ########################################
    # Command line options
    ########################################

    parser = argparse.ArgumentParser()
    parser.add_argument( 'resultfile', type=str )
    parser.add_argument( '--particle', type=str, default='TODO', choices=[ 'electron', 'photon', 'TODO' ] )
    parser.add_argument( '--region', type=str, default='TODO', choices=[ 'EB', 'EE', 'TODO' ] )
    parser.add_argument( '--ecaltrk', action='store_true', help='Tells the program trk variables are included')
    parser.add_argument( '--testrun', action='store_true', help='selects only a few events for testing purposes')

    parser.add_argument( '-s', '--second-regression', action='store_true', help='Use this flag if the sample is trained twice')
    parser.add_argument( '--ntup', type=str, default=defaultNtup, help='Pass path to an Ntuple (current default is {0})'.format(defaultNtup) )
    parser.add_argument( '--ptbins', metavar='N', type=float, nargs='+',
                         help='supply a list of global pt bins (there is a default list in the code)' )
    args = parser.parse_args()


    ########################################
    # Settings
    ########################################

    # This is the _results.root filename
    ws_file = os.path.abspath( args.resultfile )

    # Region needs to be determined carefully because the right workspace needs to be loaded
    if args.region == 'TODO':
        if 'EB' in ws_file:
            region = 'EB'
            dobarrel = True
        elif 'EE' in ws_file:
            region = 'EE'
            dobarrel = False
        else:
            print 'Could not determine region (EB or EE) from the filename; pass it manually using the flag --region **'
            return
    else:
        if args.region == 'EB':
            region = 'EB'
            dobarrel = True
        elif args.region == 'EE':
            region = 'EE'
            dobarrel = False


    if args.particle == 'TODO':
        if 'electron' in ws_file:
            particle = 'electron'
        elif 'photon' in ws_file:
            particle = 'photon'
        else:
            print 'Could not determine particle (electron or photon) from the filename; pass it manually using the flag --region **'
            return
    else:
        particle = args.particle


    if args.ecaltrk:
        # User explicitely tells to use ecaltrk variables
        ecaltrk = True
        ecaltrkstr = 'TRK'
    else:
        # Find out if trk variables were included
        if 'ECALonly' in ws_file:
            ecaltrk = False
            ecaltrkstr = ''
        elif 'ECALTRK' in ws_file:
            ecaltrk = True
            ecaltrkstr = 'TRK'
        else:
            ecaltrk = False
            ecaltrkstr = ''


    plotdir = 'plotsPY_{0}_{1}'.format( particle, region )
    if ecaltrk: plotdir += '_ECALTRK'
    if args.second_regression: plotdir += '_secondRegression'

    tree_name = particle.capitalize() + 'Tree'
    #    if args.second_regression: tree_name = 'correction'

    pline()
    print 'Summary of input data for Fit.py:'
    print '    WS file:   ' + ws_file
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

    scRawEnergy       = ROOT.RooRealVar( "scRawEnergy",          "scRawEnergy", 0.)
    scPreshowerEnergy = ROOT.RooRealVar( "scPreshowerEnergy",    "scPreshowerEnergy", 0.)
    r9                = ROOT.RooRealVar( "r9",                   "r9", 0.)
    nVtx              = ROOT.RooRealVar( "nVtx",                 "nVtx", 0.)
    pt                = ROOT.RooRealVar( "pt",                   "pt", 0.)


    genEta            = ROOT.RooRealVar( "genEta",               "genEta", 0. )
    genE              = ROOT.RooRealVar( "genEnergy",            "genEnergy", 0. )
    genPt             = ROOT.RooRealVar( "genPt",                "genPt", 0. )
    
    cor74E            = ROOT.RooRealVar( "corrEnergy74X",        "corrEnergy74X", 0. )
    cor74Eerror       = ROOT.RooRealVar( "corrEnergy74XError",   "corrEnergy74XError", 0. )

    # Only add to ArgList if TRK variables were included in the training
    trkMom            = ROOT.RooRealVar( "trkMomentum",          "trkMomentum", 0.)
    trkMomE           = ROOT.RooRealVar( "trkMomentumError",     "trkMomentumError", 0.)
    trkEta            = ROOT.RooRealVar( "trkEta",               "trkEta", 0.)
    trkPhi            = ROOT.RooRealVar( "trkPhi",               "trkPhi", 0.)
    fbrem             = ROOT.RooRealVar( "fbrem",                "fbrem", 0.)

    # These variables are only available for second-regression mode
    response          = ROOT.RooRealVar( "response",             "response", 0.)
    resolution        = ROOT.RooRealVar( "resolution",           "resolution", 0.)
    response2         = ROOT.RooRealVar( "response2",             "response2", 0.)
    resolution2       = ROOT.RooRealVar( "resolution2",           "resolution2", 0.)

    full5x5_r9        = ROOT.RooRealVar( "full5x5_r9",           "full5x5_r9", 0. )


    trkMomentum         = ROOT.RooRealVar( "trkMomentum",         "trkMomentum", 0. )
    trkMomentumRelError = ROOT.RooRealVar( "trkMomentumRelError", "trkMomentumRelError", 0. )

    ecalDrivenVar     = ROOT.RooRealVar( "eleEcalDrivenSeed",         "eleEcalDrivenSeed", 0. )


    # ======================================
    # Set ranges where reasonably possible

    r9.setRange(    0., 1.2 );
    pt.setRange(    0., 10000. );
    genE.setRange(  0., 10000. );
    genPt.setRange( 0., 10000. );

    if dobarrel:
        genEta.setRange( -1.5, 1.5 )
    else:
        genEta.setRange( -3, 3 )


    # ======================================
    # Define which vars to use

    Vars = [
        scRawEnergy,
        scPreshowerEnergy,
        full5x5_r9,
        nVtx,
        pt,
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
            trkMom,
            trkMomE,
            trkEta,
            trkPhi,
            fbrem
            ])

    if args.second_regression:
        # Use different variables in this case
        Vars = [
            scRawEnergy,
            scPreshowerEnergy,
            full5x5_r9,
            nVtx,
            genEta,
            genE,
            genPt,
            cor74E,
            response,
            resolution,
            response2,
            resolution2,

            # Used to compute the 74X Ep combination result
            pt,
            trkEta,
            full5x5_r9,
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

    rawArgList = ROOT.RooArgList( scRawEnergy, scPreshowerEnergy, genE )
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
        ecorArgList = ROOT.RooArgList( response, scRawEnergy, scPreshowerEnergy, genE )
        ecorformula = ROOT.RooFormulaVar(
            'ecorformula', 'corr. (ECAL)',
            '(response * ((scRawEnergy+scPreshowerEnergy)/genEnergy))',
            ecorArgList
            )

        ecorvar = hdata.addColumn(ecorformula)
        ecorvar.setRange( 0., 2. )
        ecorvar.setBins(nBinningHistVars)

    elif args.second_regression:
        # distinguish between corrected ECAL and corrected 2step
        
        ecalCorrArgList = ROOT.RooArgList( response, scRawEnergy, scPreshowerEnergy, genE )
        ecalCorrFormula = ROOT.RooFormulaVar(
            'ecalCorrFormula', 'corr. (ECAL)',
            '(@0 * ((@1+@2)/@3))',
            ecalCorrArgList
            )
        ecalCorrVar = hdata.addColumn(ecalCorrFormula)
        ecalCorrVar.setRange( 0., 2. )
        ecalCorrVar.setBins(nBinningHistVars)

        trkCorrArgList = ROOT.RooArgList( response2, genE, trkMomentum, trkMomentumRelError, scRawEnergy, scPreshowerEnergy, resolution, response )
        trkCorrFormula = ROOT.RooFormulaVar(
            'trkCorrFormula', 'E/p comb.',
            '(@0 / (@1 * (@2*@2*@3*@3 + (@4+@5)*(@4+@5)*@6*@6) / ( (@4+@5)*@7*@2*@2*@3*@3 + trkMomentum*(@4+@5)*(@4+@5)*@6*@6 )))',
            trkCorrArgList
            )

        trkCorrVar = hdata.addColumn(trkCorrFormula)
        trkCorrVar.setRange( 0., 2. )
        trkCorrVar.setBins(nBinningHistVars)

        Ep74CorrArgList = ROOT.RooArgList( pt, trkEta, genE )
        Ep74CorrFormula = ROOT.RooFormulaVar(
            'Ep74CorrFormula', 'E/p comb. (74X)',
            '((@0*TMath::CosH(@1))/@2)',
            Ep74CorrArgList
            )

        Ep74CorrVar = hdata.addColumn(Ep74CorrFormula)
        Ep74CorrVar.setRange( 0., 2. )
        Ep74CorrVar.setBins(nBinningHistVars)


        trkWeightArgList = ROOT.RooArgList(
            scRawEnergy, scPreshowerEnergy, resolution,
            trkMomentum, trkMomentumRelError
            )
        trkWeightStr = (
            '(' +
            '  ((scRawEnergy+scPreshowerEnergy)*(scRawEnergy+scPreshowerEnergy)*resolution*resolution)' +
            '    /  ' +
            '  ( trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError' +
            '    + (scRawEnergy+scPreshowerEnergy)*(scRawEnergy+scPreshowerEnergy)*resolution*resolution )' +
            ')'
            )
        trkWeightStr = trkWeightStr.replace(' ','')
        trkWeightFormula = ROOT.RooFormulaVar(
            'trkWeight', 'trkWeight',
            trkWeightStr,
            trkWeightArgList
            )

        trkWeightVar = hdata.addColumn(trkWeightFormula)
        trkWeightVar.setRange( 0., 2. )
        trkWeightVar.setBins(200)


        ecalWeightArgList = ROOT.RooArgList(
            scRawEnergy, scPreshowerEnergy, resolution,
            trkMomentum, trkMomentumRelError
            )
        ecalWeightStr = (
            '(' +
            '  (trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError)' +
            '    /  ' +
            '  ( trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError' +
            '    + (scRawEnergy+scPreshowerEnergy)*(scRawEnergy+scPreshowerEnergy)*resolution*resolution )' +
            ')'
            )
        ecalWeightStr = ecalWeightStr.replace(' ','')
        ecalWeightFormula = ROOT.RooFormulaVar(
            'ecalWeight', 'ecalWeight',
            ecalWeightStr,
            ecalWeightArgList
            )

        ecalWeightVar = hdata.addColumn(ecalWeightFormula)
        ecalWeightVar.setRange( 0., 2. )
        ecalWeightVar.setBins(200)



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


        if hugeTrkErrorStudy:
            print '          Reducing total dataset FURTHER to trkWeightVar<0.01'
            hdata_globalPtBin = hdata_globalPtBin.reduce( '(trkWeight<0.01)' )
            print '              Number of entries in this genPt selection now: ' + str(hdata_globalPtBin.numEntries())


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
        # Weight plots

        # # Get the finer genPt bounds inside this global bin
        # localPt_bounds = allPt_bounds[ allPt_bounds.index(min_globalPt) : allPt_bounds.index(max_globalPt) + 1 ]


        # TRKW_name = 'TRKW-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        # TRKW_sliceplot = SlicePlot(
        #     name     = TRKW_name,
        #     longname = particle + region + ecaltrkstr + '_' + TRKW_name,
        #     plotdir  = plotdir
        #     )
        # TRKW_sliceplot.SetDataset( hdata_globalPtBin )
        # TRKW_sliceplot.SetHistVars( [ trkWeightVar, ecalWeightVar ] )
        # TRKW_sliceplot.SetSliceVar(
        #     genPt,
        #     localPt_bounds,
        #     'p_{t, gen}'
        #     )
        # # Specify that it's not an energy ratio (for plotting purposes only)
        # setattr( TRKW_sliceplot, 'notAnEnergyRatio', True )
        # setattr( TRKW_sliceplot, 'disableDrawFits', True )
        # TRKW_sliceplot.FitSlices()

        # continue


        # ======================================
        # ecalDriven plot

        # ecalDriven_bounds = [ 
        #     -0.1, 0.5, 1.1
        #     ]

        # ecalDriven_name = 'ecalDriven-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        # ecalDriven_sliceplot = SlicePlot(
        #     name     = ecalDriven_name,
        #     longname = particle + region + ecaltrkstr + '_' + ecalDriven_name,
        #     plotdir  = plotdir
        #     )
        # ecalDriven_sliceplot.SetDataset( hdata_globalPtBin )
        # ecalDriven_sliceplot.SetHistVars(histogramVariables)
        # ecalDriven_sliceplot.SetSliceVar(
        #     ecalDrivenVar,
        #     ecalDriven_bounds,
        #     'ecalDriven'
        #     )
        # ecalDriven_sliceplot.FitSlices()

         #        continue


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
        # r9 plot

        r9Var = full5x5_r9

        r9_bounds = [ 
            0.5, 0.8, 0.85, 0.9, 0.92, 0.94, 0.95, 0.96, 0.97, 1.2
            ]

        r9_name = 'R9-{0:04d}-{1:04d}'.format( int(min_globalPt), int(max_globalPt) )
        r9_sliceplot = SlicePlot(
            name     = r9_name,
            longname = particle + region + ecaltrkstr + '_' + r9_name,
            plotdir  = plotdir
            )
        r9_sliceplot.SetDataset( hdata_globalPtBin )
        r9_sliceplot.SetHistVars(histogramVariables)
        r9_sliceplot.SetSliceVar(
            r9Var,
            r9_bounds,
            'r9'
            )
        r9_sliceplot.FitSlices()









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
