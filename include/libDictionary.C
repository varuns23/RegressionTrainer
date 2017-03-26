// File loader.C
#include <map>
#include <string>
#include "../src/GBRTree.cpp"
#include "../src/GBRForest.cpp"

#ifdef __MAKECINT__
//#pragma link C++ class std::pair<std::string,std::string>+;
//#pragma link C++ class std::map<std::string,float>+;
#pragma link C++ class GBRTree+;
#pragma link C++ class GBRForest+;
#pragma link C++ class std::vector<GBRTree>+;
#endif

void libDictionary(){}
