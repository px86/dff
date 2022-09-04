CXX = g++
CXXFLAGS = -Wall -Werror -Wpedantic -Weffc++ -std=c++17 -O3
LIBS = -lpthread

dff: src/main.cpp sha256.o dff.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: src/%.cpp src/include/%.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf *.o dff
