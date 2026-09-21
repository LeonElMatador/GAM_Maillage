CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g
TARGET := tp1.exe

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all check clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

check:
	$(CXX) $(CXXFLAGS) -fsyntax-only $(SOURCES)

clean:
	$(RM) $(OBJECTS) $(TARGET)