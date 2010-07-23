#ifndef HAM_TRIE_SEARCHER_HH
#define HAM_TRIE_SEARCHER_HH

#include "char_stream.hh"
#include "../util/mmap_t.hh"

namespace HAM {
  namespace Trie {
    class Searcher {
    public:
      Searcher(const char* filepath) : mm(filepath) {
	if(mm) {
	  const unsigned* up = reinterpret_cast<const unsigned*>(mm.ptr);
	  
	  node_size = up[0];
	  up++;

	  base = up;
	  up += node_size;
	  
	  chck = reinterpret_cast<const unsigned char*>(up);
	}
      }

      operator bool() const { return mm; }

      template<class Callback>
      void each_common_prefix(const char* key, Callback& fn) const {
	unsigned node_index=0;
	CharStream in(key);
	for(unsigned offset=0;; offset++) {
	  unsigned terminal_index = base[node_index] + '\0';
	  if(chck[terminal_index] == '\0') {
	    fn(key, offset, terminal_index, base[terminal_index]/10000000.0);
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
      Util::mmap_t mm;
      unsigned node_size;
      const unsigned *base;
      const unsigned char *chck;
    };
  }
}
#endif
