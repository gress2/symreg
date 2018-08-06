#ifndef SYMREG_MCTS_DNN_HPP_
#define SYMREG_MCTS_DNN_HPP_

#include <iostream>
#include <numeric>
#include "util.hpp"
#include <dlib/dnn.h>
#include "training_example.hpp"

namespace symreg
{

class DNN {
  private:
    int policy_dim_;
    std::mt19937 mt_;
    std::vector<double> random_prob_dist(int);
    double random_prob();
  public:
    DNN(int);
    template <class State>
    std::pair<double, std::vector<double>> inference(State&&); 
    void train(training_examples&);
};

DNN::DNN(int policy_dim)
 : policy_dim_(policy_dim),
   mt_(std::random_device()())
{}

std::vector<double> DNN::random_prob_dist(int size) {
  std::vector<double> dist(size);
  for (auto& e : dist) {
    e = util::get_random_int(0, 1000, mt_);
  }
  auto sum = std::accumulate(dist.begin(), dist.end(), 0);
  for (auto& e : dist) {
    e /= sum;
  }
  return dist;
}

double DNN::random_prob() {
  return util::get_random_int(0, 1000, mt_) / 1000; 
}

template <class State>
std::pair<double, std::vector<double>> DNN::inference(State&& state) {
  // TODO: real inference
  return std::make_pair(random_prob(), random_prob_dist(policy_dim_));
}

void DNN::train(training_examples& examples) {
  // TODO: actually train
}


}

#endif
