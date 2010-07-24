#ifndef HAM_FEATURE_EXTRACTOR_HH
#define HAM_FEATURE_EXTRACTOR_HH

#include "trie/searcher.hh"

namespace HAM {
  class FeatureExtractor {
  public:
    FeatureExtractor(const char* model_index_path) 
      : srch(model_index_path) {}

    operator bool() const { return srch; }

    template <class Callback>
    void each_feature(const char* text, Callback& fn) const {
      srch.each_common_prefix(text, fn);
    }

    private:
    Trie::Searcher srch;
  };
}

#endif
