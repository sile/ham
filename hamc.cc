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
  ARG_ERROR:
    std::cerr << "Usage: hamc [--lower-frequency-limit=2] <model-index>" << std::endl;
    return 1;
  }
  
  // parse arguments
  unsigned lower_frequency_limit=2;
  int arg_i=1;
  if(strncmp(argv[arg_i], OPT1, strlen(OPT1))==0)
    lower_frequency_limit = atoi(argv[arg_i++]+strlen(OPT1));

  if(arg_i == argc)
    goto ARG_ERROR;  // too few arguments

  const char* index_filepath=argv[arg_i];
  
  // parse model file
  HAM::Trie::Builder::KeyList keys;
  std::string line;

  unsigned total_ham_count=0;  
  unsigned total_spam_count=0;
  
  std::cerr << "= Read model definition:" << std::endl;

  // first line
  if(std::getline(std::cin, line)) {
    total_ham_count  = parse_hex(line.c_str());
    total_spam_count = parse_hex(line.c_str()+9);
  }
  std::cerr << "  == HAM count : " << total_ham_count << std::endl 
	    << "  == SPAM count: " << total_spam_count << std::endl;

  // rest lines
  Builder::ProbabilityCalculator pc(total_ham_count, total_spam_count);
  while(std::getline(std::cin, line)) {
    const char* s = line.c_str();
    unsigned ham_count  = parse_hex(s);
    unsigned spam_count = parse_hex(s+9);
    
    bool all     = total_ham_count==ham_count && total_spam_count==spam_count;
    bool too_few = ham_count+spam_count < lower_frequency_limit;
    if(!(all || too_few))
      keys.push_back(Builder::Key(s+18, pc.bayesian_spam_probability(ham_count, spam_count)));
  }
  std::sort(keys.begin(), keys.end());

  // omit redundant features
  if(keys.empty()==false) {
    std::reverse(keys.begin(), keys.end());
    std::size_t tail = keys.size();
    for(std::size_t i=keys.size()-1; i > 0; i--)
      // TODO: comment
      if(keys[i].probability == keys[i-1].probability && 
	 strncmp(keys[i].rest(), keys[i-1].rest(), strlen(keys[i].rest()))==0)
	std::swap(keys[--tail], keys[i]);
    keys.erase(keys.begin()+tail, keys.end());
    std::sort(keys.begin(), keys.end());
  }
  
  // convert to trie format and save
  HAM::Trie::Builder bld(keys);
  std::cerr << "= Build trie index: " << std::endl;
  bld.build();
  
  std::cerr << "= Save: " << index_filepath << std::endl;
  bld.save(index_filepath);
  
  std::cerr << "DONE" << std::endl;
  return 0;
}
