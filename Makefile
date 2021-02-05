CXX			:= clang++
CXXFLAGS	:= -std=c++11 -Iinclude/ -lsdl2 -lsdl2_image
DBGFLAGS	:= -g -D_DEBUG
SRC			:= source/raycaster.cc

.PHONY: raycaster

raycaster:
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(SRC) -o rayc