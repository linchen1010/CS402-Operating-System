#
# This is the Makefile that can be used to create the "listtest" executable
# To create "listtest" executable, do:
#	make for All
#	make listtest for part(A)
#	make warmup1 for part(B)

warmup1All: warmup1 listtest

warmup1: warmup1.o my402list.o
	gcc -o warmup1 -g warmup1.o my402list.o

listtest: listtest.o my402list.o
	gcc -o listtest -g listtest.o my402list.o

listtest.o: listtest.c my402list.h
	gcc -g -c -Wall listtest.c

warmup1.o: warmup1.c my402list.h
	gcc -g -c -Wall warmup1.c

my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

clean:
	rm -f *.o listtest warmup1
