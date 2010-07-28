#include <iostream>
#include <fstream>
#include <string>
#include <tr1/unordered_map>
#include <cstdlib>
#include <cstring>
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
  
  bool train_mbox_file(std::string filepath, bool is_ham) {
    std::ifstream in(filepath.c_str());
    if(!in) 
      return false;
    
    std::string line;
    while(std::getline(in,line)) {
      unsigned offset=0;
      if(line.empty())
	continue;
      if(strncmp("From ", line.c_str(), strlen("From "))==0) {
	doc_id++;
	total.inc(is_ham, doc_id);
	offset = strlen("From ");
      }

      Callback fn(tc, is_ham, doc_id);
      ngram.each_token(line.c_str()+offset, fn);
    }
    return true;
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
  
  unsigned cnt=0;
  std::cerr << "= " << (is_ham?"HAM":"SPAM") << ": " << dir << std::endl;
  for(dirent* dirp=readdir(dp); dirp; dirp=readdir(dp))
    if(dirp->d_name[0] != '.') {
      std::string path = dir+"/"+dirp->d_name;
      std::cerr << "  == " << ++cnt << "#" << path << std::endl;
      tr.train_file(path.c_str(), is_ham);
    }
  std::cerr << std::endl;
  closedir(dp);  
  return true;
}

int main(int argc, char** argv) {
  if(argc != 4 && argc != 5) {
  ARG_ERROR:
    std::cerr << "Usage: hamt [--mbox] <learn-dir> <ngram-min> <ngram-max>" << std::endl;
    return 1;
  }

  // parse arguments
  int arg_i = 1;
  bool is_mbox = false;
  
  if(strcmp(argv[arg_i],"--mbox")==0) {
    is_mbox = true;
    arg_i++;
  }
  if(argc-arg_i != 3)
    goto ARG_ERROR;  

  // init
  std::string rootdir = argv[arg_i];
  Train tr(atoi(argv[arg_i+1]), atoi(argv[arg_i+2]));

  // train
  if(is_mbox) {
    // ## mbox formatted text
    // ham
    if(tr.train_mbox_file(rootdir+"/ham.mbx",true)==false) {
      std::cerr << "Can't open file: " << rootdir+"/ham.mbx" << std::endl;
      return 1;
    }
    
    // spam
    if(tr.train_mbox_file(rootdir+"/spam.mbx",false)==false) {
      std::cerr << "Can't open file: " << rootdir+"/spam.mbx" << std::endl;
      return 1;
    }
  } else {
    // ## plane text
    // ham
    if(train_files(tr, rootdir+"/ham",true)==false) {
      std::cerr << "Can't open directory: " << rootdir+"/ham" << std::endl;
      return 1;
    }
    
    // spam
    if(train_files(tr, rootdir+"/spam",false)==false) {
      std::cerr << "Can't open directory: " << rootdir+"/spam" << std::endl;
      return 1;
    }
  } 
  
  // output 
  tr.output_model();
  
  std::cerr << "DONE" << std::endl;
  return 0;
}
