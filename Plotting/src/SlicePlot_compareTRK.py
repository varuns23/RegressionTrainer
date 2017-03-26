import os
import pickle
from math import sqrt, exp,log
from array import array

import ROOT
ROOT.gROOT.SetBatch(True)

c_comp = ROOT.TCanvas( 'c_comp', 'c_comp', 1000, 800 )


def MakePlots_comparison( self, comp ):

    ########################################
    # Plotting
    ########################################

    self.plotdir += '_Comparison'
    if not os.path.isdir(self.plotdir): os.makedirs(self.plotdir)

    self.p( 'Making plots for comparison', 1 )
    self.p( 'self:    ' + self.longname, 2 )
    self.p( 'comp:    ' + comp.longname, 2 )
    self.p( 'plotdir: ' + self.plotdir, 2 )


    # ======================================
    # Per bin fits

    # Find a clever number of divisions for the canvas
    int_sqrt_bins = int(sqrt(self.n_bins))
    dec_sqrt_bins = sqrt(self.n_bins) - int_sqrt_bins

    if dec_sqrt_bins < 0.0000001:
        n_columns = int_sqrt_bins
        n_rows    = int_sqrt_bins
    elif dec_sqrt_bins < 0.5:
        n_columns = int_sqrt_bins + 1
        n_rows    = int_sqrt_bins
    else:
        n_columns = int_sqrt_bins + 1
        n_rows    = int_sqrt_bins + 1

    c_comp.Divide( n_columns, n_rows )



    for histvar in self.histvars:

        histvarname = histvar.GetName()

        legs = []

        for i_bin in xrange(self.n_bins):

            c_comp.cd(i_bin+1)
            ROOT.gPad.SetLeftMargin(0.20)
            ROOT.gPad.SetBottomMargin(0.14)
            ROOT.gPad.SetRightMargin(0.01)
            ROOT.gPad.SetTopMargin(0.1)

            H = self.Fit[histvarname]['fitdata'][i_bin]

            H.SetLineColor(2)
            H.Draw('HIST')

            histvartitle = 'E_{{{0}}}/E_{{true}}'.format( histvar.GetTitle() )
            # Default is to assume it's an energy ratio, if not change the plotting label a bit
            if hasattr( self, 'notAnEnergyRatio' ) and self.notAnEnergyRatio:
                histvartitle = histvar.GetTitle()

            H.SetTitle( '{0} ( {2} < {1} < {3} )'.format(
                histvartitle,
                self.slicevarname,
                self.bounds[i_bin], self.bounds[i_bin+1]
                ))
            H.SetTitleSize(0.06);

            H.GetXaxis().SetTitle( histvartitle )
            H.GetXaxis().SetRangeUser( 0., 1.3 );

            H.GetXaxis().SetLabelSize(0.05);
            H.GetXaxis().SetTitleSize(0.06);
            H.GetYaxis().SetLabelSize(0.05);
            H.GetYaxis().SetTitleSize(0.06);

            # H_CB = self.Fit[histvarname]['CBhist'][i_bin]
            # H_CB.Scale( H.Integral() / H_CB.Integral() * H_CB.GetNbinsX() / H.GetNbinsX() )
            # H_CB.SetLineColor(2)
            # H_CB.Draw('HISTSAMEL')

            H2 = comp.Fit[histvarname]['fitdata'][i_bin]
            H2.SetLineColor(4)
            H2.SetName( 'comp_' + H2.GetName() )
            H2.Draw('HISTSAME')

            leg = ROOT.TLegend( 0.20, 0.5, 0.45, 0.9 )
            leg.AddEntry( H.GetName(),  'arg1', 'l' )
            leg.AddEntry( H2.GetName(), 'arg2', 'l' )
            legs.append(leg)
            legs[-1].Draw('SAME')

        self.Save( c_comp, 'ComparisonPerBin' + histvarname.capitalize() )



    # ======================================
    # Plot of variable over slices

    c_comp.Clear()

    c_comp.SetLeftMargin(   self.sliceplot_LeftMargin )
    c_comp.SetRightMargin(  self.sliceplot_RightMargin )
    c_comp.SetBottomMargin( self.sliceplot_BottomMargin )
    c_comp.SetTopMargin(    self.sliceplot_TopMargin )

    ROOT.gPad.SetGridx()
    ROOT.gPad.SetGridy()


    base = ROOT.TH1F()
    base.Draw()

    base.GetXaxis().SetLimits( self.bounds[0], self.bounds[-1] )
    base.SetMinimum(self.sliceplot_y_min)
    base.SetMaximum(self.sliceplot_y_max)

    base.GetXaxis().SetTitle( self.slicevarname )
    base.GetXaxis().SetNdivisions(505)
    base.GetYaxis().SetTitleOffset(1.1)
    base.GetXaxis().SetLabelSize(0.05)
    base.GetXaxis().SetTitleSize(0.06)
    base.GetYaxis().SetLabelSize(0.05)
    base.GetYaxis().SetTitleSize(0.06)


    # First the means

    base.GetYaxis().SetTitle( '#mu' )

    leg_mu = ROOT.TLegend(
        self.sliceplot_LeftMargin,      1.0-self.sliceplot_TopMargin,
        1.0-self.sliceplot_RightMargin, 1.0-self.sliceplot_TopMargin+self.sliceplot_legheight  )
    leg_mu.SetNColumns(2)
    leg_mu.SetFillStyle(0)

    Hmus = []
    Hmus_filled = []


    for i_histvar, histvar in enumerate(self.histvars):

        Hmu = ROOT.TH1F( 'mu_'+histvar.GetName(), '', self.n_bins, array('d',self.bounds) )

        for i_bin in xrange(self.n_bins):
            bin_width  = self.bounds[i_bin+1] - self.bounds[i_bin]
            bin_center = self.bounds[i_bin] + 0.5*bin_width
            Hmu.SetBinContent( i_bin+1, self.Fit[histvar.GetName()]['CBvals'][i_bin][2] )
            Hmu.SetBinError(   i_bin+1, self.Fit[histvar.GetName()]['CBerrs'][i_bin][2] )

        # This is the line with line error bars
        Hmu.SetMarkerSize(0)
        Hmu.SetLineColor( self.colorlist[i_histvar] )
        Hmu.SetLineWidth(2)

        # This draws filled squares around the error bars (should be drawn before the line object)
        Hmu_filled = Hmu.Clone()
        Hmu_filled.SetName( Hmu_filled.GetName() + '_clone' )
        Hmu_filled.SetFillColorAlpha( self.colorlist[i_histvar], 0.4 )
        Hmu_filled.SetMarkerSize(0)

        Hmu_filled.Draw('SAMEE2')
        Hmu.Draw('HISTSAMEE')

        # Append for persistence
        Hmus.append( Hmu )
        Hmus_filled.append( Hmu_filled )

        leg_mu.AddEntry( Hmu_filled.GetName(), '#mu_{ECALonly, ' + histvar.GetTitle() + '}  ', 'lf' )


    for i_histvar, histvar in enumerate(comp.histvars):
        if '74' in histvar.GetName() or 'raw' in histvar.GetName(): continue

        Hmu = ROOT.TH1F( 'Cmu_'+histvar.GetName(), '', comp.n_bins, array('d',comp.bounds) )

        for i_bin in xrange(comp.n_bins):
            bin_width  = comp.bounds[i_bin+1] - comp.bounds[i_bin]
            bin_center = comp.bounds[i_bin] + 0.5*bin_width
            Hmu.SetBinContent( i_bin+1, comp.Fit[histvar.GetName()]['CBvals'][i_bin][2] )
            Hmu.SetBinError(   i_bin+1, comp.Fit[histvar.GetName()]['CBerrs'][i_bin][2] )

        # This is the line with line error bars
        Hmu.SetMarkerSize(0)
        Hmu.SetLineColor( comp.colorlist[i_histvar] )
        Hmu.SetLineWidth(2)
        Hmu.SetLineStyle(2)

        # This draws filled squares around the error bars (should be drawn before the line object)
        Hmu_filled = Hmu.Clone()
        Hmu_filled.SetName( Hmu_filled.GetName() + '_clone' )
        Hmu_filled.SetFillColorAlpha( comp.colorlist[i_histvar], 0.2 )
        Hmu_filled.SetMarkerSize(0)

        Hmu_filled.Draw('SAMEE2')
        Hmu.Draw('HISTSAMEE')

        # Append for persistence
        Hmus.append( Hmu )
        Hmus_filled.append( Hmu_filled )

        leg_mu.AddEntry( Hmu_filled.GetName(), '#mu_{ECAL+TRK, ' + histvar.GetTitle() + '}  ', 'lf' )


    leg_mu.Draw('SAME')
    self.Save( c_comp, 'ComparisonMuOverBins' )




    # Then the sigmas

    c_comp.Clear()
    base.SetMinimum( self.sliceplotsigma_y_min )
    base.SetMaximum( self.sliceplotsigma_y_max )
    base.Draw()

    base.GetYaxis().SetTitle( '#sigma_{eff}' )

    leg_sigma = ROOT.TLegend(
        self.sliceplot_LeftMargin,      1.0-self.sliceplot_TopMargin,
        1.0-self.sliceplot_RightMargin, 1.0-self.sliceplot_TopMargin+self.sliceplot_legheight  )
    leg_sigma.SetNColumns(2)
    leg_sigma.SetFillStyle(0)

    Hsigmas = []
    Hsigmas_filled = []

    for i_histvar, histvar in enumerate(self.histvars):

        Hsigma = ROOT.TH1F( 'sigma_'+histvar.GetName(), '', self.n_bins, array('d',self.bounds) )

        for i_bin in xrange(self.n_bins):
            bin_width  = self.bounds[i_bin+1] - self.bounds[i_bin]
            bin_center = self.bounds[i_bin] + 0.5*bin_width
            Hsigma.SetBinContent( i_bin+1, self.Fit[histvar.GetName()]['effsigma'][i_bin] )
            Hsigma.SetBinError(   i_bin+1, self.Fit[histvar.GetName()]['CBerrs'][i_bin][3] )

        # This is the line with line error bars
        Hsigma.SetMarkerSize(0)
        Hsigma.SetLineColor( self.colorlist[i_histvar] )
        Hsigma.SetLineWidth(2)

        # This draws filled squares around the error bars (should be drawn before the line object)
        Hsigma_filled = Hsigma.Clone()
        Hsigma_filled.SetName( Hsigma_filled.GetName() + '_clone' )
        Hsigma_filled.SetFillColorAlpha( self.colorlist[i_histvar], 0.4 )
        Hsigma_filled.SetMarkerSize(0)

        Hsigma_filled.Draw('SAMEE2')
        Hsigma.Draw('HISTSAMEE')

        # Append for persistence
        Hsigmas.append( Hsigma )
        Hsigmas_filled.append( Hsigma_filled )

        leg_sigma.AddEntry( Hsigma_filled.GetName(), '#sigma_{ECALonly, ' + histvar.GetTitle() + '}', 'lf' )


    for i_histvar, histvar in enumerate(comp.histvars):
        if '74' in histvar.GetName() or 'raw' in histvar.GetName(): continue

        Hsigma = ROOT.TH1F( 'Csigma_'+histvar.GetName(), '', comp.n_bins, array('d',comp.bounds) )

        for i_bin in xrange(comp.n_bins):
            bin_width  = comp.bounds[i_bin+1] - comp.bounds[i_bin]
            bin_center = comp.bounds[i_bin] + 0.5*bin_width
            Hsigma.SetBinContent( i_bin+1, comp.Fit[histvar.GetName()]['effsigma'][i_bin] )
            Hsigma.SetBinError(   i_bin+1, comp.Fit[histvar.GetName()]['CBerrs'][i_bin][3] )

        # This is the line with line error bars
        Hsigma.SetMarkerSize(0)
        Hsigma.SetLineColor( comp.colorlist[i_histvar] )
        Hsigma.SetLineStyle(2)
        Hsigma.SetLineWidth(2)

        # This draws filled squares around the error bars (should be drawn before the line object)
        Hsigma_filled = Hsigma.Clone()
        Hsigma_filled.SetName( Hsigma_filled.GetName() + '_clone' )
        Hsigma_filled.SetFillColorAlpha( comp.colorlist[i_histvar], 0.2 )
        Hsigma_filled.SetMarkerSize(0)

        Hsigma_filled.Draw('SAMEE2')
        Hsigma.Draw('HISTSAMEE')

        # Append for persistence
        Hsigmas.append( Hsigma )
        Hsigmas_filled.append( Hsigma_filled )

        leg_sigma.AddEntry( Hsigma_filled.GetName(), '#sigma_{ECAL+TRK, ' + histvar.GetTitle() + '}', 'lf' )


    leg_sigma.Draw('SAME')
    self.Save( c_comp, 'ComparisonEffSigmaOverBins' )


