/*************************************************
  This will add a branch to the specified TTree
  in the specified files.  Useful for adding
  event weights, cross-sections, etc...

  Michael B. Anderson
  Feb 20, 2009
*************************************************/

#include <vector>
#include <cmath>
#include <iostream>

// #include "Math/VectorUtil.h"
// #include "Math/GenVector/LorentzVector.h"
#include "TMath.h"
#include "TFile.h"
#include "TString.h"
#include "TBranch.h"
#include "TTree.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TF1.h"

using namespace std;
using namespace ROOT;

// typedef ROOT::Math::LorentzVector< ROOT::Math::PxPyPzE4D<float> > LorentzVector;

int main() {
  
  //************************************************************
  //                      Variables                           //
  vector<TString> fileName;

  TString USER = (TString)getenv("USER");
  if ( USER == "tklijnsm" ) {
    fileName.push_back("/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing.root");
    fileName.push_back("/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_testing_sample.root");
    fileName.push_back("/afs/cern.ch/work/t/tklijnsm/public/CMSSW_8_0_4/src/NTuples/Ntup_Jul22_fullpt_training.root");
    }
  else {
    fileName.push_back("/data/userdata/rclsa/ElectronTrees/Jul22/Ntup_Jul22_fullpt_testing_sample.root");
    fileName.push_back("/data/userdata/rclsa/ElectronTrees/Jul22/Ntup_Jul22_fullpt_testing.root");
    fileName.push_back("/data/userdata/rclsa/ElectronTrees/Jul22/Ntup_Jul22_fullpt_training.root");
    }

  float trkMomentum, trkEta, scRawEnergy, scPreshowerEnergy;
  int scIsEB;

  float ECALweight, TRKweight;  
  TString dirName = "een_analyzer";
  TString treeName = "ElectronTree";

  //                  END of Variables                        //
  //************************************************************


  // Get resolution file

  
  // Loop over all the Files
  for (int i=0; i < fileName.size(); i++) {

    cout << "Opening " << fileName[i].Data() << endl;

    TFile* currentFile = new TFile(fileName[i],"update");
    TDirectoryFile* directory = (TDirectoryFile*) currentFile->Get(dirName);
    TTree *tree = (TTree*) directory->Get(treeName);

    tree->SetBranchStatus("trkMomentum", 1);
    tree->SetBranchAddress("trkMomentum", &trkMomentum);
    tree->SetBranchStatus("trkEta", 1);
    tree->SetBranchAddress("trkEta", &trkEta);
    tree->SetBranchStatus("scRawEnergy", 1);
    tree->SetBranchAddress("scRawEnergy", &scRawEnergy);
    tree->SetBranchStatus("scPreshowerEnergy", 1);
    tree->SetBranchAddress("scPreshowerEnergy", &scPreshowerEnergy);
    tree->SetBranchStatus("scIsEB", 1);
    tree->SetBranchAddress("scIsEB", &scIsEB);

    TBranch* br1 = tree->Branch("ECALweight", &ECALweight, "ECALweight/F");
    TBranch* br2 = tree->Branch("TRKweight", &TRKweight, "TRKweight/F");
    			   
    // Loop over all the entries, and add the new branch
    Int_t numEntries = (Int_t)tree->GetEntries();
    for (Int_t j=0; j<numEntries; j++) {

      float ECALenergy = scPreshowerEnergy + scRawEnergy;
      tree->GetEntry(j);

      if (trkMomentum < 350. && trkMomentum > 4.) {
	if (scIsEB) {
	  ECALweight = 1./( 0.05*ECALenergy+0.35 );
	  TRKweight = 1./( 0.0012*trkMomentum*trkMomentum*TMath::Sqrt(trkMomentum/TMath::CosH(trkEta)) +
			   0.00018*trkMomentum*trkMomentum*trkMomentum/TMath::CosH(trkEta) -
			   0.00000018*trkMomentum*trkMomentum*trkMomentum*trkMomentum/(TMath::CosH(trkEta)*TMath::CosH(trkEta)) );	
	} else {
	  ECALweight = 1./( 0.03*ECALenergy*ECALenergy/(1+0.07*ECALenergy) + 0.05*ECALenergy*ECALenergy*TMath::Exp(-0.04*ECALenergy) );
	  TRKweight = 1./( 0.02*trkMomentum*trkMomentum*TMath::Sqrt(trkMomentum/TMath::CosH(trkEta)) -
			   0.0004*trkMomentum*trkMomentum*trkMomentum/TMath::CosH(trkEta) -
			   0.00000016*trkMomentum*trkMomentum*trkMomentum*trkMomentum/(TMath::CosH(trkEta)*TMath::CosH(trkEta)) );
	}
      } else if (trkMomentum > 350.) {
	ECALweight = 1.;
	TRKweight = 0.;
      } else {
	ECALweight = 0.;
	TRKweight = 1.;
      }

      if (TRKweight < 0.)  cout << "TRKweight less than 0 " << trkMomentum << " " << trkEta << endl;
      if (ECALweight < 0.) cout << "ECALweight less than 0 " << ECALenergy << endl;
      
      br1->Fill();
      br2->Fill();
      

    }

    directory->cd();
    tree->Write("", TObject::kOverwrite); // save new version only
    currentFile->Close();
    cout << "...closed file." << endl;
  }
}
