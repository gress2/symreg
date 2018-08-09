#pragma once

#include <array>
#include <iostream>
#include <queue>
#include <set>

namespace symreg
{

/**
 * @brief a generic priority queue which holds up to N elements
 */
template<typename T, typename Cmp, typename Sign>
class fixed_priority_queue {
  private:
    std::priority_queue<T, std::vector<T>, Cmp> priq_;
    Cmp cmp_;
    Sign sign_;
    std::set<decltype(sign_(priq_.top()))> signs_; 
    std::size_t N_;
  public:
    fixed_priority_queue(Cmp, Sign, int); 
    void push(T); 
    std::vector<T> dump();
};

/**
 * @brief fixed size priority queue constructor
 * @param cmp a lambda (returning a bool) for comparing two elements of type T
 * @param sign a lambda which returns a signature of element type T. 
 * the priority queue ensures that elements with duplicate signatures are not
 * stored in the queue simultaneously 
 */
template <class T, class Cmp, class Sign>
fixed_priority_queue<T, Cmp, Sign>::fixed_priority_queue(Cmp cmp, Sign sign, int N)
  : priq_(cmp), cmp_(cmp), sign_(sign), N_(N)
{}

/**
 * @brief method of adding (or attempting to add) items to the queue
 * @param t element to be added to queue
 */
template <class T, class Cmp, class Sign>
void fixed_priority_queue<T, Cmp, Sign>::push(T t) {
  auto sign = sign_(t);
  if (signs_.count(sign)) {
    return;
  }

  if (priq_.size() == N_) {
    if (cmp_(t, priq_.top())) {
      signs_.erase(signs_.find(sign_(priq_.top()))); 
      priq_.pop();
      priq_.push(t);
      signs_.insert(sign);
    }
  } else {
    priq_.push(t);
    signs_.insert(sign);
  }
}

/**
 * @brief converts the queue into an array, emptying the queue 
 * in the process
 * @return the priority queue converted into an array
 */
template <class T, class Cmp, class Sign>
std::vector<T> fixed_priority_queue<T, Cmp, Sign>::dump() {
  std::vector<T> vec;
  int i = 0;
  while(!priq_.empty()) {
    vec.push_back(priq_.top());
    priq_.pop();
    i++;
  }
  return vec;
}

}
