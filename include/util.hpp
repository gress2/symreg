#pragma once

#include <random>

#include "cpptoml.hpp"

namespace symreg
{
namespace util
{

class config {
  private:
    std::shared_ptr<cpptoml::table> tbl_;
  public:
    config(std::shared_ptr<cpptoml::table>);
    template <class T>
    T get(std::string);
};

config::config(std::shared_ptr<cpptoml::table> tbl) 
  : tbl_(tbl)
{}

template <class T>
T config::get(std::string key) {
  auto option = tbl_->get_qualified_as<T>(key);
  if (!option) {
    std::cerr << "Error: key [" << key << "] not in config" << std::endl;
    exit(1);
  }
  return option.value_or(T{});
}

  /**
   * @brief simply returns a random integer in the range [lower, upper] 
   *
   * build a uniform integer distribution to be used with the MCTS class' Marsenne twister
   *
   * @param lower the lowest possible integer which may be returned
   * @param upper the highest possible integer which may be returned
   * @return a random integer on [lower, upper]
   */
  int get_random_int(int lower, int upper, std::mt19937& mt) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(lower, upper);
    return dist(mt); 
  }

}
}
