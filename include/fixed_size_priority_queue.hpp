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
template<typename T, typename Cmp, typename Sign, int N>
class fixed_priority_queue {
  private:
    std::priority_queue<T, std::vector<T>, Cmp> priq_;
    Cmp cmp_;
    Sign sign_;
    std::set<decltype(sign_(priq_.top()))> signs_; 
  public:
    fixed_priority_queue(Cmp, Sign); 
    void push(T); 
    std::array<T, N> dump();
};

/**
 * @brief fixed size priority queue constructor
 * @param cmp a lambda (returning a bool) for comparing two elements of type N
 * @param sign a lambda which returns a signature of element type N. 
 * the priority queue ensures that elements with duplicate signatures are not
 * stored in the queue simultaneously 
 */
template <class T, class Cmp, class Sign, int N>
fixed_priority_queue<T, Cmp, Sign, N>::fixed_priority_queue(Cmp cmp, Sign sign)
  : priq_(cmp), cmp_(cmp), sign_(sign)
{}

/**
 * @brief method of adding (or attempting to add) items to the queue
 * @param t element to be added to queue
 */
template <class T, class Cmp, class Sign, int N>
void fixed_priority_queue<T, Cmp, Sign, N>::push(T t) {
  auto sign = sign_(t);
  if (signs_.count(sign)) {
    return;
  }

  if (priq_.size() == N) {
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
template <class T, class Cmp, class Sign, int N>
std::array<T, N> fixed_priority_queue<T, Cmp, Sign, N>::dump() {
  std::array<T, N> ary;
  int i = 0;
  while(!priq_.empty()) {
    ary[i] = priq_.top();
    priq_.pop();
    i++;
  }
  return ary;
}

}
