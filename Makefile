	CC=gcc
 	MODFLAGS := -Wall

# %1 is the first parameter input
all:
# emd8216.cpp :source file of DLL
# lib8216.so :output file
# must have # and a blank for comments
# must use tab to the first blank character for command
	echo
	gcc -Wall -c -fPIC emd8216.cpp

	g++ myServer.cpp emd8216.cpp -o myServer

	gcc -shared -o lib8216.so \
			emd8216.o

	cp ./lib8216.so /usr/lib/
 
clean:
	-rm -f *.o *.so 

