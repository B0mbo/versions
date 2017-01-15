#
# filestatus makefile
#

GCC=g++
CLINK=-std=c++11 -lpthread
PRJNAME=file_status

all:
	$(GCC) -o ./$(PRJNAME) *.cpp $(CLINK)
