#ifndef HAM_TRIE_SEARCHER_HH
#define HAM_TRIE_SEARCHER_HH

#include "char_stream.hh"
#include "../util/mmap_t.hh"

namespace HAM {
  namespace Trie {
    class Searcher {
    public:
      Searcher(const char* filepath) : mm(filepath), prev_tail(NULL) {
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

      void reset() const {
	prev_tail=NULL;
      }

      template<class Callback>
      void longest_common_prefix(const char* key, Callback& fn) const {
	unsigned last_matched_index=0;
	unsigned last_matched_offset=0;

	unsigned node_index=0;
	CharStream in(key);
	for(unsigned offset=0;; offset++) {
	  unsigned terminal_index = base[node_index] + '\0';
	  if(chck[terminal_index] == '\0') {
	    last_matched_index  = terminal_index;
	    last_matched_offset = offset;
	    if(in.peek()=='\0')
	      break;
	  }

	  unsigned next_index = base[node_index] + in.read();
	  if(chck[next_index] == in.prev())
	    node_index = next_index;
	  else
	    break;
	}  	
	if(last_matched_offset > 0 && (!prev_tail || key+last_matched_offset > prev_tail)) {
	  prev_tail = key+last_matched_offset;
	  fn(key, last_matched_offset, last_matched_index, 
	     base[last_matched_index]/10000000.0);
	}
      }
    private:
      Util::mmap_t mm;
      unsigned node_size;
      const unsigned *base;
      const unsigned char *chck;
      mutable const char* prev_tail;
    };
  }
}
#endif
