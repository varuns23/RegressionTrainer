#include "CondFormats/EgammaObjects/interface/GBRForest.h"
#include "CondFormats/EgammaObjects/interface/GBRForestD.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeFormula.h"
#include "ParReader.h"

// Needed for randomly assigned weight
#include "TRandom.h"
#include "TF1.h"

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <iterator>
#include <typeinfo>
#include <cstring>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace boost;
using namespace boost::program_options;
using namespace boost::filesystem;

#define debug false
#define testing false

#define saveTRKvarse true


bool replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

//#######################################
// Main
//#######################################

int main(int argc, char** argv) {
  
  double responseMin = -1.0 ;
  double responseMax = 3.0 ;
  double resolutionMin = 0.0002 ;
  double resolutionMax = 0.5 ;


  double responseScale = 0.5*( responseMax - responseMin );
  double responseOffset = responseMin + 0.5*( responseMax - responseMin );

  double resolutionScale = 0.5*( resolutionMax - resolutionMin );
  double resolutionOffset = resolutionMin + 0.5*( resolutionMax - resolutionMin );


  string parameterFile;
  string testingFileName;
  string outputFileName;
  string trainingEB;
  string trainingEE;
  string trainingEBhighpt;
  string trainingEEhighpt;
  string supressVariable;
  

  float limhighpt ;
  options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print usage message")
    ("parameter,p", value<string>(&parameterFile), "Parameter file")
    ("barrel,b", value<string>(&trainingEB), "EB training")
    ("endcap,e", value<string>(&trainingEE), "EE training")
    ("barrelhighpt,c", value<string>(&trainingEBhighpt), "EB training highpt")
    ("endcaphighpt,f", value<string>(&trainingEEhighpt), "EE training highpt")
    ("testing,t", value<string>(&testingFileName), "Testing tree")
    ("output,o", value<string>(&outputFileName), "Output friend tree")
    ("zero,z", value<string>(&supressVariable), "Force variable to zero (for sensitivity studies)")
    ("limhighpt,l", value<float>(&limhighpt), "Boundary for high pT")
    ;

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
  
  if (vm.count("help") || !vm.count("parameter")) {
    cout << desc << "\n";
    return 1;
  }

  if (!vm.count("limhightpt")) limhighpt = 300.;

  if (!vm.count("testing")) {
    testingFileName = getenv("CMSSW_BASE");
    testingFileName = testingFileName + "/src/NTuples/Ntup_Jul22_fullpt_testing.root";
  }

  if (!vm.count("output")) {
    outputFileName = parameterFile;
    replace(outputFileName, "python/", "");
    replace(outputFileName, "_EB", "");
    replace(outputFileName, "_EE", "");
    replace(outputFileName, ".config", "_application.root");
  }
  
  bool doEB = vm.count("barrel");
  bool doEE = vm.count("endcap");
  bool doEBhighpt = vm.count("barrelhighpt");
  bool doEEhighpt = vm.count("endcaphighpt");

  if (!doEB) {
    trainingEB = parameterFile;
    replace(trainingEB, "python/", "");
    replace(trainingEB, "_EE", "_EB");
    replace(trainingEB, ".config", "_results.root");
    doEB = true;
  }

  if (!doEE) {
    trainingEE = parameterFile;
    replace(trainingEE, "python/", "");
    replace(trainingEE, "_EB", "_EE");
    replace(trainingEE, ".config", "_results.root");
    doEE = true;
  }

  if (!exists(trainingEB)) doEB = false;
  if (!exists(trainingEE)) doEE = false;
  
  if (doEB || doEE) { 

    GBRForestD* forestEBresponse;
    GBRForestD* forestEBresolution;
    GBRForestD* forestEEresponse;
    GBRForestD* forestEEresolution;

    GBRForestD* forestEBresponsehighpt;
    GBRForestD* forestEBresolutionhighpt;
    GBRForestD* forestEEresponsehighpt;
    GBRForestD* forestEEresolutionhighpt;

    TFile* fileEB;
    TFile* fileEE;

    TFile* fileEBhighpt;
    TFile* fileEEhighpt;
    
    if (doEB) {
      fileEB = TFile::Open(trainingEB.c_str());
      forestEBresponse = (GBRForestD*) fileEB->Get("EBCorrection");
      forestEBresolution =  (GBRForestD*) fileEB->Get("EBUncertainty");
      if (doEBhighpt) {
	fileEBhighpt = TFile::Open(trainingEBhighpt.c_str());
	forestEBresponsehighpt = (GBRForestD*) fileEBhighpt->Get("EBCorrection");
	forestEBresolutionhighpt =  (GBRForestD*) fileEBhighpt->Get("EBUncertainty");
      } else {
	forestEBresponsehighpt = forestEBresponse;
	forestEBresolutionhighpt = forestEBresolution;
      }
      if (forestEBresponse && forestEBresolution) {
	cout << "Got EB forests" << endl;
      } else
	return 1;
    }

    if (doEE) {
      fileEE = TFile::Open(trainingEE.c_str());
      forestEEresponse =  (GBRForestD*) fileEE->Get("EECorrection");
      forestEEresolution =  (GBRForestD*) fileEE->Get("EEUncertainty");
      if (doEEhighpt) {
	fileEEhighpt = TFile::Open(trainingEEhighpt.c_str());
	forestEEresponsehighpt = (GBRForestD*) fileEEhighpt->Get("EECorrection");
	forestEEresolutionhighpt =  (GBRForestD*) fileEEhighpt->Get("EEUncertainty");
      } else {
	forestEEresponsehighpt = forestEEresponse;
	forestEEresolutionhighpt = forestEEresolution;
      }
      if (forestEEresponse && forestEEresolution) {
	cout << "Got EE forests" << endl;
      } else
	return 1;
    }

    bool isElectron = contains(parameterFile, "electron");
    bool isPhoton = contains(parameterFile, "photon");
    if (isElectron && isPhoton) {
      cout << "Sorry, I cannot decide if this parameter file is for electrons of photons" << endl;
      return 1;
    }
    
    ParReader m_reader; m_reader.read(parameterFile);
    TFile* testingFile = TFile::Open(testingFileName.c_str());
    TTree* testingTree = isElectron ? (TTree*) testingFile->Get("een_analyzer/ElectronTree") : (TTree*) testingFile->Get("een_analyzer/PhotonTree");

    TTreeFormula genE("genEnergy", "genEnergy", testingTree);
    TTreeFormula generatorPt("genPt", "genPt", testingTree);
    TTreeFormula rawE("scRawEnergy+scPreshowerEnergy", "scRawEnergy+scPreshowerEnergy", testingTree);
    
    char_separator<char> sep(":");
    tokenizer<char_separator<char>> tokensEB(m_reader.m_regParams[0].variablesEB, sep);
    tokenizer<char_separator<char>> tokensEE(m_reader.m_regParams[0].variablesEE, sep);
    std::vector<TTreeFormula*> inputformsEB;
    std::vector<TTreeFormula*> inputformsEE;
    cout << "EB variables" << endl;
    for (const auto& it : tokensEB) {      
      inputformsEB.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree));
      cout << "  " << it.c_str() <<endl;
    }
    cout << "EE variables" << endl;
    for (const auto& it : tokensEE) {
      inputformsEE.push_back(new TTreeFormula(it.c_str(),it.c_str(),testingTree));
      cout << "  " << it.c_str() <<endl;
    }


    string EBcondition = contains(m_reader.m_regParams[0].cutEB, "scIsEB") ? m_reader.m_regParams[0].cutEB : "scIsEB";
    string EEcondition = contains(m_reader.m_regParams[0].cutEE, "scIsEB") ? m_reader.m_regParams[0].cutEE : "!scIsEB";
    cout << "I will use for EB the definition:  "  << EBcondition.c_str() << endl;
    cout << "I will use for EE the definition:  "  << EEcondition.c_str() << endl;
    TTreeFormula formIsEB(EBcondition.c_str(), EBcondition.c_str(), testingTree);
    TTreeFormula formIsEE(EEcondition.c_str(), EEcondition.c_str(), testingTree);
    
    int nvarsEB = distance(tokensEB.begin(), tokensEB.end());
    int nvarsEE = distance(tokensEE.begin(), tokensEE.end());

    Float_t response = 0.;
    Float_t resolution = 0.;
    Float_t *valsEB = new Float_t[nvarsEB];
    Float_t *valsEE = new Float_t[nvarsEE];

    //initialize new friend tree
    TFile* outputFile = TFile::Open(outputFileName.c_str(), "RECREATE");
    outputFile->mkdir("een_analyzer");
    outputFile->cd("een_analyzer");

    TTree *friendtree = new TTree("correction", "correction");
    friendtree->Branch("response", &response, "response/F");
    friendtree->Branch("resolution", &resolution, "resolution/F");


    // ------------------------
    // All the TRK vars, so merging is not necessary

    int scIsEB;
    float genEnergy;
    float scRawEnergy;
    float scPreshowerEnergy;
    float trkMomentumRelError;
    float trkMomentum;
    float eleEcalDriven;
    float eleEcalDrivenSeed;
    float full5x5_r9;
    float eOverP;
    float fbrem;
    float gsfchi2;
    float gsfndof;
    float trkEta;
    float trkPhi;

    float genPt;
    float genEta;
    int   NtupID;
    int   eventNumber;

    int run;
    int luminosityBlock;

    float pt;
    float scEta;
    float corrEnergy74X;
    float corrEnergy74XError;

    float gsfnhits;

    testingTree->SetBranchAddress( "scIsEB", &scIsEB );
    testingTree->SetBranchAddress( "genEnergy", &genEnergy );
    testingTree->SetBranchAddress( "scRawEnergy", &scRawEnergy );
    testingTree->SetBranchAddress( "scPreshowerEnergy", &scPreshowerEnergy );
    testingTree->SetBranchAddress( "trkMomentumRelError", &trkMomentumRelError );
    testingTree->SetBranchAddress( "trkMomentum", &trkMomentum );
    testingTree->SetBranchAddress( "eleEcalDriven", &eleEcalDriven );
    testingTree->SetBranchAddress( "eleEcalDrivenSeed", &eleEcalDrivenSeed );
    testingTree->SetBranchAddress( "eOverPuncorr", &eOverP);
    testingTree->SetBranchAddress( "full5x5_r9", &full5x5_r9 );
    testingTree->SetBranchAddress( "fbrem", &fbrem );
    testingTree->SetBranchAddress( "gsfchi2", &gsfchi2 );
    testingTree->SetBranchAddress( "gsfndof", &gsfndof );
    testingTree->SetBranchAddress( "trkEta", &trkEta );
    testingTree->SetBranchAddress( "trkPhi", &trkPhi );

    testingTree->SetBranchAddress( "genPt", &genPt );
    testingTree->SetBranchAddress( "genEta", &genEta );
    testingTree->SetBranchAddress( "NtupID", &NtupID );
    testingTree->SetBranchAddress( "eventNumber", &eventNumber );

    testingTree->SetBranchAddress( "run", &run );
    testingTree->SetBranchAddress( "luminosityBlock", &luminosityBlock );

    testingTree->SetBranchAddress( "pt", &pt );
    testingTree->SetBranchAddress( "scEta", &scEta );
    testingTree->SetBranchAddress( "corrEnergy74X", &corrEnergy74X );
    testingTree->SetBranchAddress( "corrEnergy74XError", &corrEnergy74XError );

    testingTree->SetBranchAddress( "gsfnhits", &gsfnhits );


    if (saveTRKvarse) {
        friendtree->Branch("scIsEB", &scIsEB, "scIsEB/I" );
        friendtree->Branch("genEnergy", &genEnergy, "genEnergy/F" );
        friendtree->Branch("scRawEnergy", &scRawEnergy, "scRawEnergy/F" );
        friendtree->Branch("scPreshowerEnergy", &scPreshowerEnergy, "scPreshowerEnergy/F" );
        friendtree->Branch("trkMomentumRelError", &trkMomentumRelError, "trkMomentumRelError/F" );
        friendtree->Branch("trkMomentum", &trkMomentum, "trkMomentum/F" );
        friendtree->Branch("eleEcalDriven", &eleEcalDriven, "eleEcalDriven/F" );
        friendtree->Branch("eleEcalDrivenSeed", &eleEcalDrivenSeed, "eleEcalDrivenSeed/F" );
        friendtree->Branch("eOverP", &eOverP, "eOverP/F" );
        friendtree->Branch("full5x5_r9", &full5x5_r9, "full5x5_r9/F" );
        friendtree->Branch("fbrem", &fbrem, "fbrem/F" );
        friendtree->Branch("gsfchi2", &gsfchi2, "gsfchi2/F" );
        friendtree->Branch("gsfndof", &gsfndof, "gsfndof/F" );
        friendtree->Branch("trkEta", &trkEta, "trkEta/F" );
        friendtree->Branch("trkPhi", &trkPhi, "trkPhi/F" );

        friendtree->Branch("genPt", &genPt, "genPt/F" );
        friendtree->Branch("genEta", &genEta, "genEta/F" );
        friendtree->Branch("NtupID", &NtupID, "NtupID/I" );
        friendtree->Branch("eventNumber", &eventNumber, "eventNumber/I" );

        friendtree->Branch("run", &run, "run/I" );
        friendtree->Branch("luminosityBlock", &luminosityBlock, "luminosityBlock/I" );

        friendtree->Branch("pt", &pt, "pt/F" );
        friendtree->Branch("scEta", &scEta, "scEta/F" );
        friendtree->Branch("corrEnergy74X", &corrEnergy74X, "corrEnergy74X/F" );
        friendtree->Branch("corrEnergy74XError", &corrEnergy74XError, "corrEnergy74XError/F" );

        friendtree->Branch("gsfnhits", &gsfnhits, "gsfnhits/F" );
    }
    // ------------------------



    // #########################################
    // This part adds on a weight variable which can be cut on

    bool usePtWeightCut = true;

    TRandom randomNumber;

    TF1 *ptWeightFunction = new TF1( "weightFunction", "TMath::Max( TMath::Min( 1.0 ,TMath::Exp(-(x-50)/50) ), 0.01 )", 0., 6500. );

    int ptWeightCut;
    if(usePtWeightCut){
        friendtree->Branch("ptWeightCut", &ptWeightCut, "ptWeightCut/I" );
        }

    // #########################################




    for (Long64_t iev=0; iev<testingTree->GetEntries(); ++iev) {
    //    for (Long64_t iev=6000000; iev<testingTree->GetEntries(); ++iev) {

      response = 0.;
      resolution = 0.;
      if (iev%100000==0) printf("%i\n",int(iev));
      testingTree->LoadTree(iev);
      bool isEB = formIsEB.EvalInstance();
      bool isEE = formIsEE.EvalInstance();

      // NtupID->GetBranch()->GetEntry(iev);
      // NtupIDVal = NtupID->GetValue();
      // if (NtupIDVal < 5000) continue;

      if (isEB) {
	for (int i=0; i<nvarsEB; ++i) {
	  valsEB[i] = inputformsEB[i]->EvalInstance();
	  if (vm.count("zero") && inputformsEB[i]->GetExpFormula().EqualTo(supressVariable.c_str())) {
	    valsEB[i] = 0.0;
	    //	    cout << "Zeroing " << inputformsEB[i]->GetExpFormula().Data() << endl;
	  }
	  if (debug) cout << i << " " << inputformsEB[i]->GetExpFormula().Data() << " " << valsEB[i] << endl;
	}
	if (doEB) {
	  if (doEBhighpt && generatorPt.EvalInstance() > limhighpt) {
	    response = forestEBresponsehighpt->GetResponse(valsEB);
	    resolution = forestEBresolutionhighpt->GetResponse(valsEB);
	  } else {
	    response = forestEBresponse->GetResponse(valsEB);
	    resolution = forestEBresolution->GetResponse(valsEB);
	  }
	} else {
	  response = 0.;
	  resolution = 0.;
	}
      } else if(isEE) {
	for (int i=0; i<nvarsEE; ++i) {
	  valsEE[i] = inputformsEE[i]->EvalInstance();
	  if (vm.count("zero") && inputformsEE[i]->GetExpFormula().EqualTo(supressVariable.c_str())) {
	    valsEE[i] = 0.0;
	    //	    cout << "Zeroing " << inputformsEE[i]->GetExpFormula().Data() << endl;
	  }
	  if (debug) cout << i << " " << inputformsEE[i]->GetExpFormula().Data() << " " << valsEE[i] << endl;
	}
	if (doEE) {
	  if (doEEhighpt && generatorPt.EvalInstance() > limhighpt) {
	    response = forestEEresponsehighpt->GetResponse(valsEE);
	    resolution = forestEEresolutionhighpt->GetResponse(valsEE);
	  } else {
	    response = forestEEresponse->GetResponse(valsEE);
	    resolution = forestEEresolution->GetResponse(valsEE);
	  }
	} else {
	  response = 0.;
	  resolution = 0.;
	}
      }

      response = responseOffset + responseScale*sin(response);
      resolution = resolutionOffset + resolutionScale*sin(resolution);
      if (debug) cout << "response " << response << endl << "resolution " << resolution << endl;
      if (debug) cout << "true response " << genE.EvalInstance()/rawE.EvalInstance() << endl;
      if (testing) response = genE.EvalInstance()/rawE.EvalInstance();
		   
      testingTree->GetEntry(iev);
      eOverP *= response;

      if (usePtWeightCut) ptWeightCut = ( ptWeightFunction->Eval(genPt) > randomNumber.Rndm() ) ? 1 : 0 ;


      friendtree->Fill();
      
    }


    // Free all the memory!
    for (std::vector<TTreeFormula*>::const_iterator it = inputformsEB.begin(); it != inputformsEB.end(); ++it) {
      delete *it;
    }
    for (std::vector<TTreeFormula*>::const_iterator it = inputformsEE.begin(); it != inputformsEE.end(); ++it) {
      delete *it;
    }

    delete[] valsEB;
    delete[] valsEE;

    // Writes output
    outputFile->cd("een_analyzer");
    friendtree->Write();
    outputFile->Close();

    
  }
  
}
