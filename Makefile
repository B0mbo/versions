#
# filestatus makefile
#

GCC=g++
CLINK=-std=c++11 -lpthread -lz
PRJNAME=file_status

all:
	$(GCC) -o ./$(PRJNAME) *.cpp $(CLINK)

clean:
	rm -v $(PRJNAME)
