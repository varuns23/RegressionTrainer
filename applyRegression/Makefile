#makefile 


CC   =   g++ 
UCFLAGS = -O3 -fopenmp -Wall -gstabs+ -g 

RUCFLAGS := $(shell root-config --cflags) -I./include/ -I${CMSSW_BASE}/src/ -I${CMSSW_RELEASE_BASE}/src/ $(foreach a,$(subst :, ,$(CMSSW_FWLITE_INCLUDE_PATH)),-I$(a))
LIBS :=  $(foreach a,$(subst :, ,$(CMSSW_FWLITE_INCLUDE_PATH)),-L$(a)/../lib/) -lboost_system -lX11 -lpthread -lm -lboost_filesystem -lboost_program_options -lgomp 
LIBS += $(shell root-config --libs) -lTreePlayer  -lTMVA -lRooFit -lRooFitCore  -L${CMSSW_BASE}/lib/${SCRAM_ARCH} -L${CMSSW_RELEASE_BASE}/lib/${SCRAM_ARCH} -lHiggsAnalysisGBRLikelihood -lCondFormatsEgammaObjects 
#LIBS += -lboost_system
GLIBS := $(shell root-config --glibs)

SRCPP = ParReader.cpp\
	applyRegression.cpp

         

OBJCPP = $(patsubst %.cpp,obj/%.o,$(SRCPP))


all : applyRegression.exe
	#obj/libDictionary_C.so

obj/%.o : %.cpp
	@echo "> compiling $*"
	@mkdir -p obj/
	@$(CC) -c $< $(UCFLAGS) $(RUCFLAGS) -o $@




applyRegression.exe : $(OBJCPP)
	@echo "> linking"
	@echo "aclibs are $(ACLIBS)"
	@$(CC) $^ -o $@ $(ACLIBS) $(LIBS) $(GLIBS)  
#	@$(CC) $^ $(ACLIBS) $(LIBS) $(GLIBS) -o $@

clean:
	@echo "> Cleaning object files"
	@rm  -f obj/*.o
        
cleanall: clean
	@echo "> Cleaning executable"
	@rm -f main.exe

print-%  : ; @echo $* = $($*)