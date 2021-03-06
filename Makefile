


# Note: CXX, CXXFLAGS, LD, LDFLAGS, SOFLAGS, LIBS, GLIBS are defined here
include $(ROOTSYS)/etc/Makefile.arch

bica1: bica1.cpp
	$(CXX) $(CXXFLAGS) bica1.cpp $(LIBS) -o bica1

bica2: bica2.cpp
	$(CXX) $(CXXFLAGS) bica2.cpp $(LIBS) -o bica2

bica3: bica3.cpp
	$(CXX) $(CXXFLAGS) bica3.cpp $(LIBS) -o bica3

bica6: bica6.cpp
	$(CXX) $(CXXFLAGS) bica6.cpp $(LIBS) -o bica6

bica7: bica7.cpp
	$(CXX) $(CXXFLAGS) bica7.cpp $(LIBS) -o bica7

clean:
	rm -f bica1 bica2 bica3 bica6
