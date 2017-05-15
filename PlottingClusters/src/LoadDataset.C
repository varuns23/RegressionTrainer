#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooExponential.h"
#include "RooGaussian.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "RooConstVar.h"
#include "RooDataSet.h"
// #include "RooHybridBDTAutoPdf.h"
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
#include "TLatex.h"
// #include "HybridGBRForest.h"
#include "RooProduct.h"
#include "RooChebychev.h"
#include "RooBernstein.h"
#include "RooPolynomial.h"
#include "RooGenericPdf.h"
//#include "HZZ2L2QRooPdfs.h"
// #include "RooDoubleCBFast.h"
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
#include "RooLinearVar.h"
#include "TH2.h"
#include "TProfile.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TGraphErrors.h"


#include "../interface/RooHybridBDTAutoPdf.h"
#include "../interface/HybridGBRForest.h"
#include "../interface/RooDoubleCBFast.h"
#include "../interface/HybridGBRForestFlex.h"


using namespace RooFit;



//#######################################
// MAIN
//#######################################




//#######################################
// Load a ws
//#######################################

// This is the returned data structure for the function LoadWorkspace
struct LoadedWS {
    RooWorkspace *ws;
    TString name;

    RooGBRTargetFlex *meantgt;
    RooRealVar *tgtvar;

    RooAbsPdf  *sigpdf;
    RooAbsReal *sigmeanlim;
    RooAbsReal *sigwidthlim;
    RooAbsReal *signlim;
    RooAbsReal *sign2lim;
    };

// Function that loads
LoadedWS LoadWorkspace(
    TString ws_file,
    bool dobarrel
    ){

    TFile *ws_rootfp = TFile::Open(ws_file);

    RooWorkspace *ws;
    if (dobarrel)
        ws = (RooWorkspace*)ws_rootfp->Get("wereg_eb");
    else
        ws = (RooWorkspace*)ws_rootfp->Get("wereg_ee");


    //#######################################
    // Load relevant variables from the ws
    //#######################################

    // Read variables from workspace
    RooGBRTargetFlex *meantgt;
    if (dobarrel)
        meantgt = static_cast<RooGBRTargetFlex*>( ws->arg("sigmeantEB") );
    else
        meantgt = static_cast<RooGBRTargetFlex*>( ws->arg("sigmeantEE") );

    RooRealVar *tgtvar = ws->var("targetvar");


    RooAbsPdf  *sigpdf;
    RooAbsReal *sigmeanlim;
    RooAbsReal *sigwidthlim;
    RooAbsReal *signlim;
    RooAbsReal *sign2lim;
    if (dobarrel){
        sigpdf      = ws->pdf("sigpdfEB");
        sigmeanlim  = ws->function("sigmeanlimEB");
        sigwidthlim = ws->function("sigwidthlimEB");
        signlim     = ws->function("signlimEB");
        sign2lim    = ws->function("sign2limEB");
        }
    else {
        sigpdf      = ws->pdf("sigpdfEE");
        sigmeanlim  = ws->function("sigmeanlimEE");
        sigwidthlim = ws->function("sigwidthlimEE");
        signlim     = ws->function("signlimEE");
        sign2lim    = ws->function("sign2limEE");
        }


    //#######################################
    // Register to-be-returned variables in a specific struct
    //#######################################

    LoadedWS LWS;

    LWS.ws = ws;
    
    if (dobarrel)
        LWS.name = "wereg_eb";
    else
        LWS.name = "wereg_ee";

    LWS.meantgt = meantgt;
    LWS.tgtvar = tgtvar;

    LWS.sigpdf      = sigpdf;
    LWS.sigmeanlim  = sigmeanlim;
    LWS.sigwidthlim = sigwidthlim;
    LWS.signlim     = signlim;
    LWS.sign2lim    = sign2lim;

    return LWS;
    }




RooDataSet* LoadDataset(
    TString eventcut_string,
    Bool_t  dobarrel,
    TString tree_file,
    TString tree_directory,
    TString tree_name,
    RooArgList vars
    ) {


    TCut eventcut  = (TCut)eventcut_string;
    TCut regioncut;
    if (dobarrel)
        regioncut = "clusLayer==-1";
    else
        regioncut = "clusLayer==-2";

    RooRealVar weightvar( "weightvar", "genEnergy > 0." ,1.);
    weightvar.SetTitle( eventcut * regioncut );

    // Get the actual tree
    TFile *tree_rootfp = TFile::Open( tree_file );
    TDirectory *tree_directoryfp = (TDirectory*)tree_rootfp->FindObjectAny(tree_directory);
    TTree *tree = (TTree*)tree_directoryfp->Get(tree_name);


    RooDataSet *hdata = RooTreeConvert::CreateDataSet( "hdata", tree, vars, weightvar );
    // cout << "In total " << hdata->numEntries() << " entries in Ntuple" << endl;

    return hdata;

    }
