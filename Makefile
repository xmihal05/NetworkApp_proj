# Makefile for FTP project
CC = g++
CFLAGS = -Wall -pedantic -Wextra
PROJECT = proj.cpp
RESULT = fclient
#LOGIN = xmihal05
#FILES = Makefile proj.cpp README manual.pdf

$(RESULT): $(PROJECT)
	$(CC) $(CFLAGS) $(PROJECT) -o $(RESULT)

clean:
	rm -f *~
	rm -f $(RESULT)

#tar: clean
#	tar -cf $(LOGIN).tar $(FILES)

#rmtar:
#	rm -f $(LOGIN).tar