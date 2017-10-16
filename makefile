#compiler
OMPL_DIR = /usr/local
INC_CLASSES = ./proj_classes/
INC_PLANNERS = ./planners/
INC_VALIDITY = ./validity_checkers/
INC_RUN = ./run/

EIGEN_DIR = /home/avishai/Documents/eigen

KDL_DIR = /usr/local

GL_INCPATH = -I/usr/include/
GL_LIBPATH = -L/usr/lib/ -L/usr/X11R6/lib/
GL_LIBS = -lGLU -lGL -lXext -lXmu -lXi -lX11 -lglut

PQP_DIR= /home/avishai/Documents/PQP_v1.3/

CXX= g++
CXXFLAGS= -I${OMPL_DIR}/include -I${OMPL_DIR}/lib/x86_64-linux-gnu -I${INC_CLASSES} -I${INC_PLANNERS} -I${PQP_DIR}/include $(GL_INCPATH) -I${KDL_DIR}/include  -I$(EIGEN_DIR) 
LDFLAGS=  -L${OMPL_DIR}/lib -L${OMPL_DIR}/lib/x86_64-linux-gnu -lompl -lompl_app_base -lompl_app -lboost_filesystem -lboost_system -lboost_serialization -lboost_program_options -Wl,-rpath ${OMPL_DIR}/lib/x86_64-linux-gnu -L${PQP_DIR}/lib -L${KDL_DIR}/lib -lPQP -lm $(GL_LIBS) -lorocos-kdl
LIBS += -L/usr/lib/x86_64-linux-gnu -lboost_system

CPPPQP = ${INC_VALIDITY}collisionDetection.cpp ${INC_VALIDITY}model.cpp

CPPAPC = ${INC_CLASSES}apc_class.cpp ${INC_VALIDITY}StateValidityCheckerPCS.cpp
CPPNR = ${INC_CLASSES}kdl_class.cpp ${INC_VALIDITY}StateValidityCheckerNR.cpp
CPPRLX = ${INC_CLASSES}kdl_class.cpp ${INC_VALIDITY}StateValidityCheckerRLX.cpp
CPPRSS = ${INC_CLASSES}apc_class.cpp ${INC_VALIDITY}StateValidityCheckerRSS.cpp

CPP_P_PCS = ${INC_RUN}plan_PCS.cpp ${INC_PLANNERS}CBiRRT_PCS.cpp ${INC_PLANNERS}RRT_PCS.cpp ${INC_PLANNERS}SBL_PCS.cpp ${INC_PLANNERS}PRM_PCS.cpp 
CPP_P_NR = ${INC_RUN}plan_NR.cpp ${INC_PLANNERS}CBiRRT_NR.cpp ${INC_PLANNERS}RRT_NR.cpp ${INC_PLANNERS}SBL_NR.cpp ${INC_PLANNERS}PRM_NR.cpp 
CPP_P_RSS = ${INC_RUN}plan_RSS.cpp ${INC_PLANNERS}CBiRRT_RSS.cpp ${INC_PLANNERS}RRT_RSS.cpp ${INC_PLANNERS}SBL_RSS.cpp  
CPP_P_RLX = ${INC_RUN}plan_RLX.cpp ${INC_PLANNERS}CBiRRT_RLX.cpp ${INC_PLANNERS}RRT_RLX.cpp ${INC_PLANNERS}SBL_RLX.cpp ${INC_PLANNERS}PRM_RLX.cpp 

all:
	$(CXX) ${CPP_P_PCS} ${CPPAPC} ${CPPPQP} -o ppcs $(CXXFLAGS) $(LDFLAGS) -DPCS -std=c++11

	$(CXX) ${CPP_P_NR} ${CPPNR} ${CPPPQP} -o pnr $(CXXFLAGS) $(LDFLAGS) -DPNR -std=c++11

	$(CXX) ${CPP_P_RSS} ${CPPRSS} ${CPPPQP} -o prss $(CXXFLAGS) $(LDFLAGS) -DPCS -std=c++11

	$(CXX) ${CPP_P_RLX} ${CPPRLX} ${CPPPQP} -o prlx $(CXXFLAGS) $(LDFLAGS) -DPNR -std=c++11
