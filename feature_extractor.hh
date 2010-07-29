#ifndef HAM_FEATURE_EXTRACTOR_HH
#define HAM_FEATURE_EXTRACTOR_HH

#include "trie/searcher.hh"

namespace HAM {
  class FeatureExtractor {
  public:
    FeatureExtractor(const char* model_index_path, bool lmo=false) 
      : srch(model_index_path), longest_match_only(lmo) {}

    operator bool() const { return srch; }

    template <class Callback>
    void each_feature(const char* text, Callback& fn) const {
      if(longest_match_only)
	srch.longest_common_prefix(text, fn);
      else
	srch.each_common_prefix(text, fn);
    }

    // TODO: description
    void reset() const { 
      srch.reset();
    }

    private:
    Trie::Searcher srch;
    const bool longest_match_only;
  };
}

#endif
