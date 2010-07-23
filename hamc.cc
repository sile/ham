#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include "trie/builder.hh"

using HAM::Trie::Builder;

unsigned parse_hex(const char* s) {
  return static_cast<unsigned>(strtol(s, NULL, 16));
}

#define OPT1 "--lower-frequency-limit="

int main(int argc, char** argv) {
  if(argc != 2 && argc != 3) {
    std::cerr << "Usage: hamc [--lower-frequency-limit=2] <model-index>" << std::endl;
    return 1;
  }

  unsigned lower_frequency_limit=2;
  unsigned arg_i=1;
  if(strncmp(argv[arg_i], OPT1, strlen(OPT1))==0)
    lower_frequency_limit = atoi(argv[arg_i++]+strlen(OPT1));
  const char* index_filepath=argv[arg_i];

  HAM::Trie::Builder::KeyList keys;
  std::string line;

  unsigned total_ham_count=0;  
  unsigned total_spam_count=0;
  
  if(std::getline(std::cin, line)) {
    total_ham_count  = parse_hex(line.c_str());
    total_spam_count = parse_hex(line.c_str()+9);
  }

  while(std::getline(std::cin, line)) {
    const char* s = line.c_str();
    unsigned ham_count  = parse_hex(s);
    unsigned spam_count = parse_hex(s+9);
    if(ham_count + spam_count >= lower_frequency_limit)
       keys.push_back(Builder::Key(s+18, ham_count, spam_count, total_ham_count, total_spam_count));
  }
  std::sort(keys.begin(), keys.end());
  
  HAM::Trie::Builder bld(keys);
  bld.build().save(index_filepath);
  
  return 0;
}
