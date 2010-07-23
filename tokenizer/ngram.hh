#ifndef HAM_TOKENIZER_NGRAM_HH
#define HAM_TOKENIZER_NGRAM_HH

#include <algorithm>
#include <vector>
#include <cstring>

namespace HAM {
  namespace Tokenizer {
    class Ngram {
    public:
      Ngram(unsigned min, unsigned max) 
	: min(std::max(min, 1u)), max(std::min(max, 32u)) {}
      
      template<class Callback>
      void each_token(const char* text, Callback& fn) const {
	std::vector<unsigned> char_start_pos;
	const unsigned len = strlen(text);
	
	for(unsigned i=0; i < len; i++) {
	  if(!(text[i]&0x80))
	    char_start_pos.push_back(i); // ascii
	  else if (text[i]&0x40)
	    char_start_pos.push_back(i); // start of a UTF-8 character byte sequence
	}
	char_start_pos.push_back(len);
	
	for(unsigned i=0; i < char_start_pos.size(); i++) 
	  for(unsigned n=min; n <= max; n++) 
	    if(i+n < char_start_pos.size())
	      fn(text+char_start_pos[i], text+char_start_pos[i+n]);
      }
      
    private:
      const unsigned min;
      const unsigned max;
    };
  }
}

#endif
