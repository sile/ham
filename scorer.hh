#ifndef HAM_SCORER_HH
#define HAM_SCORER_HH

#include "feature_extractor.hh"
#include <tr1/unordered_set>
#include <cmath>
#include <algorithm>

namespace HAM {
  class Scorer {
    typedef std::tr1::unordered_set<unsigned> FeatureIdSet;
    friend class Trie::Searcher;
    
  public:
    Scorer(const FeatureExtractor& fe)
      : fe(fe), ham_acc_prob(0.0), spam_acc_prob(0.0) {}
    
    void add_text(const char* text) {
      fe.reset();
      for(; *text != '\0'; text++)
	fe.each_feature(text, *this);
    }

    double calc_score() const {
      double h = 1.0 - fisher(spam_acc_prob);
      double s = 1.0 - fisher(ham_acc_prob);
      return (1.0-h+s)/2.0;
    }

  private:
    void operator()(const char* text, unsigned len, unsigned feature_id, double spam_probability) {
      if(used.find(feature_id) != used.end())
	return;
      used.insert(feature_id);
      
      ham_acc_prob  += log(1.0 - spam_probability);
      spam_acc_prob += log(spam_probability);
    }

    double fisher(double acc_prob) const {
      return inverse_chi_square(-2.0*acc_prob, 2*used.size());
    }

    static const double MAX_ALLOWABLE_M=700.0;
    double inverse_chi_square(double chi, unsigned degrees_of_freedom) const {
      double m;
      unsigned df;
      make_adjustments(chi, degrees_of_freedom, 1.0, m, df);
      
      if(m > MAX_ALLOWABLE_M)
	make_adjustments(chi, degrees_of_freedom, MAX_ALLOWABLE_M/m, m, df);
      
      double term  = exp(-m);
      double sum   = term;
      double limit = df/2;

      for(double i=1.0; i < limit; i++) {
	term *= m/i;
	sum  += term;
      }

      return std::min(sum, 1.0);
    }

    static unsigned round(double d) { return static_cast<unsigned>(d+0.5); }
    static void make_adjustments(double chi, unsigned df, double esf, double& m, unsigned& adjustedDF) {
      unsigned halfDF = df/2;
      unsigned adjustedHalfDF = std::max(1u, round(esf * halfDF));
      double adjustedProp = static_cast<double>(adjustedHalfDF) / halfDF;
      double adjustedChi = chi * adjustedProp;
      
      adjustedDF = adjustedHalfDF*2;
      m = adjustedChi / 2.0;
    }
    
  private:
    const FeatureExtractor& fe;
    FeatureIdSet used;
    double ham_acc_prob;
    double spam_acc_prob;
  };
}

#endif
