#ifndef HAM_TRIE_SEARCHER_HH
#define HAM_TRIE_SEARCHER_HH

#include <cstdio>
#include "char_stream.hh"

// TODO: use mmap
namespace HAM {
  namespace Trie {
    class Searcher {
    public:
      Searcher(const char* filepath) {
	FILE* f = fopen(filepath, "rb");
    
	unsigned node_size;
	fread(&node_size, sizeof(unsigned), 1, f);
	base = new unsigned [node_size];
	chck = new unsigned char [node_size];
    
	fread(base, sizeof(unsigned), node_size, f);
	fread(chck, sizeof(unsigned char), node_size, f);
	fclose(f);
      }
      ~Searcher() {
	delete [] base;
	delete [] chck;
      }

      // TODO
      operator bool() const { return true; }

      template<class Callback>
      void each_common_prefix(const char* key, Callback& fn) const {
	unsigned node_index=0;
	CharStream in(key);
	for(unsigned offset=0;; offset++) {
	  unsigned terminal_index = base[node_index] + '\0';
	  if(chck[terminal_index] == '\0') {
	    fn(key, offset, terminal_index, base[terminal_index]/10000000.0d);
	    if(in.peek()=='\0')
	      break;
	  }

	  unsigned next_index = base[node_index] + in.read();
	  if(chck[next_index] == in.prev())
	    node_index = next_index;
	  else
	    break;
	}    
      }

    private:
      unsigned *base;
      unsigned char *chck;
    };
  }
}
#endif
