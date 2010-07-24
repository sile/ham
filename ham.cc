#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "feature_extractor.hh"
#include "scorer.hh"

#define OPT1 "--min-spam-score="

int main(int argc, char** argv) {
  if(argc != 2 && argc != 3) {
  ARG_ERROR:
    std::cerr << "Usage: ham [--min-spam-score=0.5] <model-index>" << std::endl;
    return 2;
  }

  // parse argument
  double min_spam_score=0.5;
  int arg_i=1;
  if(strncmp(argv[arg_i], OPT1, strlen(OPT1))==0)
    min_spam_score = atof(argv[arg_i++]+strlen(OPT1));

  if(arg_i == argc)
    goto ARG_ERROR;  // too few arguments
  
  const char* model_index_path = argv[arg_i];

  // load model
  HAM::FeatureExtractor fe(model_index_path);
  if(!fe) {
    std::cerr << "Can't open file: " << model_index_path << std::endl;
    return 2;
  }

  // read text
  HAM::Scorer doc(fe);
  
  std::string line;
  while(std::getline(std::cin, line))
    doc.add_text(line.c_str());

  // calculate score and print result
  double score = doc.calc_score();
  bool is_ham = score < min_spam_score;
  printf("%05f\t%s\n", score, is_ham?"HAM":"SPAM");

  return is_ham ? 0 : 1;
}
