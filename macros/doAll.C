{
  gROOT->ProcessLine(".L resolution_fitter.C+g");
  resolution_fitter("/data/userdata/rclsa/ElectronTrees/Jul17/Ntup_Jul15_fullpt_testing.root");
  gROOT->ProcessLine(".x draw.C");
}
  
    
