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

#define debug true
#define debug2 true
#define testing false

bool replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

int main(int argc, char** argv) {

  int ibetter = 0;
  int iworse = 0;

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
    ("gsf,g", "Do GSF track regression")
    ("zero,z", value<string>(&supressVariable), "Force variable to zero (for sensitivity studies)")
    ;

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
  
  if (vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }

  bool doGSF = false;
  if (vm.count("gsf")) {
    doGSF = true;
  }

  if (!vm.count("config")) {
    configPrefix = trainingPrefix;
    replace(configPrefix, "../", "../python/");
  }

  double responseMin = 0.5 ;
  double responseMax = 2.1 ;
  double resolutionMin = 0.0002 ;
  double resolutionMax = 0.5 ;

  if (doGSF) {
    responseMin = 0.8;
    responseMax = 1.5;
  }

  cout << "Response parameters "  << responseMin << " " << responseMax << endl;

  double responseScale = 0.5*( responseMax - responseMin );
  double responseOffset = responseMin + 0.5*( responseMax - responseMin );

  double resolutionScale = 0.5*( resolutionMax - resolutionMin );
  double resolutionOffset = resolutionMin + 0.5*( resolutionMax - resolutionMin );

  GBRForestD* forest_EB_ecal_single_lowpt_scale;
  GBRForestD* forest_EB_ecal_single_medpt_scale;
  GBRForestD* forest_EB_ecal_single_highpt_scale;
  GBRForestD* forest_EB_ecal_mult_lowpt_scale;
  GBRForestD* forest_EB_ecal_mult_medpt_scale;
  GBRForestD* forest_EB_ecal_mult_highpt_scale;
  GBRForestD* forest_EB_ecal_sat_scale;

  GBRForestD* forest_EE_ecal_single_lowpt_scale;
  GBRForestD* forest_EE_ecal_single_medpt_scale;
  GBRForestD* forest_EE_ecal_single_highpt_scale;
  GBRForestD* forest_EE_ecal_mult_lowpt_scale;
  GBRForestD* forest_EE_ecal_mult_medpt_scale;
  GBRForestD* forest_EE_ecal_mult_highpt_scale;
  GBRForestD* forest_EE_ecal_sat_scale;

  GBRForestD* forest_EB_ecal_single_lowpt_resolution;
  GBRForestD* forest_EB_ecal_single_medpt_resolution;
  GBRForestD* forest_EB_ecal_single_highpt_resolution;
  GBRForestD* forest_EB_ecal_mult_lowpt_resolution;
  GBRForestD* forest_EB_ecal_mult_medpt_resolution;
  GBRForestD* forest_EB_ecal_mult_highpt_resolution;
  GBRForestD* forest_EB_ecal_sat_resolution;

  GBRForestD* forest_EE_ecal_single_lowpt_resolution;
  GBRForestD* forest_EE_ecal_single_medpt_resolution;
  GBRForestD* forest_EE_ecal_single_highpt_resolution;
  GBRForestD* forest_EE_ecal_mult_lowpt_resolution;
  GBRForestD* forest_EE_ecal_mult_medpt_resolution;
  GBRForestD* forest_EE_ecal_mult_highpt_resolution;
  GBRForestD* forest_EE_ecal_sat_resolution;

  GBRForestD* forest_EB_trk_lowpt_scale;
  GBRForestD* forest_EB_trk_highpt_scale;
  GBRForestD* forest_EE_trk_lowpt_scale;
  GBRForestD* forest_EE_trk_highpt_scale;

  GBRForestD* forest_EB_trk_lowpt_resolution;
  GBRForestD* forest_EB_trk_highpt_resolution;
  GBRForestD* forest_EE_trk_lowpt_resolution;
  GBRForestD* forest_EE_trk_highpt_resolution;

  std::vector<TFile*> file_;

  if (!doGSF) {

    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_single_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_single_lowpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_single_lowpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_single_medpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_single_medpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_single_medpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_single_highpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_single_highpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_single_highpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_mult_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_mult_lowpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_mult_lowpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_mult_medpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_mult_medpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_mult_medpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_mult_highpt_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_mult_highpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_mult_highpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_ecal_sat_results.root", trainingPrefix.c_str())));  
    forest_EB_ecal_sat_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_ecal_sat_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");

    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_single_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_single_lowpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_single_lowpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_single_medpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_single_medpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_single_medpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_single_highpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_single_highpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_single_highpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_mult_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_mult_lowpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_mult_lowpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_mult_medpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_mult_medpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_mult_medpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_mult_highpt_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_mult_highpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_mult_highpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_ecal_sat_results.root", trainingPrefix.c_str())));  
    forest_EE_ecal_sat_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_ecal_sat_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");

  } else {

    file_.push_back(TFile::Open(TString::Format("%s_EB_trk_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EB_trk_lowpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_trk_lowpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EB_trk_highpt_results.root", trainingPrefix.c_str())));  
    forest_EB_trk_highpt_scale = (GBRForestD*) file_.back()->Get("EBCorrection");
    forest_EB_trk_highpt_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_trk_lowpt_results.root", trainingPrefix.c_str())));  
    forest_EE_trk_lowpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_trk_lowpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");
    file_.push_back(TFile::Open(TString::Format("%s_EE_trk_highpt_results.root", trainingPrefix.c_str())));  
    forest_EE_trk_highpt_scale = (GBRForestD*) file_.back()->Get("EECorrection");
    forest_EE_trk_highpt_resolution = (GBRForestD*) file_.back()->Get("EEUncertainty");

  }  

  bool isElectron = contains(trainingPrefix, "electron");
  bool isPhoton = contains(trainingPrefix, "photon");
  if (isElectron && isPhoton) {
    cout << "Sorry, I cannot decide if this parameter file is for electrons of photons" << endl;
    return 1;
  }
  if (isElectron && debug) {
    cout << "Running electron regression" << endl;
  }
  if (isPhoton && debug) {
    cout << "Running photon regression" << endl;
  }

  ParReader reader_EB_ecal_single_lowpt; reader_EB_ecal_single_lowpt.read(TString::Format("%s_EB_ecal_single_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_single_medpt; reader_EB_ecal_single_medpt.read(TString::Format("%s_EB_ecal_single_medpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_single_highpt; reader_EB_ecal_single_highpt.read(TString::Format("%s_EB_ecal_single_highpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_mult_lowpt; reader_EB_ecal_mult_lowpt.read(TString::Format("%s_EB_ecal_mult_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_mult_medpt; reader_EB_ecal_mult_medpt.read(TString::Format("%s_EB_ecal_mult_medpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_mult_highpt; reader_EB_ecal_mult_highpt.read(TString::Format("%s_EB_ecal_mult_highpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_ecal_sat; reader_EB_ecal_sat.read(TString::Format("%s_EB_ecal_sat.config", configPrefix.c_str()).Data());

  ParReader reader_EE_ecal_single_lowpt; reader_EE_ecal_single_lowpt.read(TString::Format("%s_EE_ecal_single_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_single_medpt; reader_EE_ecal_single_medpt.read(TString::Format("%s_EE_ecal_single_medpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_single_highpt; reader_EE_ecal_single_highpt.read(TString::Format("%s_EE_ecal_single_highpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_mult_lowpt; reader_EE_ecal_mult_lowpt.read(TString::Format("%s_EE_ecal_mult_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_mult_medpt; reader_EE_ecal_mult_medpt.read(TString::Format("%s_EE_ecal_mult_medpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_mult_highpt; reader_EE_ecal_mult_highpt.read(TString::Format("%s_EE_ecal_mult_highpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_ecal_sat; reader_EE_ecal_sat.read(TString::Format("%s_EE_ecal_sat.config", configPrefix.c_str()).Data());

  ParReader reader_EB_trk_lowpt; if (doGSF) reader_EB_trk_lowpt.read(TString::Format("%s_EB_trk_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EB_trk_highpt; if (doGSF) reader_EB_trk_highpt.read(TString::Format("%s_EB_trk_highpt.config", configPrefix.c_str()).Data());

  ParReader reader_EE_trk_lowpt; if (doGSF) reader_EE_trk_lowpt.read(TString::Format("%s_EE_trk_lowpt.config", configPrefix.c_str()).Data());
  ParReader reader_EE_trk_highpt; if (doGSF) reader_EE_trk_highpt.read(TString::Format("%s_EE_trk_highpt.config", configPrefix.c_str()).Data());

  TFile* testingFile = TFile::Open(testingFileName.c_str());
  TTree* testingTree = isElectron ? (TTree*) testingFile->Get("een_analyzer/ElectronTree") : (TTree*) testingFile->Get("een_analyzer/PhotonTree");
  
  TTreeFormula genE("genEnergy", "genEnergy", testingTree);
  TTreeFormula genPt("genPt", "genPt", testingTree);
  TTreeFormula oldE("corrEnergy74X", "corrEnergy74X", testingTree);
  TTreeFormula oldEerror("corrEnergy74XError", "corrEnergy74XError", testingTree);
  TTreeFormula isEB("isEB", "isEB", testingTree);
  TTreeFormula numberOfClusters("numberOfClusters", "numberOfClusters", testingTree);
  TTreeFormula nrSaturatedCrysIn5x5("nrSaturatedCrysIn5x5", "nrSaturatedCrysIn5x5", testingTree);
  TTreeFormula eOverPuncorr("eOverPuncorr", "eOverPuncorr", testingTree);
  TTreeFormula rawE("rawEnergy+preshowerEnergy", "rawEnergy+preshowerEnergy", testingTree);
  TTreeFormula rawComb("rawComb", "( genEnergy * (trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + (rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*resolution*resolution) / ( (rawEnergy+preshowerEnergy)*response*trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + trkMomentum*(rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*resolution*resolution ))", testingTree);
  
  char_separator<char> sep(":");
  std::vector<TTreeFormula*> inputforms_EB_ecal_single_lowpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_single_lowpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_single_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_single_medpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_single_medpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_single_medpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_single_highpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_single_highpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_single_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_mult_lowpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_mult_lowpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_mult_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_mult_medpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_mult_medpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_mult_medpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_mult_highpt;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_mult_highpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_mult_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EB_ecal_sat;
  { tokenizer<char_separator<char>> tokens(reader_EB_ecal_sat.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_ecal_sat.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_single_lowpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_single_lowpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_single_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_single_medpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_single_medpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_single_medpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_single_highpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_single_highpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_single_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_mult_lowpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_mult_lowpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_mult_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_mult_medpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_mult_medpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_mult_medpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_mult_highpt;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_mult_highpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_mult_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  std::vector<TTreeFormula*> inputforms_EE_ecal_sat;
  { tokenizer<char_separator<char>> tokens(reader_EE_ecal_sat.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_ecal_sat.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }

  std::vector<TTreeFormula*> inputforms_EB_trk_lowpt;
  std::vector<TTreeFormula*> inputforms_EB_trk_highpt;
  std::vector<TTreeFormula*> inputforms_EE_trk_lowpt;
  std::vector<TTreeFormula*> inputforms_EE_trk_highpt;
  
  if (doGSF) {
    { tokenizer<char_separator<char>> tokens(reader_EB_trk_lowpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_trk_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
    { tokenizer<char_separator<char>> tokens(reader_EB_trk_highpt.m_regParams[0].variablesEB, sep); for (const auto& it : tokens) inputforms_EB_trk_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
    { tokenizer<char_separator<char>> tokens(reader_EE_trk_lowpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_trk_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
    { tokenizer<char_separator<char>> tokens(reader_EE_trk_highpt.m_regParams[0].variablesEE, sep); for (const auto& it : tokens) inputforms_EE_trk_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); }
  }

  std::vector<float> vals;
  Float_t response = 0.;
  Float_t resolution = 0.;
  Float_t eOverP = 0.;

  //initialize new friend tree
  TFile* outputFile = TFile::Open(outputFileName.c_str(), "RECREATE");
  outputFile->mkdir("een_analyzer");
  outputFile->cd("een_analyzer");

  TTree *friendtree = new TTree("correction", "correction");
  if (!doGSF) {
    friendtree->Branch("response", &response, "response/F");
    friendtree->Branch("resolution", &resolution, "resolution/F");
    friendtree->Branch("eOverP", &eOverP, "eOverP/F");
  } else {
    friendtree->Branch("response2", &response, "response2/F");
    friendtree->Branch("resolution2", &resolution, "resolution2/F");
  }
  for (Long64_t iev=0; iev<testingTree->GetEntries(); ++iev) {

    response = 0.;
    resolution = 0.;
    if (iev%100000==0) printf("%i\n",int(iev));
    testingTree->LoadTree(iev);

    if (!doGSF) {
      bool EB_ = isEB.EvalInstance();
      bool EE_ = !EB_;
      bool low_ = genPt.EvalInstance() < 300.;
      bool med_ = genPt.EvalInstance() >= 300. && genPt.EvalInstance() < 1000.;
      bool high_ = genPt.EvalInstance() >= 1000.;
      bool single_ = numberOfClusters.EvalInstance() == 1;
      bool mult_ = !single_;
      bool sat_ = nrSaturatedCrysIn5x5.EvalInstance() > 0;
      vals.clear();

      if (debug) {
	cout << "EB: " << EB_ << endl
	     << "EE: " << EE_ << endl
	     << "low: " << low_ << endl
	     << "med: " << med_ << endl
	     << "high: " << high_ << endl
	     << "single: " << single_ << endl
	     << "mult: " << mult_ << endl
	     << "sat: " << sat_ << endl;
      }

      if (EB_ && low_ && single_ && !sat_) { for (auto&& input : inputforms_EB_ecal_single_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_single_lowpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_single_lowpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && med_ && single_ && !sat_) { for (auto&& input : inputforms_EB_ecal_single_medpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_single_medpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_single_medpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && high_ && single_ && !sat_) { for (auto&& input : inputforms_EB_ecal_single_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_single_highpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_single_highpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && low_ && mult_ && !sat_) { for (auto&& input : inputforms_EB_ecal_mult_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_mult_lowpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_mult_lowpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && med_ && mult_ && !sat_) { for (auto&& input : inputforms_EB_ecal_mult_medpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_mult_medpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_mult_medpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && high_ && mult_ && !sat_) { for (auto&& input : inputforms_EB_ecal_mult_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_mult_highpt_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_mult_highpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && sat_) { for (auto&& input : inputforms_EB_ecal_sat) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EB_ecal_sat_scale->GetResponse(vals.data()); resolution = forest_EB_ecal_sat_resolution->GetResponse(vals.data()); }
      else if (EE_ && low_ && single_ && !sat_) { for (auto&& input : inputforms_EE_ecal_single_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_single_lowpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_single_lowpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && med_ && single_ && !sat_) { for (auto&& input : inputforms_EE_ecal_single_medpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_single_medpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_single_medpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && high_ && single_ && !sat_) { for (auto&& input : inputforms_EE_ecal_single_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_single_highpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_single_highpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && low_ && mult_ && !sat_) { for (auto&& input : inputforms_EE_ecal_mult_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_mult_lowpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_mult_lowpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && med_ && mult_ && !sat_) { for (auto&& input : inputforms_EE_ecal_mult_medpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_mult_medpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_mult_medpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && high_ && mult_ && !sat_) { for (auto&& input : inputforms_EE_ecal_mult_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_mult_highpt_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_mult_highpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && sat_) { for (auto&& input : inputforms_EE_ecal_sat) { input->GetNdata(); vals.push_back(input->EvalInstance()); if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; } 
	response = forest_EE_ecal_sat_scale->GetResponse(vals.data()); resolution = forest_EE_ecal_sat_resolution->GetResponse(vals.data()); }
    } else {
      bool EB_ = isEB.EvalInstance();
      bool EE_ = !EB_;
      bool low_ = genPt.EvalInstance() < 50.;
      bool high_ = !low_;
      vals.clear();

      if (genPt.EvalInstance() > 200.) { response = 1.; resolution = 0. ; } 
      else if (EB_ && low_) { for (auto&& input : inputforms_EB_trk_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); }
	response = forest_EB_trk_lowpt_scale->GetResponse(vals.data()); resolution = forest_EB_trk_lowpt_resolution->GetResponse(vals.data()); }
      else if (EB_ && high_) { for (auto&& input : inputforms_EB_trk_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); }
	response = forest_EB_trk_highpt_scale->GetResponse(vals.data()); resolution = forest_EB_trk_highpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && low_) { for (auto&& input : inputforms_EE_trk_lowpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); }
	response = forest_EE_trk_lowpt_scale->GetResponse(vals.data()); resolution = forest_EE_trk_lowpt_resolution->GetResponse(vals.data()); }
      else if (EE_ && high_) { for (auto&& input : inputforms_EE_trk_highpt) { input->GetNdata(); vals.push_back(input->EvalInstance()); }
	response = forest_EE_trk_highpt_scale->GetResponse(vals.data()); resolution = forest_EE_trk_highpt_resolution->GetResponse(vals.data()); }

    }

    if (TMath::Abs(response) > TMath::Pi()/2 ) response = 1.0;
    else response = responseOffset + responseScale*sin(response);

    if (TMath::Abs(resolution) > TMath::Pi()/2 ) resolution = 1.0;
    else resolution = resolutionOffset + resolutionScale*sin(resolution);

    if (debug) cout << "raw + PS " << rawE.EvalInstance() << endl;
    if (debug) cout << "response " << response << endl << "resolution " << resolution << endl;
    if (debug) cout << "74X response " << oldE.EvalInstance()/rawE.EvalInstance() << endl << "74X resolution " << oldEerror.EvalInstance()/oldE.EvalInstance() << endl;
    if (debug2) { 
      if ( std::abs(oldE.EvalInstance()/rawE.EvalInstance() - genE.EvalInstance()/rawE.EvalInstance()) < std::abs(response - genE.EvalInstance()/rawE.EvalInstance()) ) { std::cout << "WORSE" << std::endl; iworse++; } else { std::cout << "BETTER" << std::endl; ibetter++; }
      cout << ibetter << " -- " << iworse << endl;
    }
    if (debug && !doGSF) cout << "true response " << genE.EvalInstance()/rawE.EvalInstance() << endl;
    if (debug && doGSF) cout << "true response " << genE.EvalInstance()/rawComb.EvalInstance() << endl;
    if (testing && !doGSF) response = genE.EvalInstance()/rawE.EvalInstance();
    if (testing && doGSF) response = genE.EvalInstance()/rawComb.EvalInstance();

    if (!doGSF) eOverP = eOverPuncorr.EvalInstance()*response;

    friendtree->Fill();
      
  }
    
  // Writes output
  outputFile->cd("een_analyzer");
  friendtree->Write();
  outputFile->Close();
    
    
}
  

