all: deal

deal: deal.c
	gcc -O3 -o $@ $^
