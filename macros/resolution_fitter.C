#include "TStyle.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"
#include "RooAbsMoment.h"
#include "RooRealVar.h"
#include "RooTruthModel.h"
#include "RooAbsPdf.h"
#include "RooExponential.h"
#include "RooFFTConvPdf.h"
#include "RooGaussian.h"
#include "RooDecay.h"
#include "RooPlot.h"
#include "RooCBFast.h"
#include "TCanvas.h"
#include "RooConstVar.h"
#include "RooDataSet.h"
#include "RooHybridBDTAutoPdf.h"
#include "RooFormulaVar.h"
#include "RooProdPdf.h"
#include "RooUniform.h"
#include "TRandom.h"
#include "TGraph.h"
#include "RooAddPdf.h"
#include "RooNDKeysPdf.h"
#include "RooExtendPdf.h"
#include "RooMinimizer.h"
#include "TFile.h"
#include "TNtuple.h"
#include "HybridGBRForest.h"
#include "RooProduct.h"
#include "RooChebychev.h"
#include "RooBernstein.h"
#include "RooPolynomial.h"
#include "RooGenericPdf.h"
#include "RooDoubleCBFast.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooCBShape.h"
#include "RooWorkspace.h"
#include "TH1D.h"
#include "TChain.h"
#include "TCut.h"
#include "TLine.h"
#include "TLegend.h"
#include "RooRandom.h"
#include "RooAddition.h"
#include "TSystem.h"
#include "RooFitResult.h"
#include "RooLinearVar.h"
//#include "RooCMSShape.h"
//#include "RooCBExGaussShape.h"
#include "RooBreitWigner.h"
#include <utility>

using namespace RooFit;
using namespace std;

int bins = 75;
float width = 5.;

Double_t effSigma(TH1 * hist, double quantile=TMath::Erf(1.0/sqrt(2.0)))
{

  TAxis *xaxis = hist->GetXaxis();
  Int_t nb = xaxis->GetNbins();
  if(nb < 10) {
    cout << "effsigma: Not a valid histo. nbins = " << nb << endl;
    return 0.;
  }

  Double_t bwid = xaxis->GetBinWidth(1);
  if(bwid == 0) {
    cout << "effsigma: Not a valid histo. bwid = " << bwid << endl;
    return 0.;
  }
  Double_t xmax = xaxis->GetXmax();
  Double_t xmin = xaxis->GetXmin();
  Double_t ave = hist->GetMean();
  Double_t rms = hist->GetRMS();

  Double_t total=0.;
  for(Int_t i=0; i<nb+2; i++) {
    total+=hist->GetBinContent(i);
  }

  Int_t ierr=0;
  Int_t ismin=999;

  Double_t rlim=quantile*total;
  Int_t nrms=rms/(bwid);    // Set scan size to +/- rms
  if(nrms > nb/10) nrms=nb/10; // Could be tuned...

  Double_t widmin=9999999.;
  for(Int_t iscan=-nrms;iscan<nrms+1;iscan++) { // Scan window centre
    Int_t ibm=(ave-xmin)/bwid+1+iscan;
    Double_t x=(ibm-0.5)*bwid+xmin;
    Double_t xj=x;
    Double_t xk=x;
    Int_t jbm=ibm;
    Int_t kbm=ibm;
    Double_t bin=hist->GetBinContent(ibm);
    total=bin;
    for(Int_t j=1;j<nb;j++){
      if(jbm < nb) {
	jbm++;
	xj+=bwid;
	bin=hist->GetBinContent(jbm);
	total+=bin;
	if(total > rlim) break;
      }
      else ierr=1;
      if(kbm > 0) {
	kbm--;
	xk-=bwid;
	bin=hist->GetBinContent(kbm);
	total+=bin;
	if(total > rlim) break;
      }
      else ierr=1;
    }
    Double_t dxf=(total-rlim)*bwid/bin;
    Double_t wid=(xj-xk+bwid-dxf)*0.5;
    if(wid < widmin) {
      widmin=wid;
      ismin=iscan;
    }
  }
  if(ismin == nrms || ismin == -nrms) ierr=3;
  if(ierr != 0) cout << "effsigma: Error of type " << ierr << endl;

  return widmin;

}

