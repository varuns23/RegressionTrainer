import os
import pickle
from math import sqrt, exp,log
from array import array

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.gROOT.LoadMacro( os.path.abspath('src/effSigma.C') )

# Returns a unique histogram name (useful if object is not accessed by name anyway)
ROOTCNT = 0
def RootName():
    global ROOTCNT
    ret = 'Object' + str(ROOTCNT)
    ROOTCNT += 1
    return ret


def FitSlices( self ):

    ########################################
    # Fill the Fit dict
    ########################################

    self.p( 'SlicePlot for {0} (bins from {1} to {2})'.format(
            self.slicevarname, self.bounds[0], self.bounds[-1] ), 1 )

    self.Fit = {}
    for histvar in self.histvars:
        self.Fit[histvar.GetName()] = {
            'CBvals'     : [],
            'CBerrs'     : [],
            'effsigma'   : [],
            'fitdata'    : [],
            'CBhist'     : [],
            }


    for i in xrange( self.n_bins ):

        x_min = self.bounds[i]
        x_max = self.bounds[i+1]

        sel_str = '{0}>{1}&&{0}<{2}'.format( self.slicevarname, x_min, x_max )
        self.p( 'Creating reduced dataset; sel_str = ' + sel_str )
        hdata_reduced = self.hdata.reduce(sel_str)

        for histvar in self.histvars:
            self.FitOneSlice( hdata_reduced, histvar )


    # Delete the big data set
    del self.hdata

    self.p( 'Dumping instance to {0}.pickle'.format(self.longname), 1 )
    if not os.path.isdir( self.pickledir ): os.makedirs( self.pickledir )
    with open( os.path.join( self.pickledir, self.longname + '.pickle' ), 'wb' ) as pickle_fp:
        pickle.dump( self, pickle_fp )



def FitOneSlice( self, hdata_reduced, histvar, unbinnedFit=False ):

    # Reduce dataset to one column, only for fitting
    hdata_fit = hdata_reduced.reduce( ROOT.RooArgSet(histvar) )

    # Fit parameters
#    mean = ROOT.RooRealVar( RootName(), RootName(), 1.,   0.7,    1.4 );
    mean = ROOT.RooRealVar( RootName(), RootName(), 1.,   0.3,    1.4 );
    sig  = ROOT.RooRealVar( RootName(), RootName(), 0.01, 0.0002, 0.8 );
    a1   = ROOT.RooRealVar( RootName(), RootName(), 1,    0.05,   10 );
    a2   = ROOT.RooRealVar( RootName(), RootName(), 2,    0.05,   10 );
    n1   = ROOT.RooRealVar( RootName(), RootName(), 3,    1.01,   500 );
    n2   = ROOT.RooRealVar( RootName(), RootName(), 3,    1.01,   500 );

    # Fit function
    pdfCB = ROOT.RooDoubleCBFast(
        RootName(), RootName(),
        histvar,
        mean, sig, a1, n1, a2, n2
        )

    self.p( 'Fitting crystal ball to dataset (histvar: {0}, fitrange: {1} to {2})'.format(
            histvar.GetName(), self.fit_x_min, self.fit_x_max ), 3 )

    self.p( 'Number of entries in fit dataset: ' + str(hdata_fit.numEntries()), 4 )
    

    if unbinnedFit:
        pdfCB.fitTo(
            hdata_fit,
            ROOT.RooFit.Range(self.fit_x_min, self.fit_x_max),
            ROOT.RooFit.PrintEvalErrors(-1), ROOT.RooFit.PrintLevel(-1)
            )
    else:
        nBinning = 1000

        hdatahist_fit = ROOT.RooDataHist(
            RootName(), RootName(),
            ROOT.RooArgSet( histvar ),
            hdata_fit
            )

        pdfCB.fitTo(hdatahist_fit,
                    ROOT.RooFit.Range(self.fit_x_min, self.fit_x_max),
                    ROOT.RooFit.PrintEvalErrors(-1), ROOT.RooFit.PrintLevel(-1)
                    )
        sig.setVal(0.5*sig.getVal())
        pdfCB.fitTo(hdatahist_fit,
                    ROOT.RooFit.Range(self.fit_x_min, self.fit_x_max),
                    ROOT.RooFit.PrintEvalErrors(-1), ROOT.RooFit.PrintLevel(-1)
                    )
#        mean.setVal(1.0)
#        pdfCB.fitTo(hdatahist_fit,
#                    ROOT.RooFit.Range(self.fit_x_min, self.fit_x_max),
#                    ROOT.RooFit.PrintEvalErrors(-1), ROOT.RooFit.PrintLevel(-1)
#                    )
            

    # Make a histogram out of the CB function so that effSigma can be calculated
    histCB = pdfCB.createHistogram( RootName(), histvar, ROOT.RooFit.Binning(2000) )
    effsigma = ROOT.effSigma( histCB )

    # Make a histogram of the reduced dataset for easy plotting later
    fitdata = super(hdata_fit.__class__, hdata_fit).createHistogram( RootName(), histvar, ROOT.RooFit.Binning(400) )
    self.p( 'Number of entries in fit histogram: ' + str(fitdata.GetEntries()), 4 )
    self.p( 'Mean of fit histogram: ' + str(fitdata.GetMean()), 5 )

    if fitdata.GetMean() > 1.5 or fitdata.GetMean() < 0.5:
        self.p( 'WARNING: strange mean (fit is designed for 0.5 < mean < 1.5)', 5 )

    # Append results
    self.Fit[histvar.GetName()]['CBvals'].append(
        [ par.getVal() for par in [ a1, n1, mean, sig, a2, n2 ] ] )
    self.Fit[histvar.GetName()]['CBerrs'].append(
        [ par.getError() for par in [ a1, n1, mean, sig, a2, n2 ] ] )
    self.Fit[histvar.GetName()]['CBhist'].append( histCB )
    self.Fit[histvar.GetName()]['effsigma'].append( effsigma )
    self.Fit[histvar.GetName()]['fitdata'].append( fitdata )

    self.p( 'Fit parameters:', 4 )
    self.p( 'mu:       ' + str(mean.getVal()) , 5 )
    self.p( 'sigma:    ' + str(sig.getVal())  , 5 )
    self.p( 'alpha1:   ' + str(a1.getVal())   , 5 )
    self.p( 'alpha2:   ' + str(a2.getVal())   , 5 )
    self.p( 'n1:       ' + str(n1.getVal())   , 5 )
    self.p( 'n2:       ' + str(n2.getVal())   , 5 )
    self.p( 'effsigma: ' + str(effsigma)      , 5 )

    self.p( 'Distribution specifics:', 4 )
    self.p( 'mean:     ' + str(hdata_fit.mean(histvar)) , 5 )
    self.p( 'sigma:    ' + str(hdata_fit.sigma(histvar)), 5 )
    self.p( 'entries:  ' + str(hdata_fit.numEntries())  , 5 )
