#
# filestatus makefile
#

GCC=g++
CLINK=-lpthread
PRJNAME=file_status

all:
	$(GCC) -o ./$(PRJNAME) $(PRJNAME).cpp $(CLINK)
