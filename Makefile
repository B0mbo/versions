#
# filestatus makefile
#

GCC=g++
CLINK=-O0 -g -std=c++11 -lpthread -lz
PRJNAME=file_status

all:
	$(GCC) -o ./$(PRJNAME) *.cpp $(CLINK)

clean:
	rm -f $(PRJNAME)
