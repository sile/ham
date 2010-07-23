CXX=g++
CXX_FLAGS=-O2 -Wall

all: bin/hamtr bin/hamc

bin/hamtr: hamtr.cc tokenizer/ngram.hh
	${CXX} ${CXX_FLAGS} -o ${@} ${<}

bin/hamc: hamc.cc trie/builder.hh trie/char_stream.hh trie/node_allocator.hh
	${CXX} ${CXX_FLAGS} -o ${@} ${<}

#gbf: gbf.cc trie/searcher.hh
#	${CXX} ${CXX_FLAGS} -o ${@} ${@}.cc -lm

clean:
	rm -f bin/*