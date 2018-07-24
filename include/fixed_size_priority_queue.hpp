#include <array>
#include <iostream>
#include <queue>

namespace symreg
{

template<typename T, typename Cmp, int N>
class fixed_priority_queue {
  private:
    std::priority_queue<T, std::vector<T>, Cmp> priq_;
    Cmp cmp_;
  public:
    fixed_priority_queue(Cmp); 
    void push(T); 
    std::array<T, N> dump();
};

template <class T, class Cmp, int N>
fixed_priority_queue<T, Cmp, N>::fixed_priority_queue(Cmp cmp)
  : priq_(cmp), cmp_(cmp)
{}

template <class T, class Cmp, int N>
void fixed_priority_queue<T, Cmp, N>::push(T t) {
  if (priq_.size() == N) {
    if (cmp_(t, priq_.top())) {
      priq_.pop();
      priq_.push(t);
    }
  } else {
    priq_.push(t);
  }
}

template <class T, class Cmp, int N>
std::array<T, N> fixed_priority_queue<T, Cmp, N>::dump() {
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
