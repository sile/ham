#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <tr1/unordered_set>
#include "trie/searcher.hh"

#include <sys/types.h>
#include <dirent.h>
#include <fstream>

typedef std::tr1::unordered_set<int> NodeIdSet;

struct Features {
  Features()
    : ham_acc_prob(0.0), spam_acc_prob(0.0) {}
  
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
};

unsigned test_files(const HAM::Trie::Searcher& srch, const std::string& dir, unsigned& file_count) {
  DIR *dp = opendir(dir.c_str());
  file_count = 0;
  if(!dp)
    return 0;
  
  unsigned h=0;
  for(dirent* dirp=readdir(dp); dirp; dirp=readdir(dp))
    if(dirp->d_name[0] != '.') {
      file_count++;
      std::string path = dir+"/"+dirp->d_name;
      
      std::ifstream in(path.c_str());
      Features fs;
      std::string line;
      while(std::getline(in, line)) {
	const char* s = line.c_str();
	for(; *s != '\0'; s++)
	  srch.each_common_prefix(s, fs);
      }
      
      double score = fs.calc();
      if(score < 0.5)
	h++;
    }
  closedir(dp);  
  return h;
}


int main(int argc, char** argv) {
  if(argc != 3) {
    std::cerr << "Usage: ham [--min-spam-score=0.5] <model-index>" << std::endl;
    return 2;
  }
  
  HAM::Trie::Searcher srch(argv[1]);
  if(!srch) {
    std::cerr << "Can't open file: " << argv[1] << std::endl;
    return 2;
  }
  
  std::string root=argv[2];
  unsigned ham_count=0;
  unsigned h1 = test_files(srch, root+"/ham", ham_count);

  unsigned spam_count=0;
  unsigned h2 = test_files(srch, root+"/spam", spam_count);
  std::cout << h1 << ',' << ham_count << ',' << h2 << ',' << spam_count << std::endl;
  std::cout << "precision: " << (double)(h1)/(h1+h2) << std::endl;
  std::cout << "recall:    " << (double)(h1)/ham_count << std::endl;
  return 0;
}

// shell?
