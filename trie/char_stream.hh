#ifndef HAM_TRIE_CHAR_STREAM_HH
#define HAM_TRIE_CHAR_STREAM_HH

namespace HAM {
  namespace Trie {
    class CharStream {
    public:
      CharStream(const char* str) : beg(str), cur(str) {}
      unsigned char read() { return *cur++; }     
      unsigned char prev() const { return cur[-1]; }
      unsigned char peek() const { return *cur; } 
      const char*   rest() const { return cur; }
      void reset() { cur=beg; }

    private:
      const char* beg;
      const char* cur;
    };
  }
}

#endif
