#include "tdrstyle.C"
#include "CMS_lumi.C"

void draw(TH1D* ptBalanceOld, TH1D* ptBalanceNew, TString title, TString filename) {
  TCanvas* canvas = new TCanvas();
  TLegend* legend = new TLegend(0.16, 0.88, 0.98, 0.95);
  ptBalanceNew->SetMarkerColor(kBlue);
  ptBalanceOld->SetTitle(TString::Format("%s;p_{T} imbalance;Events", title.Data()));				
  ptBalanceOld->SetMaximum(1.15*ptBalanceOld->GetMaximum());
  ptBalanceOld->Draw("hist");
  ptBalanceNew->Draw("e0,same");
  legend->AddEntry(ptBalanceNew, "Legacy regression", "lp");
  legend->AddEntry(ptBalanceOld, "74X regression", "lp");
  legend->SetShadowColor(kWhite);
  legend->SetNColumns(2);
  legend->Draw();
  CMS_lumi(canvas, 0, 10);
  canvas->Print(filename+"_lin.png");
  ptBalanceOld->SetMaximum(80*ptBalanceOld->GetMaximum());
  ptBalanceOld->Draw("hist");
  ptBalanceNew->Draw("e0,same");
  legend->Draw();
  canvas->SetLogy();
  CMS_lumi(canvas, 0, 10);
  canvas->Print(filename+"_log.png");
}

void printTails(const char* fileName = "../applyRegression/data_electron_withRegression.root", const char* treeName = "een_analyzer/ElectronTree", const char* directory = "./") {
  setTDRStyle();

  writeExtraText = true;                    // if extra text
  extraText  = "Internal";                  // default extra text is "Preliminary"
  lumi_sqrtS = "36 fb^{-1} (13 TeV)";       // used with iPeriod = 0, e.g. for simulation-only plots (default is an empty string)
    
  TString oldPtName = "corrEnergy74X/cosh(eta)";
  TString newPtName = "e80xECALonly/cosh(eta)";
  TString refPtName = "tp_tagpt";
    
  TFile* file = TFile::Open(fileName);
  TTreeReader reader(treeName, file);
  TTreeFormula refPt("refPt", refPtName, reader.GetTree());
  TTreeFormula oldImbalance("oldImbalance", TString::Format("(%s-%s)/%s", oldPtName.Data(), refPtName.Data(), refPtName.Data()), reader.GetTree());
  TTreeFormula newImbalance("newImbalance", TString::Format("(%s-%s)/%s", newPtName.Data(), refPtName.Data(), refPtName.Data()), reader.GetTree());

  TTreeFormula sieie("full5x5_sigmaIetaIeta", "full5x5_sigmaIetaIeta[0]", reader.GetTree());
  TTreeFormula hoe("hadronicOverEm", "hadronicOverEm", reader.GetTree());

  std::map<TString, TH1D*> histsOld;
  std::map<TString, TH1D*> histsNew;
  
  std::map<TString, TString> categories;
  categories["inclusive"] = "All events";
  categories["EB"] = "Barrel only";
  categories["EE"] = "Endcap only";
  categories["EBlowPT"] = "Barrel, 40 < p_{T} < 70 GeV";
  categories["EBmedPT"] = "Barrel, 70 < p_{T} < 150 GeV";
  categories["EBhighPT"] = "Barrel, p_{T} > 150 GeV";
  categories["EElowPT"] = "Endcap, 40 < p_{T} < 70 GeV";
  categories["EEmedPT"] = "Endcap, 70 < p_{T} < 150 GeV";
  categories["EEhighPT"] = "Endcap, p_{T} > 150 GeV";
  categories["EBlowR9"] = "Barrel, R_{9} < 0.94";
  categories["EBhighR9"] = "Barrel, R_{9} > 0.94";
  categories["EElowR9"] = "Endcap, R_{9} < 0.94";
  categories["EEhighR9"] = "Endcap, R_{9} > 0.94";

  std::map<TString, TTreeFormula*> formulaes;
  formulaes["inclusive"] = new TTreeFormula("inclusive", "abs(eta)<2.5 && tp_tagpt < 500", reader.GetTree());
  formulaes["EB"] = new TTreeFormula("EB", "abs(eta)<1.444 && tp_tagpt < 500", reader.GetTree());
  formulaes["EE"] = new TTreeFormula("EE", "abs(eta)>1.556 && abs(eta)<2.5 && tp_tagpt < 500", reader.GetTree());
  formulaes["EBlowPT"] = new TTreeFormula("EBlowPT", "abs(eta)<1.444 && tp_tagpt > 40. && tp_tagpt < 70.", reader.GetTree());
  formulaes["EBmedPT"] = new TTreeFormula("EBmedPT", "abs(eta)<1.444 && tp_tagpt > 70. && tp_tagpt < 150.", reader.GetTree());
  formulaes["EBhighPT"] = new TTreeFormula("EBhighPT", "abs(eta)<1.444 && tp_tagpt > 150. && tp_tagpt < 500", reader.GetTree());
  formulaes["EElowPT"] = new TTreeFormula("EElowPT", "abs(eta)>1.556 && abs(eta)<2.5 && tp_tagpt > 40. && tp_tagpt < 70.", reader.GetTree());
  formulaes["EEmedPT"] = new TTreeFormula("EEmedPT", "abs(eta)>1.556 && abs(eta)<2.5 && tp_tagpt > 70. && tp_tagpt < 150.", reader.GetTree());
  formulaes["EEhighPT"] = new TTreeFormula("EEhighPT", "abs(eta)>1.556 && abs(eta)<2.5 && tp_tagpt > 150. && tp_tagpt < 500.", reader.GetTree());
  formulaes["EBlowR9"] = new TTreeFormula("EBlowPT", "abs(eta)<1.444 && r9[0]<0.94", reader.GetTree());
  formulaes["EBhighR9"] = new TTreeFormula("EBlowPT", "abs(eta)<1.444 && r9[0]>0.94", reader.GetTree());
  formulaes["EElowR9"] = new TTreeFormula("EEmedPT", "abs(eta)>1.556 && abs(eta)<2.5 && r9[0]<0.94", reader.GetTree());
  formulaes["EEhighR9"] = new TTreeFormula("EEmedPT", "abs(eta)>1.556 && abs(eta)<2.5 && r9[0]>0.94", reader.GetTree());

  for (auto cats : categories) {
    histsOld[cats.first] = new TH1D(cats.first + "_old", cats.second, 100, -4., 4);
    histsNew[cats.first] = new TH1D(cats.first + "_new", cats.second, 100, -4., 4);
  }

  while(reader.Next()) {

    if (sieie.EvalInstance() > 0.012) continue;
    if (hoe.EvalInstance() > 0.1) continue;

    for (auto cats : categories) {
      if (formulaes[cats.first]->EvalInstance()) {
	histsOld[cats.first]->Fill(oldImbalance.EvalInstance());
	histsNew[cats.first]->Fill(newImbalance.EvalInstance());
      }
    }
  }
  
  // Prints
  for (auto cats : categories) 
    draw(histsOld[cats.first], histsNew[cats.first], cats.second, TString::Format("%s/%s", directory, cats.first.Data()));

  
}

void doAll() {

  printTails("../applyRegression/data_electron_withRegression.root", "een_analyzer/ElectronTree", "~/eos/www/Legacy/Tails/Electron/");
  printTails("../applyRegression/data_photon_withRegression.root", "een_analyzer/ElectronTree", "~/eos/www/Legacy/Tails/Photon/");
  
}
