#include <iostream>
#include <fstream>
#include <string>
#include <tr1/unordered_map>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include "tokenizer/ngram.hh"

struct Counter {
  Counter() : ham_count(0), spam_count(0), prev_doc_id(0) {}
  void inc(bool is_ham, unsigned doc_id) { 
    if(prev_doc_id == doc_id)
      return;
    prev_doc_id = doc_id;
    is_ham ? ham_count++ : spam_count++; 
  }
  
  unsigned ham_count;
  unsigned spam_count;
  unsigned prev_doc_id;
};

typedef std::tr1::unordered_map<std::string, Counter> TokenCounter;

class Train {
  struct Callback {
    Callback(TokenCounter& tc, bool is_ham, unsigned doc_id) 
      : tc(tc), is_ham(is_ham), doc_id(doc_id) {}
    void operator()(const char* beg, const char* end) const {
      tc[std::string(beg,end)].inc(is_ham, doc_id);
    }
    TokenCounter& tc;
    const bool is_ham;
    const unsigned doc_id;
  };
  
public:
  Train(unsigned ngram_min, unsigned ngram_max) 
    : ngram(ngram_min, ngram_max), doc_id(0) {}
  
  void train_file(const char* filepath, bool is_ham) {
    std::ifstream in(filepath);
    if(!in) {
      std::cerr << "WARN: Can't open file: " << filepath << std::endl;
      return;
    }
    doc_id++;
    total.inc(is_ham, doc_id);
    
    std::string line;
    Callback fn(tc, is_ham, doc_id);
    while(std::getline(in,line))
      ngram.each_token(line.c_str(), fn);
  }
  
  void output_model() const {
    // first line
    printf("%08x %08x %s\n", total.ham_count, total.spam_count, "==TOTAL==");
    
    // rest lines
    TokenCounter::const_iterator it = tc.begin();
    for(; it != tc.end(); ++it) {
      const std::string& w = it->first;
      const Counter& c = it->second;
      
      printf("%08x %08x %s\n", c.ham_count, c.spam_count, w.c_str());
    }
  }

private:
  HAM::Tokenizer::Ngram ngram;
  TokenCounter tc;
  Counter total;
  unsigned doc_id;
};

bool train_files(Train& tr, const std::string& dir, bool is_ham) {
  DIR *dp = opendir(dir.c_str());
  if(!dp)
    return false;
  
  std::cerr << "= " << (is_ham?"HAM":"SPAM") << ": " << dir << std::endl;
  for(dirent* dirp=readdir(dp); dirp; dirp=readdir(dp))
    if(dirp->d_name[0] != '.') {
      std::string path = dir+"/"+dirp->d_name;
      std::cerr << "  == " << path << std::endl;
      tr.train_file(path.c_str(), is_ham);
    }
  std::cerr << std::endl;
  closedir(dp);  
  return true;
}

int main(int argc, char** argv) {
  if(argc != 4) {
    std::cerr << "Usage: hamtr <learn-dir> <ngram-min> <ngram-max>" << std::endl;
    return 1;
  }

  // init
  const char* rootdir = argv[1];
  Train tr(atoi(argv[2]), atoi(argv[3]));
  
  // train ham
  if(train_files(tr, std::string(rootdir)+"/ham",true)==false) {
    std::cerr << "Can't open directory: " << std::string(rootdir)+"/ham" << std::endl;
    return 1;
  }

  // train spam
  if(train_files(tr, std::string(rootdir)+"/spam",false)==false) {
    std::cerr << "Can't open directory: " << std::string(rootdir)+"/spam" << std::endl;
    return 1;
  }

  // output 
  tr.output_model();
    
  return 0;
}
