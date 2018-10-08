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
#define debug2 false
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

  string region;
  string configPrefix;
  string trainingPrefix;
  string testingFileName;
  string outputFileName;
  string supressVariable;


  options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print usage message")
    ("region,r", value<string>(&region), "detector region: EB/EE")
    ("config,c", value<string>(&configPrefix), "Configuration prefix")
    ("prefix,p", value<string>(&trainingPrefix), "Training prefix")
    ("testing,t", value<string>(&testingFileName), "Testing tree")
    ("output,o", value<string>(&outputFileName), "Output friend tree")
    ("gsf,g", "Do GSF track regression")
    ("sat,s", "Do not apply saturation regression")
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
  bool applySat = true;
  if (vm.count("sat")) {
    applySat = false;
  }

  if (!vm.count("config")) {
    configPrefix = trainingPrefix;
    replace(configPrefix, "../", "../python/");
  }

  double responseMin = 0.7 ;
  double responseMax = 1.7 ;
  double resolutionMin = 0.0002 ;
  double resolutionMax = 0.5 ;

  double responseMin_sat = -1.0 ;
  double responseMax_sat = 3.0 ;

  cout << "Response parameters "  << responseMin << " " << responseMax << endl;

  double responseScale = 0.5*( responseMax - responseMin );
  double responseOffset = responseMin + 0.5*( responseMax - responseMin );

  double responseScale_sat = 0.5*( responseMax_sat - responseMin_sat );
  double responseOffset_sat = responseMin_sat + 0.5*( responseMax_sat - responseMin_sat );

  double resolutionScale = 0.5*( resolutionMax - resolutionMin );
  double resolutionOffset = resolutionMin + 0.5*( resolutionMax - resolutionMin );

  GBRForestD* forest_ecal_lowpt_scale;
  GBRForestD* forest_ecal_highpt_scale;
  GBRForestD* forest_ecal_satd_highpt_scale;

  GBRForestD* forest_ecal_lowpt_resolution;
  GBRForestD* forest_ecal_highpt_resolution;
  GBRForestD* forest_ecal_satd_highpt_resolution;

  GBRForestD* forest_trk_lowpt_scale;
  GBRForestD* forest_trk_highpt_scale;
  GBRForestD* forest_trk_lowpt_resolution;
  GBRForestD* forest_trk_highpt_resolution;

  std::vector<TFile*> file_;

  if (!doGSF) {

    file_.push_back(TFile::Open(TString::Format("%s_%s_ecal_lowpt_results.root", trainingPrefix.c_str(), region.c_str())));  
    forest_ecal_lowpt_scale      = (GBRForestD*) file_.back()->Get(TString::Format("%sCorrection", region.c_str()));
    forest_ecal_lowpt_resolution = (GBRForestD*) file_.back()->Get(TString::Format("%sUncertainity", region.c_str()));

    file_.push_back(TFile::Open(TString::Format("%s_%s_ecal_highpt_results.root", trainingPrefix.c_str(), region.c_str())));  
    forest_ecal_highpt_scale      = (GBRForestD*) file_.back()->Get(TString::Format("%sCorrection", region.c_str()));
    forest_ecal_highpt_resolution = (GBRForestD*) file_.back()->Get(TString::Format("%sUncertainity", region.c_str()));

    file_.push_back(TFile::Open(TString::Format("%s_%s_ecal_satd_highpt_results.root", trainingPrefix.c_str(), region.c_str())));  
    forest_ecal_satd_highpt_scale      = (GBRForestD*) file_.back()->Get(TString::Format("%sCorrection", region.c_str()));
    forest_ecal_satd_highpt_resolution = (GBRForestD*) file_.back()->Get(TString::Format("%sUncertainity", region.c_str()));

  } else {

    file_.push_back(TFile::Open(TString::Format("%s_%s_trk_lowpt_results.root", trainingPrefix.c_str(), region.c_str())));  
    forest_trk_lowpt_scale      = (GBRForestD*) file_.back()->Get(TString::Format("%sCorrection", region.c_str()));
    forest_trk_lowpt_resolution = (GBRForestD*) file_.back()->Get(TString::Format("%sUncertainity", region.c_str()));

    file_.push_back(TFile::Open(TString::Format("%s_%s_trk_highpt_results.root", trainingPrefix.c_str(), region.c_str())));  
    forest_trk_highpt_scale      = (GBRForestD*) file_.back()->Get(TString::Format("%sCorrection", region.c_str()));
    forest_trk_highpt_resolution = (GBRForestD*) file_.back()->Get(TString::Format("%sUncertainity", region.c_str()));
  }  

  bool isElectron = contains(trainingPrefix, "electron");
  bool isPhoton   = contains(trainingPrefix, "photon");
  if (isElectron && isPhoton) {
    cout << "Sorry, I cannot decide if this parameter file is for electrons or photons" << endl;
    return 1;
  }
  if (isElectron && debug) {
    cout << "Running electron regression" << endl;
  }
  if (isPhoton && debug) {
    cout << "Running photon regression" << endl;
  }

  ParReader reader_ecal_lowpt; 
  reader_ecal_lowpt.read(TString::Format("%s_%s_ecal_lowpt.config", configPrefix.c_str(), region.c_str()).Data());
  ParReader reader_ecal_highpt; 
  reader_ecal_highpt.read(TString::Format("%s_%s_ecal_highpt.config", configPrefix.c_str(), region.c_str()).Data());
  ParReader reader_ecal_satd_highpt; 
  reader_ecal_satd_highpt.read(TString::Format("%s_%s_ecal_satd_highpt.config", configPrefix.c_str(), region.c_str()).Data());

  ParReader reader_trk_lowpt; 
  if (doGSF) reader_trk_lowpt.read(TString::Format("%s_%s_trk_lowpt.config", configPrefix.c_str(), region.c_str()).Data());
  ParReader reader_trk_highpt; 
  if (doGSF) reader_trk_highpt.read(TString::Format("%s_%s_trk_highpt.config", configPrefix.c_str(), region.c_str()).Data());

  TFile* testingFile = TFile::Open(testingFileName.c_str());
  TTree* testingTree = isElectron ? (TTree*) testingFile->Get("een_analyzer/ElectronTree") : (TTree*) testingFile->Get("een_analyzer/PhotonTree");

  TTreeFormula genE("genEnergy", "genEnergy", testingTree);
  TTreeFormula genPt("genPt", "genPt", testingTree);
  TTreeFormula oldE("corrEnergy", "corrEnergy", testingTree);
  TTreeFormula oldEerror("corrEnergyError", "corrEnergyError", testingTree);
  TTreeFormula isEB("isEB", "isEB", testingTree);
  TTreeFormula numberOfClusters("numberOfClusters", "numberOfClusters", testingTree);
  TTreeFormula nrSaturatedCrysIn5x5("nrSaturatedCrysIn5x5", "nrSaturatedCrysIn5x5", testingTree);
  TTreeFormula eOverPuncorr("eOverPuncorr", "eOverPuncorr", testingTree);
  TTreeFormula rawE("rawEnergy+preshowerEnergy", "rawEnergy+preshowerEnergy", testingTree);
  //TTreeFormula rawComb("rawComb", "( 1. / ( (trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + (rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*resolution*resolution) / ( (rawEnergy+preshowerEnergy)*response*trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + trkMomentum*(rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*resolution*resolution ) ) )", testingTree);
  TTreeFormula rawComb("rawComb", "( 1. / ( (trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + (rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*1*1) / ( (rawEnergy+preshowerEnergy)*1*trkMomentum*trkMomentum*trkMomentumRelError*trkMomentumRelError + trkMomentum*(rawEnergy+preshowerEnergy)*(rawEnergy+preshowerEnergy)*1*1 ) ) )", testingTree);

  char_separator<char> sep(":");
  std::vector<TTreeFormula*> inputforms_ecal_lowpt;
  if(region == "EB"){ 
    tokenizer<char_separator<char>> tokens(reader_ecal_lowpt.m_regParams[0].variablesEB, sep); 
    for (const auto& it : tokens) inputforms_ecal_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }else if(region == "EE"){
    tokenizer<char_separator<char>> tokens(reader_ecal_lowpt.m_regParams[0].variablesEE, sep); 
    for (const auto& it : tokens) inputforms_ecal_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }

  std::vector<TTreeFormula*> inputforms_ecal_highpt;
  if(region == "EB"){ 
    tokenizer<char_separator<char>> tokens(reader_ecal_highpt.m_regParams[0].variablesEB, sep); 
    for (const auto& it : tokens) inputforms_ecal_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }else if(region == "EE"){
    tokenizer<char_separator<char>> tokens(reader_ecal_highpt.m_regParams[0].variablesEE, sep); 
    for (const auto& it : tokens) inputforms_ecal_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }

  std::vector<TTreeFormula*> inputforms_ecal_satd_highpt;
  if(region == "EB"){ 
    tokenizer<char_separator<char>> tokens(reader_ecal_satd_highpt.m_regParams[0].variablesEB, sep); 
    for (const auto& it : tokens) inputforms_ecal_satd_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }else if(region == "EE"){
    tokenizer<char_separator<char>> tokens(reader_ecal_satd_highpt.m_regParams[0].variablesEE, sep); 
    for (const auto& it : tokens) inputforms_ecal_satd_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
  }

  std::vector<TTreeFormula*> inputforms_trk_lowpt;
  std::vector<TTreeFormula*> inputforms_trk_highpt;

  if (doGSF) {
    if(region == "EB"){ 
      tokenizer<char_separator<char>> tokens(reader_trk_lowpt.m_regParams[0].variablesEB, sep); 
      for (const auto& it : tokens) inputforms_trk_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
    }else if(region == "EE"){
      tokenizer<char_separator<char>> tokens(reader_trk_lowpt.m_regParams[0].variablesEE, sep); 
      for (const auto& it : tokens) inputforms_trk_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
    }
    if(region == "EB"){ 
      tokenizer<char_separator<char>> tokens(reader_trk_highpt.m_regParams[0].variablesEB, sep); 
      for (const auto& it : tokens) inputforms_trk_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
    }else if(region == "EE"){
      tokenizer<char_separator<char>> tokens(reader_trk_highpt.m_regParams[0].variablesEE, sep); 
      for (const auto& it : tokens) inputforms_trk_highpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
    }
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

  //for (Long64_t iev=0; iev<testingTree->GetEntries(); ++iev) {
  for (Long64_t iev=4; iev<100; ++iev) {

    response = 0.;
    resolution = 0.;
    if (iev%100000==0) printf("%i\n",int(iev));
    testingTree->LoadTree(iev);

    if( (region == "EB" && isEB.EvalInstance()) || (region == "EE" && !(isEB.EvalInstance())) ) {

    bool EB_ = isEB.EvalInstance();
    bool EE_ = !EB_;
    bool sat_ = nrSaturatedCrysIn5x5.EvalInstance() > 0;

    if (!doGSF) {
      bool low_ = genPt.EvalInstance() < 300.;
      bool high_ = genPt.EvalInstance() >= 300.;
      vals.clear();

      if (debug) {
	cout << "EB: " << EB_ << endl
	  << "EE: " << EE_ << endl
	  << "low: " << low_ << endl
	  << "high: " << high_ << endl
	  << "sat: " << sat_ << endl;
      }
      std::cout<<" low_ = " << low_ << "    high_ = "<< high_ << "    sat_ = "<< sat_ <<std::endl;
      if (low_ && !sat_){ 
	for (auto&& input : inputforms_ecal_lowpt){ 
	  input->GetNdata(); 
	  vals.push_back(input->EvalInstance()); 
	  if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	} 
	response   = forest_ecal_lowpt_scale->GetResponse(vals.data()); 
	resolution = forest_ecal_lowpt_resolution->GetResponse(vals.data()); 
      } else if (high_ && !sat_){ 
	for (auto&& input : inputforms_ecal_highpt){ 
	  input->GetNdata(); 
	  vals.push_back(input->EvalInstance()); 
	  if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	} 
	response   = forest_ecal_highpt_scale->GetResponse(vals.data()); 
	resolution = forest_ecal_highpt_resolution->GetResponse(vals.data()); 
      } else if (sat_){ 
    std::cout<< " CHECK 1 "<< std::endl;
	for (auto&& input : inputforms_ecal_satd_highpt){
	  input->GetNdata();
	  vals.push_back(input->EvalInstance()); 
	  if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	} 
    std::cout<< " CHECK 2 "<< std::endl;
	response   = forest_ecal_satd_highpt_scale->GetResponse(vals.data()); 
    std::cout<< " CHECK 3 "<< std::endl;
	resolution = forest_ecal_satd_highpt_resolution->GetResponse(vals.data()); 
    std::cout<< " CHECK 4 "<< std::endl;
      }
    std::cout<< " CHECK 5 "<< std::endl;
    } else { // doGSF
      bool low_ = genPt.EvalInstance() < 50.;
      bool high_ = !low_;
      vals.clear();

      if (genPt.EvalInstance() > 200.){ 
	response = 1.; 
	resolution = 0. ; 
      } else if ( low_) { 
	for (auto&& input : inputforms_trk_lowpt){ 
	  input->GetNdata(); 
	  vals.push_back(input->EvalInstance()); 
	  if (debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	}
	response   = forest_trk_lowpt_scale->GetResponse(vals.data()); 
	resolution = forest_trk_lowpt_resolution->GetResponse(vals.data()); 
      } else if (high_) { 
	for (auto&& input : inputforms_trk_highpt){ 
	  input->GetNdata(); 
	  vals.push_back(input->EvalInstance()); 
	  if (debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	}
	response   = forest_trk_highpt_scale->GetResponse(vals.data()); 
	resolution = forest_trk_highpt_resolution->GetResponse(vals.data()); 
      }
    }

    if (debug) cout << "raw response " << response << endl << "raw resolution " << resolution << endl;

    response = sat_? responseOffset_sat + responseScale_sat*sin(response) : responseOffset + responseScale*sin(response);
    resolution = resolutionOffset + resolutionScale*sin(resolution);

    if (debug && !doGSF) cout << "raw + PS " << rawE.EvalInstance() << endl;
    if (debug && doGSF) cout << "raw Comb " << rawComb.EvalInstance() << endl;
    if (debug) cout << "response " << response << endl << "resolution " << resolution << endl;
    if (debug) cout << "74X response " << oldE.EvalInstance()/rawE.EvalInstance() << endl << "74X resolution " << oldEerror.EvalInstance()/oldE.EvalInstance() << endl;
    if (debug2) { 
      if (!doGSF) {
	if ( std::abs(oldE.EvalInstance()/rawE.EvalInstance() - genE.EvalInstance()/rawE.EvalInstance()) < std::abs(response - genE.EvalInstance()/rawE.EvalInstance()) ) { std::cout << "WORSE" << std::endl; iworse++; } else { std::cout << "BETTER" << std::endl; ibetter++; }
	cout << ibetter << " -- " << iworse << endl;
      } else {
	if ( std::abs(oldE.EvalInstance()/rawE.EvalInstance() - genE.EvalInstance()/rawE.EvalInstance()) < std::abs(response - genE.EvalInstance()/rawComb.EvalInstance()) ) { std::cout << "WORSE" << std::endl; iworse++; } else { std::cout << "BETTER" << std::endl; ibetter++; }
	cout << ibetter << " -- " << iworse << endl;
      }	
    }
    if (debug && !doGSF) cout << "true response " << genE.EvalInstance()/rawE.EvalInstance() << endl;
    if (debug && doGSF) cout << "true response " << genE.EvalInstance()/rawComb.EvalInstance() << endl;
    if (testing && !doGSF) response = genE.EvalInstance()/rawE.EvalInstance();
    if (testing && doGSF) response = genE.EvalInstance()/rawComb.EvalInstance();

    if (!doGSF) eOverP = eOverPuncorr.EvalInstance()*response;

    friendtree->Fill();
    }
  }

  // Writes output
  outputFile->cd("een_analyzer");
  friendtree->Write();
  outputFile->Close();

}
