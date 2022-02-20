CXX = g++
CXXFLAGS = -Wall -Werror -Wpedantic -std=c++17 -O3

dff: src/main.cpp sha256.o dff.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: src/%.cpp src/include/%.hpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf *.o dff
