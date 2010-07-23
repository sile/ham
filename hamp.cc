#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <tr1/unordered_set>
#include "trie/searcher.hh"

typedef std::vector<double> probabilities;
typedef std::tr1::unordered_set<int> NodeIdSet;

struct Features {
  Features()
    : score(0.0) {}
  
  void operator()(const char* key, unsigned len, int node_id, double spam_probability) {
    if(used.find(node_id) != used.end())
      return;
    used.insert(node_id);
    
    ham_acc_prob  += log(1.0 - spam_probability);
    spam_acc_prob += log(spam_probability);
  }

  double calc() const {
    double h = 1.0 - fisher(spam_acc_prob);
    double s = 1.0 - fisher(ham_acc_prob);
    return (1.0-h+s)/2.0;
  }

  double fisher(double probability_product) const {
    return inverse_chi_square(-2.0*probability_product, used.size()*2);
  }

  static const double MAX_ALLOWABLE_M=700.0;
  double inverse_chi_square(double value, unsigned degrees_of_freedom) const {
    double m;
    unsigned df;
    make_adjustments(value, degrees_of_freedom, 1.0, m, df);
    
    if(m > MAX_ALLOWABLE_M)
      make_adjustments(value, degrees_of_freedom, MAX_ALLOWABLE_M/m, m, df);
    
    double term = exp(-m);

    double sum = term;
    double limit = df/2;
    for(double i=1.0; i < limit; i++) {
      term *= m/i;
      sum  += term;
    }

    //std::cout << "term=" << term << ", " << "m=" << m << ", sum=" << sum << ", limit=" << limit << std::endl;
    return std::min(sum,1.0);
  }

  static unsigned round(double d) { return static_cast<unsigned>(d+.5); }
  
  void make_adjustments(double fChi, unsigned iDF, double fESF, double& fM, unsigned& iAdjustedDF) const {
    unsigned iHalfDF = iDF/2;
    unsigned iAdjustedHalfDF = std::max(1u, round(fESF * iHalfDF));
    double fAdjustedProp = static_cast<double>(iAdjustedHalfDF) / iHalfDF;
    double fAdjustedChi = fChi * fAdjustedProp;

    iAdjustedDF = iAdjustedHalfDF*2;
    fM = fAdjustedChi / 2.0;
  }

  NodeIdSet used;
  double ham_acc_prob;
  double spam_acc_prob;
  double score;
};

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: ham [--min-spam-score=0.5] <model-index>" << std::endl;
    return 2;
  }
  
  HAM::Trie::Searcher srch(argv[1]);
  if(!srch) {
    std::cerr << "Can't open file: " << argv[1] << std::endl;
    return 2;
  }
  
  Features fs;
  std::string line;
  while(std::getline(std::cin, line)) {
    const char* s = line.c_str();
    for(; *s != '\0'; s++)
      srch.each_common_prefix(s, fs);
  }

  double score = fs.calc();
  printf("%05f\n", score);

  return score < 0.5 ? 0 : 1;
}
