CXX = g++
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
INCLUDE = -Isrc/include
EXE = lightning

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(OBJ) $(INCLUDE) -o $(EXE)

clean:
	rm -f $(EXE) $(OBJ)