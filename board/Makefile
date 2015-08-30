CXXFLAGS += -std=c++11
LDLIBS = -lpcb

board: newboard.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o board newboard.cpp $(LDLIBS)

clean:
	$(RM) board
