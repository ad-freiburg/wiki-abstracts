CXX = g++ -O3 -Wall -std=c++11
MAIN_BINARIES = $(basename $(wildcard src/*Main.cpp))
HEADER = $(wildcard src/*.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Main.cpp %Test.cpp, $(wildcard src/*.cpp))))
CPPLINT_PATH = ./cpplint.py
CPPLINT_FILTERS = -runtime/references,-build/header_guard,-build/include,-build/c++11

.PRECIOUS: %.o

all: compile

compile: $(MAIN_BINARIES) $(TEST_BINARIES)

clean:
	rm -f src/*.o
	rm -f $(MAIN_BINARIES)
	rm -f $(TEST_BINARIES)

%Main: %Main.o $(OBJECTS)
	$(CXX) -o $@ $^

%.o: %.cpp $(HEADER)
	$(CXX) -c $< -o $@
