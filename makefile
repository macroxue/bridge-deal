all: deal encode

OPTS=-std=c++0x -Wall

deal: deal.c
	gcc -O3 -o $@ $^
encode: encode.cc
	g++ -O3 $(OPTS) -o $@ $^
