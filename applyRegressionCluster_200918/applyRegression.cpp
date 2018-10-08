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
    ("zero,z", value<string>(&supressVariable), "Force variable to zero (for sensitivity studies)")
    ;

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);

  if (vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }

  /*VS
    if (!vm.count("config")) {
    configPrefix = trainingPrefix;
    replace(configPrefix, "../", "../python/");
    }
    VS*/

  double responseMin = 0.7 ;
  double responseMax = 2.1 ;
  double resolutionMin = 0.0002 ;
  double resolutionMax = 0.5 ;

  cout << "Response parameters "  << responseMin << " " << responseMax << endl;

  double responseScale = 0.5*( responseMax - responseMin );
  double responseOffset = responseMin + 0.5*( responseMax - responseMin );

  double resolutionScale = 0.5*( resolutionMax - resolutionMin );
  double resolutionOffset = resolutionMin + 0.5*( resolutionMax - resolutionMin );


  GBRForestD* forest_ecal_scale;
  GBRForestD* forest_ecal_resolution;

  std::vector<TFile*> file_;

  file_.push_back(TFile::Open(TString::Format("%s_%s_ecal_results.root", trainingPrefix.c_str(), region.c_str())));  
  forest_ecal_scale      = (GBRForestD*) file_.back()->Get("EBCorrection");
  forest_ecal_resolution = (GBRForestD*) file_.back()->Get("EBUncertainty");

  ParReader reader_ecal_lowpt; 
  reader_ecal_lowpt.read(TString::Format("%s_%s_ecal.config", configPrefix.c_str(), region.c_str()).Data());

  TFile* testingFile = TFile::Open(testingFileName.c_str());
  TTree* testingTree = (TTree*) testingFile->Get("een_analyzer/ClusterTree");

  TTreeFormula genE("genEnergy", "genEnergy", testingTree);
  TTreeFormula genPt("genPt", "genPt", testingTree);
  TTreeFormula oldE("corrEnergy", "corrEnergy", testingTree);
  TTreeFormula oldEerror("corrEnergyError", "corrEnergyError", testingTree);
  TTreeFormula isEB("isEB", "isEB", testingTree);
  TTreeFormula numberOfClusters("numberOfClusters", "numberOfClusters", testingTree);
  TTreeFormula nrSaturatedCrysIn5x5("nrSaturatedCrysIn5x5", "nrSaturatedCrysIn5x5", testingTree);
  TTreeFormula rawE("rawEnergy+preshowerEnergy", "rawEnergy+preshowerEnergy", testingTree);

  char_separator<char> sep(":");
  std::vector<TTreeFormula*> inputforms_ecal_lowpt;
  { 
    tokenizer<char_separator<char>> tokens(reader_ecal_lowpt.m_regParams[0].variablesEB, sep); 
    for (const auto& it : tokens) 
      inputforms_ecal_lowpt.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree)); 
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
  friendtree->Branch("response", &response, "response/F");
  friendtree->Branch("resolution", &resolution, "resolution/F");
  friendtree->Branch("eOverP", &eOverP, "eOverP/F");
  for (Long64_t iev=0; iev<testingTree->GetEntries(); ++iev) {
    response = 0.;
    resolution = 0.;
    if (iev%100000==0) printf("%i\n",int(iev));
    testingTree->LoadTree(iev);

    if( (region == "EB" && isEB.EvalInstance()) || (region == "EE" && !(isEB.EvalInstance())) ) {

      bool low_ = genPt.EvalInstance() < 300.;
      //    bool high_ = genPt.EvalInstance() >= 300.;
      vals.clear();

      if ( low_ ) { 
	for (auto&& input : inputforms_ecal_lowpt) { 
	  input->GetNdata(); 
	  vals.push_back(input->EvalInstance()); 
	  if(debug) cout << input->GetExpFormula().Data() << " " << input->EvalInstance() << endl; 
	} 
	response = forest_ecal_scale->GetResponse(vals.data()); 
	resolution = forest_ecal_resolution->GetResponse(vals.data());
	if(debug) cout <<" response - " << response << " :::  resolution - "<<  resolution << std::endl;
      }

      response   = responseOffset + responseScale*sin(response);
      resolution = resolutionOffset + resolutionScale*sin(resolution);
      friendtree->Fill();

    }
  }

  // Writes output
  outputFile->cd("een_analyzer");
  friendtree->Write();
  outputFile->Close();

}
