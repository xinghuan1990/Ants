ifdef LONG
INTT = -DLONG
endif

ifdef EDGELONG
INTE = -DEDGELONG
endif

#compilers

PCC = g++
PCFLAGS = -fcilkplus -lcilkrts -lmemkind -O2 $(INTT) $(INTE)


COMMON= ligra.h graph.h vertex.h utils.h IO.h parallel.h gettime.h blockRadixSort.h transpose.h parseCommandLine.h vertexSubset.h

ALL= PageRank BFS BC Components Radii Triangle BellmanFord KCore MIS

all: $(ALL)

% : %.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< 
	
$(COMMON):
	ln -s ../graph/$@ .

.PHONY : clean

clean :
	rm -f *.o $(ALL)

cleansrc :
	rm -f *.o $(ALL)
	rm $(COMMON)
