CXXFLAGS += -std=c++11
LDLIBS += -lchdl

exe: netlist_gen netlist_map

netlist_map: netlist_map.cpp
netlist_gen: netlist_gen.cpp

clean:
	rm -f netlist_gen netlist_map