void resolution_fitter(const char* filename) {

  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING) ;
  RooMsgService::instance().setSilentMode(true);
  
  TFile* inputFile = TFile::Open(filename);
  TTree* tree = (TTree*) inputFile->Get("een_analyzer/ElectronTree");

  RooRealVar scRawEnergy("scRawEnergy", "scRawEnergy", 0., 5000.);
  RooRealVar scPreshowerEnergy("scPreshowerEnergy", "scPreshowerEnergy", 0., 5000.);
  RooRealVar trkMomentum("trkMomentum", "trkMomentum", 0., 5000.);
  RooRealVar trkEta("trkEta", "trkEta", -5., 5.);
  RooRealVar genEnergy("genEnergy", "genEnergy", 0., 5000.);
  RooRealVar genPt("genPt", "genPt", 0., 5000.);
  RooRealVar scIsEB("scIsEB", "scIsEB", 0., 2.);

  // 3189
  RooArgSet vars(scRawEnergy, scPreshowerEnergy, trkMomentum, trkEta, genEnergy, genPt, scIsEB);

  RooDataSet eb("eb", "eb", tree, vars, TString::Format("scIsEB").Data());
  RooDataSet ee("ee", "ee", tree, vars, TString::Format("!scIsEB").Data());

  RooFormulaVar ECALresponse("ECALresponse", "ECALresponse", "(scRawEnergy+scPreshowerEnergy-genEnergy)/genEnergy", RooArgList(genEnergy, scRawEnergy, scPreshowerEnergy));
  RooFormulaVar TRKresponse("TRKresponse", "TRKresponse", "(trkMomentum/cosh(trkEta)-genPt)/genPt", RooArgList(genPt, trkMomentum, trkEta));

  RooDataSet* eb_clone = new RooDataSet(eb, "eb_clone");
  RooDataSet* ee_clone = new RooDataSet(ee, "ee_clone");
  RooRealVar* ECALresponse_eb = (RooRealVar*) eb_clone->addColumn(ECALresponse);
  RooRealVar* TRKresponse_eb = (RooRealVar*) eb_clone->addColumn(TRKresponse);
  
  RooRealVar* ECALresponse_ee = (RooRealVar*) ee_clone->addColumn(ECALresponse);
  RooRealVar* TRKresponse_ee = (RooRealVar*) ee_clone->addColumn(TRKresponse);

  RooArgSet vars_eb(vars);
  vars_eb.add(RooArgSet(*ECALresponse_eb, *TRKresponse_eb));
  RooArgSet vars_ee(vars);
  vars_ee.add(RooArgSet(*ECALresponse_ee, *TRKresponse_ee));

  for (int i=0; i<100; i++) {
      cout << ((RooRealVar*) eb_clone->get(i)->find("ECALresponse"))->getVal() << endl;
      cout << ((RooRealVar*) eb_clone->get(i)->find("TRKresponse"))->getVal() << endl;
  }
    
  vector<float> binning;
  for (int i=0; i<bins; i++) binning.push_back(width*i);

  TGraphErrors* ECALgraph_eb = new TGraphErrors(binning.size()-1);
  TGraphErrors* ECALgraph_ee = new TGraphErrors(binning.size()-1);
  TGraphErrors* TRKgraph_eb = new TGraphErrors(binning.size()-1);
  TGraphErrors* TRKgraph_ee = new TGraphErrors(binning.size()-1);

  TCanvas* c1 = new TCanvas();
  
  for (size_t i=0; i<binning.size()-1; i++) {

    RooDataSet* ECAL_reduced_eb = (RooDataSet*) eb_clone->reduce(vars_eb,TString::Format("(genEnergy)>%f && (genEnergy)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* ECAL_reduced_ee = (RooDataSet*) ee_clone->reduce(vars_ee,TString::Format("(genEnergy)>%f && (genEnergy)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* TRK_reduced_eb = (RooDataSet*) eb_clone->reduce(vars_eb,TString::Format("(genPt)>%f && (genPt)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* TRK_reduced_ee = (RooDataSet*) ee_clone->reduce(vars_ee,TString::Format("(genPt)>%f && (genPt)<=%f", binning[i], binning[i+1]).Data());

    RooDataSet* ECAL_s_reduced_eb = (RooDataSet*) eb_clone->reduce(RooArgSet(*ECALresponse_eb),TString::Format("(genEnergy)>%f && (genEnergy)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* ECAL_s_reduced_ee = (RooDataSet*) ee_clone->reduce(RooArgSet(*ECALresponse_ee),TString::Format("(genEnergy)>%f && (genEnergy)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* TRK_s_reduced_eb = (RooDataSet*) eb_clone->reduce(RooArgSet(*TRKresponse_eb),TString::Format("(genPt)>%f && (genPt)<=%f", binning[i], binning[i+1]).Data());
    RooDataSet* TRK_s_reduced_ee = (RooDataSet*) ee_clone->reduce(RooArgSet(*TRKresponse_ee),TString::Format("(genPt)>%f && (genPt)<=%f", binning[i], binning[i+1]).Data());

    RooPlot* ECALframe_eb = ECALresponse_eb->frame(-0.6,0.2,60);
    RooPlot* ECALframe_ee = ECALresponse_ee->frame(-0.6,0.2,60);
    RooPlot* TRKframe_eb = TRKresponse_eb->frame(-0.4,0.3,60);
    RooPlot* TRKframe_ee = TRKresponse_ee->frame(-0.4,0.3,60);

    RooRealVar mean("mean", "mean", 0., -0.6, 0.3);
    RooRealVar sigma("sigma", "sigma", 0.05, 0., 10.);
    RooRealVar alpha1("alpha1", "alpha1", 3., 0.02, 10.);
    RooRealVar alpha2("alpha2", "alpha2", 3., 0.02, 10.);
    RooRealVar n1("n1", "n1", 3.0, 1.01, 500.);
    RooRealVar n2("n2", "n2", 3.0, 1.01, 500.);

    RooDoubleCBFast ECALresolution_eb("ECALresolution_eb", "ECALresolution_eb", *ECALresponse_eb, mean, sigma, alpha1, n1, alpha2, n2);  
    RooDoubleCBFast ECALresolution_ee("ECALresolution_ee", "ECALresolution_ee", *ECALresponse_ee, mean, sigma, alpha1, n1, alpha2, n2);  
    RooDoubleCBFast TRKresolution_eb("TRKresolution_eb", "TRKresolution_eb", *TRKresponse_eb, mean, sigma, alpha1, n1, alpha2, n2);  
    RooDoubleCBFast TRKresolution_ee("TRKresolution_ee", "TRKresolution_ee", *TRKresponse_ee, mean, sigma, alpha1, n1, alpha2, n2);  

    ECALresponse_eb->setRange("ECALrange_eb", -0.6,0.2);
    ECALresponse_ee->setRange("ECALrange_ee", -0.6,0.2);
    TRKresponse_eb->setRange("TRKrange_eb", -0.4,0.3);
    TRKresponse_ee->setRange("TRKrange_ee", -0.4,0.3);
    if (ECAL_s_reduced_eb->numEntries() > 2000) { 
      //      mean.setVal(ECAL_s_reduced_eb->mean(*ECALresponse_eb));
      //      sigma.setVal(ECAL_s_reduced_eb->sigma(*ECALresponse_eb));
      mean.setVal(0.);
      sigma.setVal(0.05);
      alpha1.setVal(3.);
      alpha2.setVal(3.);
      n1.setVal(3.);
      n2.setVal(3.);
      ECALresolution_eb.fitTo(*ECAL_s_reduced_eb, Range(-0.6,0.2));
      RooFitResult* fitRes = ECALresolution_eb.fitTo(*ECAL_s_reduced_eb, Save(), Range(-0.6,0.2));    
      TH1 *hist = ECAL_s_reduced_eb->createHistogram("hraw",*ECALresponse_eb,Binning(800,-1.,1.));
      double sigma_eff = effSigma(hist);
      ECALgraph_eb->SetPoint(i, ECAL_reduced_eb->mean(genEnergy), sigma_eff*sigma_eff);
      // if (fitRes->status() == 0) {
      // 	ECALgraph_eb->SetPoint(i, ECAL_reduced_eb->mean(genEnergy), ECALresolution_eb.sigma(*ECALresponse_eb)->getVal()*ECALresolution_eb.sigma(*ECALresponse_eb)->getVal());
      // 	ECALgraph_eb->SetPointError(i, ECAL_reduced_eb->sigma(genEnergy), 2.*sigma.getVal()*sigma.getError());
      // }
      c1->Clear();
      ECAL_s_reduced_eb->plotOn(ECALframe_eb);
      ECALresolution_eb.plotOn(ECALframe_eb, NormRange("ECALrange_eb"));
      ECALframe_eb->Draw();
      c1->Print(TString::Format("figures/ECALresolution_EB_%.0f_%.0f.png", binning[i], binning[i+1]).Data());
    }

    if (TRK_s_reduced_eb->numEntries() > 2000) { 
      //      mean.setVal(TRK_s_reduced_eb->mean(*TRKresponse_eb));
      //      sigma.setVal(TRK_s_reduced_eb->sigma(*TRKresponse_eb));
      mean.setVal(0.);
      sigma.setVal(0.05);
      alpha1.setVal(3.);
      alpha2.setVal(3.);
      n1.setVal(3.);
      n2.setVal(3.);
      TRKresolution_eb.fitTo(*TRK_s_reduced_eb, Range(-0.4,0.3));
      RooFitResult* fitRes = TRKresolution_eb.fitTo(*TRK_s_reduced_eb, Save(), Range(-0.4,0.3));    
      TH1 *hist = TRK_s_reduced_eb->createHistogram("hraw",*TRKresponse_eb,Binning(800,-1.,1.));
      double sigma_eff = effSigma(hist);
      TRKgraph_eb->SetPoint(i, TRK_reduced_eb->mean(genPt), sigma_eff*sigma_eff);
      // if (fitRes->status() == 0) {
      // 	TRKgraph_eb->SetPoint(i, TRK_reduced_eb->mean(genPt), TRKresolution_eb.sigma(*TRKresponse_eb)->getVal()*TRKresolution_eb.sigma(*TRKresponse_eb)->getVal());
      // 	TRKgraph_eb->SetPointError(i, TRK_reduced_eb->sigma(genPt), 2.*sigma.getVal()*sigma.getError());
      // }
      c1->Clear();
      TRK_s_reduced_eb->plotOn(TRKframe_eb);
      TRKresolution_eb.plotOn(TRKframe_eb, NormRange("TRKrange_eb"));
      TRKframe_eb->Draw();
      c1->Print(TString::Format("figures/TRKresolution_EB_%.0f_%.0f.png", binning[i], binning[i+1]).Data());

    }

    if (ECAL_s_reduced_ee->numEntries() > 2000) {
      //      mean.setVal(ECAL_s_reduced_ee->mean(*ECALresponse_ee));
      //      sigma.setVal(ECAL_s_reduced_ee->sigma(*ECALresponse_ee));
      mean.setVal(0.);
      sigma.setVal(0.05);
      alpha1.setVal(3.);
      alpha2.setVal(3.);
      n1.setVal(3.);
      n2.setVal(3.);
      ECALresolution_ee.fitTo(*ECAL_s_reduced_ee, Range(-0.6,0.2));
      RooFitResult* fitRes = ECALresolution_ee.fitTo(*ECAL_s_reduced_ee, Save(), Range(-0.6,0.2));
      TH1 *hist = ECAL_s_reduced_ee->createHistogram("hraw",*ECALresponse_ee,Binning(800,-1.,1.));
      double sigma_eff = effSigma(hist);
      ECALgraph_ee->SetPoint(i, ECAL_reduced_ee->mean(genEnergy), sigma_eff*sigma_eff);
      //      if (fitRes->status() == 0) {	
      //	ECALgraph_ee->SetPoint(i, ECAL_reduced_ee->mean(genEnergy), ECALresolution_ee.sigma(*ECALresponse_ee)->getVal()*ECALresolution_ee.sigma(*ECALresponse_ee)->getVal());
      //	ECALgraph_ee->SetPointError(i, ECAL_reduced_ee->sigma(genEnergy), 2.*sigma.getVal()*sigma.getError());
      //      }
      c1->Clear();
      ECAL_s_reduced_ee->plotOn(ECALframe_ee);
      ECALresolution_ee.plotOn(ECALframe_ee, NormRange("ECALrange_ee"));
      ECALframe_ee->Draw();
      c1->Print(TString::Format("figures/ECALresolution_EE_%.0f_%.0f.png", binning[i], binning[i+1]).Data());
    }

    if (TRK_reduced_ee->numEntries() > 2000) { 
      //      mean.setVal(TRK_s_reduced_ee->mean(*TRKresponse_ee));
      //      sigma.setVal(TRK_s_reduced_ee->sigma(*TRKresponse_ee));
      mean.setVal(0.);
      sigma.setVal(0.05);
      alpha1.setVal(3.);
      alpha2.setVal(3.);
      n1.setVal(3.);
      n2.setVal(3.);
      TRKresolution_ee.fitTo(*TRK_s_reduced_ee, Range(-0.4,0.3));
      RooFitResult* fitRes = TRKresolution_ee.fitTo(*TRK_s_reduced_ee, Save(), Range(-0.4,0.3));
      TH1 *hist = TRK_s_reduced_ee->createHistogram("hraw",*TRKresponse_ee,Binning(800,-1.,1.));
      double sigma_eff = effSigma(hist);
      TRKgraph_ee->SetPoint(i, TRK_reduced_ee->mean(genPt), sigma_eff*sigma_eff);
      // if (fitRes->status() == 0) {
      // 	TRKgraph_ee->SetPoint(i, TRK_reduced_ee->mean(genPt), TRKresolution_ee.sigma(*TRKresponse_ee)->getVal()*TRKresolution_ee.sigma(*TRKresponse_ee)->getVal());
      // 	TRKgraph_ee->SetPointError(i, TRK_reduced_ee->sigma(genPt), 2.*sigma.getVal()*sigma.getError());
      // }
      c1->Clear();
      TRK_s_reduced_ee->plotOn(TRKframe_ee);
      TRKresolution_ee.plotOn(TRKframe_ee, NormRange("TRKrange_ee"));
      TRKframe_ee->Draw();
      c1->Print(TString::Format("figures/TRKresolution_EE_%.0f_%.0f.png", binning[i], binning[i+1]).Data());
    }
  }
  TFile* output = TFile::Open("resolution.root", "RECREATE");
  ECALgraph_eb->Write("ECALgraph_eb");
  ECALgraph_ee->Write("ECALgraph_ee");
  TRKgraph_eb->Write("TRKgraph_eb");
  TRKgraph_ee->Write("TRKgraph_ee");
  output->Close();
}


