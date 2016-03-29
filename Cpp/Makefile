#
# Makefile for the huff project.
#

CXX=clang++
CXXFLAGS=-std=c++11 -c -O -emit-llvm -I../../Spica/Cpp
LLVMLINK=llvm-link
LLVMLINKFLAGS=
# It shouldn't be necessary to use Boost here since my application does not require it.
# It shouldn't be necessary to use curses here since my application does not require it.
LINKFLAGS=-L/usr/lib/i386-linux-gnu -L$(BOOST_HOME)/stage/lib -lboost_regex -lncurses -lpthread
HSOURCES=CodeTree.cpp \
	huff.cpp
PSOURCES=CodeTree.cpp \
	puff.cpp
HOBJECTS=$(HSOURCES:.cpp=.bc)
POBJECTS=$(PSOURCES:.cpp=.bc)
LIBSPICA=../../Spica/Cpp/libSpicaCpp.bc

%.bc:	%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

all:	huff puff

# HUFF

huff:	huff.o
	$(CXX) $< $(LINKFLAGS) -o $@

huff.o:	huff.bc2
	llc -filetype=obj huff.bc2 -o huff.o

huff.bc2:	huff.bc1
	opt -o huff.bc2 -globaldce huff.bc1

huff.bc1:	$(HOBJECTS) $(LIBSPICA)
	$(LLVMLINK) $(LLVMLINKFLAGS) -o $@ $(HOBJECTS) $(LIBSPICA)

# PUFF

puff:	puff.o
	$(CXX) $< $(LINKFLAGS) -o $@

puff.o:	puff.bc2
	llc -filetype=obj puff.bc2 -o puff.o

puff.bc2:	puff.bc1
	opt -o puff.bc2 -globaldce puff.bc1

puff.bc1:	$(POBJECTS) $(LIBSPICA)
	$(LLVMLINK) $(LLVMLINKFLAGS) -o $@ $(POBJECTS) $(LIBSPICA)

# File Dependencies
###################

# Module dependencies -- Produced with 'depend' on Mon Jul 15 11:46:55 2013


CodeTree.bc:	CodeTree.cpp CodeTree.hpp 

huff.bc:	huff.cpp ../../Spica/Cpp/BitFile.hpp CodeTree.hpp 

puff.bc:	puff.cpp ../../Spica/Cpp/BitFile.hpp CodeTree.hpp 


# Additional Rules
##################
clean:
	rm -f *.bc *.bc1 *.bc2 *.o huff *.s *.ll *~
