#include "CondFormats/EgammaObjects/interface/GBRForest.h"
#include "CondFormats/EgammaObjects/interface/GBRForestD.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeFormula.h"
#include "ParReader.h"

// Needed for randomly assigned weight
#include "TRandom.h"
#include "TF1.h"
#include "TMath.h"

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <iterator>
#include <typeinfo>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <array>

using namespace std;
using namespace boost;
using namespace boost::program_options;
using namespace boost::filesystem;

#define debug false

bool replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

int main(int argc, char** argv) {

  string configPrefix;
  string trainingPrefix;
  string testingFileName;
  string outputFileName;
  string supressVariable;
  

  options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print usage message")
    ("config,c", value<string>(&configPrefix), "Configuration prefix")
    ("prefix,p", value<string>(&trainingPrefix), "Training prefix")
    ("testing,t", value<string>(&testingFileName), "Testing tree")
    ("output,o", value<string>(&outputFileName), "Output friend tree")
    ;

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
  
  if (vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }

  if (!vm.count("config")) {
    configPrefix = trainingPrefix;
    replace(configPrefix, "../", "../python/");
  }

  double responseMin = -0.336 ;
  double responseMax = 0.916 ;
  double resolutionMin = 0.001 ;
  double resolutionMax = 0.4 ;

  cout << "Response parameters "  << responseMin << " " << responseMax << endl;

  double responseScale = 0.5*( responseMax - responseMin );
  double responseOffset = responseMin + 0.5*( responseMax - responseMin );

  double resolutionScale = 0.5*( resolutionMax - resolutionMin );
  double resolutionOffset = resolutionMin + 0.5*( resolutionMax - resolutionMin );

  GBRForestD* forest_EB_full_pt1_scale;
  GBRForestD* forest_EB_full_pt1_resolution;
  GBRForestD* forest_EB_full_pt2_scale;
  GBRForestD* forest_EB_full_pt2_resolution;
  GBRForestD* forest_EB_full_pt3_scale;
  GBRForestD* forest_EB_full_pt3_resolution;

  GBRForestD* forest_EE_full_pt1_scale;
  GBRForestD* forest_EE_full_pt1_resolution;
  GBRForestD* forest_EE_full_pt2_scale;
  GBRForestD* forest_EE_full_pt2_resolution;
  GBRForestD* forest_EE_full_pt3_scale;
  GBRForestD* forest_EE_full_pt3_resolution;

  GBRForestD* forest_EB_zs_scale;
  GBRForestD* forest_EB_zs_resolution;

  GBRForestD* forest_EE_zs_scale;
  GBRForestD* forest_EE_zs_resolution;

  std::vector<TFile*> file_;

  file_.push_back(TFile::Open(TString::Format("%s_EB_full_pt1_results.root", trainingPrefix.c_str())));
  forest_EB_full_pt1_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
  forest_EB_full_pt1_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
  file_.push_back(TFile::Open(TString::Format("%s_EB_full_pt2_results.root", trainingPrefix.c_str())));
  forest_EB_full_pt2_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
  forest_EB_full_pt2_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
  file_.push_back(TFile::Open(TString::Format("%s_EB_full_pt3_results.root", trainingPrefix.c_str())));
  forest_EB_full_pt3_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
  forest_EB_full_pt3_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");

  file_.push_back(TFile::Open(TString::Format("%s_EE_full_pt1_results.root", trainingPrefix.c_str())));
  forest_EE_full_pt1_scale = (GBRForestD*) file_.back()->Get("EECorrection");
  forest_EE_full_pt1_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
  file_.push_back(TFile::Open(TString::Format("%s_EE_full_pt2_results.root", trainingPrefix.c_str())));
  forest_EE_full_pt2_scale = (GBRForestD*) file_.back()->Get("EECorrection");
  forest_EE_full_pt2_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
  file_.push_back(TFile::Open(TString::Format("%s_EE_full_pt3_results.root", trainingPrefix.c_str())));
  forest_EE_full_pt3_scale = (GBRForestD*) file_.back()->Get("EECorrection");
  forest_EE_full_pt3_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");

  file_.push_back(TFile::Open(TString::Format("%s_EB_zs_results.root", trainingPrefix.c_str())));
  forest_EB_zs_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
  forest_EB_zs_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");

  file_.push_back(TFile::Open(TString::Format("%s_EE_zs_results.root", trainingPrefix.c_str())));
  forest_EE_zs_scale = (GBRForestD*) file_.back()->Get("EECorrection");
  forest_EE_zs_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");

  ParReader reader_EB_full_pt1; reader_EB_full_pt1.read(TString::Format("%s_EB_full_pt1.config", configPrefix.c_str()).Data());
  ParReader reader_EB_full_pt2; reader_EB_full_pt2.read(TString::Format("%s_EB_full_pt2.config", configPrefix.c_str()).Data());
  ParReader reader_EB_full_pt3; reader_EB_full_pt3.read(TString::Format("%s_EB_full_pt3.config", configPrefix.c_str()).Data());

  ParReader reader_EE_full_pt1; reader_EE_full_pt1.read(TString::Format("%s_EE_full_pt1.config", configPrefix.c_str()).Data());
  ParReader reader_EE_full_pt2; reader_EE_full_pt2.read(TString::Format("%s_EE_full_pt2.config", configPrefix.c_str()).Data());
  ParReader reader_EE_full_pt3; reader_EE_full_pt3.read(TString::Format("%s_EE_full_pt3.config", configPrefix.c_str()).Data());

  ParReader reader_EB_zs; reader_EB_zs.read(TString::Format("%s_EB_zs.config", configPrefix.c_str()).Data());

  ParReader reader_EE_zs; reader_EE_zs.read(TString::Format("%s_EE_zs.config", configPrefix.c_str()).Data());

  TFile* testingFile = TFile::Open(testingFileName.c_str());
  TTree* testingTree = (TTree*) testingFile->Get("een_analyzer/PfTree");

  TTreeFormula clusrawE("clusrawE", "clusrawE", testingTree);
  TTreeFormula cluscorrE("cluscorrE", "cluscorrE", testingTree);
  TTreeFormula genEnergy("genEnergy", "genEnergy", testingTree);
  TTreeFormula clusEta("clusEta", "clusEta", testingTree);
  TTreeFormula clusPS1("clusPS1", "clusPS1", testingTree);
  TTreeFormula clusPS2("clusPS2", "clusPS2", testingTree);
  TTreeFormula clusFlag("clusFlag", "clusFlag", testingTree);
  TTreeFormula clusLayer("clusLayer", "clusLayer", testingTree);
  
  char_separator<char> sep(":");

  std::vector<TTreeFormula*> inputforms_EB_full_pt1;
  { tokenizer<char_separator<char>> tokens(reader_EB_full_pt1.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_full_pt1.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_full_pt2;
  { tokenizer<char_separator<char>> tokens(reader_EB_full_pt2.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_full_pt2.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_full_pt3;
  { tokenizer<char_separator<char>> tokens(reader_EB_full_pt3.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_full_pt3.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }

  std::vector<TTreeFormula*> inputforms_EE_full_pt1;
  { tokenizer<char_separator<char>> tokens(reader_EE_full_pt1.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_full_pt1.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_full_pt2;
  { tokenizer<char_separator<char>> tokens(reader_EE_full_pt2.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_full_pt2.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_full_pt3;
  { tokenizer<char_separator<char>> tokens(reader_EE_full_pt3.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_full_pt3.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }

  std::vector<TTreeFormula*> inputforms_EB_zs;
  { tokenizer<char_separator<char>> tokens(reader_EB_zs.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_zs.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }

  std::vector<TTreeFormula*> inputforms_EE_zs;
  { tokenizer<char_separator<char>> tokens(reader_EE_zs.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_zs.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }


  std::vector<float> vals;
  Float_t response = 0.;
  Float_t resolution = 0.;

  //initialize new friend tree
  TFile* outputFile = TFile::Open(outputFileName.c_str(), "RECREATE");
  outputFile->mkdir("een_analyzer");
  outputFile->cd("een_analyzer");

  TTree *friendtree = new TTree("correction", "correction");
  friendtree->Branch("response", &response, "response/F");
  friendtree->Branch("resolution", &resolution, "resolution/F");

  for (Long64_t iev=0; iev<testingTree->GetEntries(); ++iev) {

    response = 0.;
    resolution = 0.;
    if (iev%100000==0) printf("%i\n",int(iev));
    testingTree->LoadTree(iev);

    bool EB_ = (clusLayer.EvalInstance() == -1);
    bool EE_ = (clusLayer.EvalInstance() == -2);
    bool pt1_ = clusrawE.EvalInstance()/TMath::CosH(clusEta.EvalInstance()) <= 4.5;
    bool pt2_ = clusrawE.EvalInstance()/TMath::CosH(clusEta.EvalInstance()) <= 18. && clusrawE.EvalInstance()/TMath::CosH(clusEta.EvalInstance()) > 4.5;
    bool pt3_ = clusrawE.EvalInstance()/TMath::CosH(clusEta.EvalInstance()) > 18.;
    bool zs_ = clusFlag.EvalInstance() == 1;
    vals.clear();

    if (EB_ && zs_) { for (auto&& input : inputforms_EB_zs) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EB_zs_scale->GetResponse(vals.data()); resolution = forest_EB_zs_resolution->GetResponse(vals.data()); }

    else if (EE_ && zs_) { for (auto&& input : inputforms_EE_zs) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EE_zs_scale->GetResponse(vals.data()); resolution = forest_EE_zs_resolution->GetResponse(vals.data()); }

    else if (EB_ && !zs_ && pt1_) { for (auto&& input : inputforms_EB_full_pt1) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EB_full_pt1_scale->GetResponse(vals.data()); resolution = forest_EB_full_pt1_resolution->GetResponse(vals.data()); }
    else if (EB_ && !zs_ && pt2_) { for (auto&& input : inputforms_EB_full_pt2) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EB_full_pt2_scale->GetResponse(vals.data()); resolution = forest_EB_full_pt2_resolution->GetResponse(vals.data()); }
    else if (EB_ && !zs_ && pt3_) { for (auto&& input : inputforms_EB_full_pt3) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EB_full_pt3_scale->GetResponse(vals.data()); resolution = forest_EB_full_pt3_resolution->GetResponse(vals.data()); }

    else if (EE_ && !zs_ && pt1_) { for (auto&& input : inputforms_EE_full_pt1) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EE_full_pt1_scale->GetResponse(vals.data()); resolution = forest_EE_full_pt1_resolution->GetResponse(vals.data()); }
    else if (EE_ && !zs_ && pt2_) { for (auto&& input : inputforms_EE_full_pt2) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EE_full_pt2_scale->GetResponse(vals.data()); resolution = forest_EE_full_pt2_resolution->GetResponse(vals.data()); }
    else if (EE_ && !zs_ && pt3_) { for (auto&& input : inputforms_EE_full_pt3) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
      response = forest_EE_full_pt3_scale->GetResponse(vals.data()); resolution = forest_EE_full_pt3_resolution->GetResponse(vals.data()); }


    if (TMath::Abs(response) > TMath::Pi()/2 ) response = 0.0;
    else response = responseOffset + responseScale*sin(response);
  
    if (TMath::Abs(resolution) > TMath::Pi()/2 ) resolution = 1.0;
    else resolution = resolutionOffset + resolutionScale*sin(resolution);

    response = TMath::Exp(response);
    resolution = TMath::Abs(response)*resolution;
  
    if (debug) cout << "91X response " << response << endl;
    if (EB_) {
      if (debug) cout << "GEN (91X) response " << genEnergy.EvalInstance()/clusrawE.EvalInstance() << endl;
    } else {
      if (debug) cout << "GEN (91X) response " << genEnergy.EvalInstance()/(clusrawE.EvalInstance()+clusPS1.EvalInstance()+clusPS2.EvalInstance()) << endl;
    }
    if (debug) cout << "74X response " << cluscorrE.EvalInstance()/clusrawE.EvalInstance() << endl;
    if (debug) cout << "GEN (74X) response " << genEnergy.EvalInstance()/clusrawE.EvalInstance() << endl;
    
    friendtree->Fill();
  }
    
  // Writes output
  outputFile->cd("een_analyzer");
  friendtree->Write();
  outputFile->Close();
    
    
}
  

