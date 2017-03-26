#include "TMath.h"

void draw() {
  TFile* input = TFile::Open("resolution.root");
  TGraphErrors* ECALgraph_eb = (TGraphErrors*) input->Get("ECALgraph_eb");
  TGraphErrors* ECALgraph_ee = (TGraphErrors*) input->Get("ECALgraph_ee");
  TGraphErrors* TRKgraph_eb = (TGraphErrors*) input->Get("TRKgraph_eb");
  TGraphErrors* TRKgraph_ee = (TGraphErrors*) input->Get("TRKgraph_ee");

  //  TF1* ECALfnc_eb = new TF1("ECALfnc_eb", "[0]+[1]/x+[2]/(x*x)+[3]/(x*x*x)", 5, 350);
  TF1* ECALfnc_eb = new TF1("ECALfnc_eb", "[1]/(x)+[2]/(x*x)", 5, 350);
  TF1* ECALfnc_ee = new TF1("ECALfnc_ee", "[0]/(1+[1]*x)+[2]*exp(-[3]*x)", 10, 350);
  TF1* TRKfnc_eb = new TF1("TRKfnc_eb", "[0]*sqrt(x)+[1]*x+[2]*x*x", 5, 350);
  TF1* TRKfnc_ee = new TF1("TRKfnc_ee", "[0]*sqrt(x)+[1]*x+[2]*x*x", 5, 350);

  ECALfnc_eb->SetParameters(0., 0.1, 0.1);
  ECALfnc_eb->SetParLimits(0, 0., 1.);
  ECALfnc_eb->SetParLimits(1, 0., 1.);
  ECALfnc_ee->SetParameters(0., 0.1, 0.1, 2);
  ECALfnc_ee->SetParLimits(0, 0., 1.);
  ECALfnc_ee->SetParLimits(1, 0., 1.);
  ECALfnc_ee->SetParLimits(2, -10., 10.);
  ECALfnc_ee->SetParLimits(3, 0., 1.);
  TRKfnc_eb->SetParameters(0.005, 0.005, 0.03, 0.3);
  TRKfnc_ee->SetParameters(0.005, 0.005, 0.03, 0.3);

  ECALgraph_eb->Fit(ECALfnc_eb, "RV");
  ECALgraph_ee->Fit(ECALfnc_ee, "RV");
  ECALgraph_eb->Fit(ECALfnc_eb, "RV");
  ECALgraph_ee->Fit(ECALfnc_ee, "RV");
  TRKgraph_eb->Fit(TRKfnc_eb, "RV");
  TRKgraph_ee->Fit(TRKfnc_ee, "RV");
  TRKgraph_eb->Fit(TRKfnc_eb, "RV");
  TRKgraph_ee->Fit(TRKfnc_ee, "RV");

  gStyle->SetOptFit(1111);
  TCanvas* c1 = new TCanvas();

  c1->Clear();
  ECALgraph_eb->Draw("A");
  ECALgraph_eb->SetMaximum(0.01);
  ECALgraph_eb->GetXaxis()->SetRangeUser(5,350);
  ECALgraph_eb->GetXaxis()->SetTitle("EB Gen Energy [GeV]");
  ECALgraph_eb->GetYaxis()->SetTitle("#sigma_{E}^{2}/E^{2}");
  ECALgraph_eb->SetMarkerStyle(20);
  ECALgraph_eb->Draw("AP");
  c1->Print("figures/ECALgraph_eb.png");

  c1->Clear();
  ECALgraph_ee->Draw("A");
  ECALgraph_ee->GetXaxis()->SetRangeUser(5,350);
  ECALgraph_ee->GetXaxis()->SetTitle("EE Gen Energy [GeV]");
  ECALgraph_ee->GetYaxis()->SetTitle("#sigma_{E}^{2}/E^{2}");
  ECALgraph_ee->SetMarkerStyle(20);
  ECALgraph_ee->Draw("AP");
  c1->Print("figures/ECALgraph_ee.png");

  c1->Clear();
  TRKgraph_eb->Draw("A");
  TRKgraph_eb->GetXaxis()->SetRangeUser(5,350);
  TRKgraph_eb->GetXaxis()->SetTitle("EB Gen p_{T} [GeV]");
  TRKgraph_eb->GetYaxis()->SetTitle("#sigma_{p_{T}}^{2}/p_T^{2}");
  TRKgraph_eb->SetMarkerStyle(20);
  TRKgraph_eb->Draw("AP");
  c1->Print("figures/TRKgraph_eb.png");

  c1->Clear();
  TRKgraph_ee->Draw("A");
  TRKgraph_ee->GetXaxis()->SetRangeUser(5,350);
  TRKgraph_ee->GetXaxis()->SetTitle("EE Gen p_{T} [GeV]");
  TRKgraph_ee->GetYaxis()->SetTitle("#sigma_{p_{T}}^{2}/p_T^{2}");
  TRKgraph_ee->SetMarkerStyle(20);
  TRKgraph_ee->Draw("AP");
  c1->Print("figures/TRKgraph_ee.png");

}
