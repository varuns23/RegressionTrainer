#makefile 


CC   =   g++
UCFLAGS = -Wall -gstabs+  
RUCFLAGS := $(shell root-config --cflags) -I../include/ -I${CMSSW_BASE}/src/ -I${CMSSW_RELEASE_BASE}/src/ -I/cvmfs/cms.cern.ch/slc6_amd64_gcc530/external/boost/1.63.0-mlhled2/include/
LIBS :=  -lboost_system -lboost_filesystem -lboost_program_options -lgomp $(shell root-config --libs) -lTreePlayer  -lTMVA -lRooFit -lRooFitCore -L${CMSSW_BASE}/lib/${SCRAM_ARCH} -L${CMSSW_RELEASE_BASE}/lib/${SCRAM_ARCH} -lHiggsAnalysisGBRLikelihood -lCondFormatsEgammaObjects -L/cvmfs/cms.cern.ch/slc6_amd64_gcc530/external/boost/1.63.0-mlhled2/lib/
GLIBS := $(shell root-config --glibs)


SRCPP = ParReader.cpp\
	applyRegression.cpp

         
#OBJCPP = $(SRCPP:.cpp=.o)
OBJCPP = $(patsubst %.cpp,obj/%.o,$(SRCPP))


all : applyRegression.exe
	#obj/libDictionary_C.so

obj/%.o : %.cpp
	@echo "> compiling $*"
	@mkdir -p obj/
	@$(CC) -c $< $(UCFLAGS) $(RUCFLAGS) -o $@

applyRegression.exe : $(OBJCPP)
	@echo "> linking"
	@$(CC) $^ -o $@ $(ACLIBS) $(LIBS) $(GLIBS)  

clean:
	@echo "> Cleaning object files"
	@rm  -f obj/*.o
        
cleanall: clean
	@echo "> Cleaning executable"
	@rm -f main.exe

