# Makefile for FTP project
CC = g++
CFLAGS = -Wall -pedantic -Wextra
SRC = proj.cpp proj.hh
RESULT = fclient
#LOGIN = 
#FILES = Makefile proj.cpp README manual.pdf

$(RESULT): $(SRC)
	$(CC) $(CFLAGS) -o $(RESULT) $(SRC)

clean:
	rm -f *~
	rm -f $(RESULT)

#tar: clean
#	tar -cf $(LOGIN).tar $(FILES)

#rmtar:
#	rm -f $(LOGIN).tar