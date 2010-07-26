CXX=g++
CXX_FLAGS=-O2 -Wall

all: bin bin/hamt bin/hamc bin/ham

bin:
	mkdir bin

bin/hamt: hamt.cc tokenizer/ngram.hh
	${CXX} ${CXX_FLAGS} -o ${@} ${<}

bin/hamc: hamc.cc trie/builder.hh trie/char_stream.hh trie/node_allocator.hh
	${CXX} ${CXX_FLAGS} -o ${@} ${<}

bin/ham: ham.cc trie/searcher.hh trie/char_stream.hh util/mmap_t.hh
	${CXX} ${CXX_FLAGS} -o ${@} ${<} -lm

install:
	cp bin/* /usr/local/bin/

clean:
	rm -f bin/*
