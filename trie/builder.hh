#ifndef HAM_TRIE_BUILDER_HH
#define HAM_TRIE_BUILDER_HH

#include "node_allocator.hh"
#include "char_stream.hh"
#include <cstdio>
#include <cstring>
#include <string>

namespace HAM {
  namespace Trie {
    class Builder {
    public:
      struct Key {
	Key(const char* text, double probability)
	  : buf(text), cs(buf.c_str()), probability(probability*10000000) {}
	bool operator<(const Key& k) const { return strcmp(rest(), k. rest()) < 0; }
	
	unsigned char read() { return cs.read(); }     
	unsigned char prev() const { return cs.prev(); }
	unsigned char peek() const { return cs.peek(); } 
	const char*   rest() const { return cs.rest(); }
	void reset() { cs.reset(); }
	
	std::string buf;
	CharStream cs;
	unsigned probability;
      };
      typedef std::vector<Key> KeyList;
      
      struct ProbabilityCalculator {
	ProbabilityCalculator(unsigned total_ham_count, unsigned total_spam_count) 
	  : total_ham_count(total_ham_count), total_spam_count(total_spam_count) {}
	
	double bayesian_spam_probability (unsigned ham_count, unsigned spam_count) const {
	  double assumed_probability = 0.5;
	  double weight = 1.0;
	  double basic_probability = spam_probability(ham_count, spam_count);
	  double data_points = ham_count+spam_count;
	  return (weight*assumed_probability + data_points*basic_probability) / (weight+data_points);
	} 

	double spam_probability(unsigned ham_count, unsigned spam_count) const {
	  double spam_freq = (double)spam_count/(double)total_spam_count;
	  double ham_freq = (double)ham_count/(double)total_ham_count;
	  return spam_freq/(spam_freq+ham_freq);
	}

	const unsigned total_ham_count;
	const unsigned total_spam_count;
      };
      
    public:
      Builder(KeyList& keys)
	: keys(keys), node_size(count_node(keys)*1.5), alloc(node_size) {
	base = new unsigned[node_size];
	chck = new unsigned char[node_size];
	memset(chck, 0xFF, sizeof(unsigned char)*node_size);
      }
      
      ~Builder() {
	delete [] base;
	delete [] chck;
      }
      
      Builder& build() {
	build_impl(0, keys.size(), 0);
	return *this;
      }
      
      void save(const char* filepath) {
	FILE* f = fopen(filepath, "wb");
	
	if(node_size > 0xFF)
	  while(chck[node_size-0xFF]==0xFF)
	    node_size--;
	
	fwrite(&node_size, sizeof(unsigned), 1, f);
	fwrite(base, sizeof(int), node_size, f);
	fwrite(chck, sizeof(unsigned char), node_size, f);
	fclose(f);
      }
      
    private:
      void build_impl(std::size_t beg, std::size_t end, int root_node) {
	if(end-beg == 1) {
	  for(; keys[beg].prev() != '\0'; keys[beg].read())
	    root_node = set_node(root_node, alloc.allocate(keys[beg].peek()), keys[beg].peek());
	  base[root_node] = keys[beg].probability;
	  return;
	}
	
	std::vector<unsigned char> children;
	std::vector<std::size_t>   ranges;
	do {
	  children.push_back(keys[beg].peek());
	  ranges.push_back(beg);
	  beg = end_of_same_node(keys, beg, end);
	} while (beg != end);
	ranges.push_back(end);

	int base_node = alloc.allocate(children);

	for(std::size_t i=0; i < children.size(); i++)
	  build_impl(ranges[i], ranges[i+1], set_node(root_node, base_node, children[i]));
      }

      int set_node(int node, int base_node, unsigned char child) {
	int next   = base_node + child;
	base[node] = base_node;
	chck[next] = child;
	return next;
      }

      unsigned end_of_same_node(KeyList& keys, std::size_t beg, std::size_t end) {
	unsigned char ch = keys[beg].read();
	std::size_t cur  = beg+1;
	for(; cur < end && ch == keys[cur].peek(); cur++)
	  keys[cur].read();
	return cur;
      }

      unsigned count_node(KeyList& keys) {
	unsigned count = count_node_impl(keys,0,keys.size());
	for(std::size_t i = 0; i < keys.size(); i++)
	  keys[i].reset();
	return count;
      }

      unsigned count_node_impl(KeyList& keys, std::size_t beg, std::size_t end) {
	if(end-beg == 1) {
	  unsigned count=0;
	  for(; keys[beg].prev() != '\0'; keys[beg].read())
	    count++;
	  return count;
	}
	
	std::vector<unsigned char> children;
	std::vector<std::size_t>   ranges;
	do {
	  children.push_back(keys[beg].peek());
	  ranges.push_back(beg);
	  beg = end_of_same_node(keys, beg, end);
	} while (beg != end);
	ranges.push_back(end);

	unsigned count=children.size();
	for(std::size_t i=0; i < children.size(); i++)
	  count += count_node_impl(keys, ranges[i], ranges[i+1]);
	return count;
      }

    private:
      KeyList keys;
      int node_size;
      unsigned* base;
      unsigned char* chck;
      NodeAllocator alloc;
    };
  }
}

#endif
