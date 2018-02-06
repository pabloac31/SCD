.SUFFIXES:
.PHONY: clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors -I.
hmonsrcs:= HoareMonitor.hpp HoareMonitor.cpp 

x1: fumadores
	./$<

x2: barbero
	./$<


fumadores: fumadores.cpp
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp

barbero: barbero.cpp
	$(compilador) $(opcionesc)  -o $@ $< HoareMonitor.cpp


clean:
	rm -f fumadores barbero
