#pragma once

#include <random>

#include "cpptoml.hpp"

namespace symreg
{
namespace util
{

/**
 * @brief a wrapper around cpptoml::table 
 * with some added convenience methods for more
 * concise config access
 */
class config {
  private:
    std::shared_ptr<cpptoml::table> tbl_;
  public:
    config(std::shared_ptr<cpptoml::table>);
    template <class T>
    T get(std::string);
    template <class T>
    std::vector<T> get_vector(std::string); 
};

/**
 * @brief config constructor
 * @param tbl a shared pointer to a cpptoml::table which
 * is produced from parsing a .toml file on disk
 */
config::config(std::shared_ptr<cpptoml::table> tbl) 
  : tbl_(tbl)
{}

/**
 * @brief a getter for retrieving values from a .toml config by key
 * @param key a table prefixed key which will be used to fetch a value with.
 * for example: "table1.prop2"
 * @return returns the value of type T corresponding to the key if the key exists
 * in the .toml. Otherwise, throws an exception. 
 */
template <class T>
T config::get(std::string key) {
  auto option = tbl_->get_qualified_as<T>(key);
  if (!option) {
    std::cerr << "Error: key [" << key << "] not in config" << std::endl;
    throw "MissingTomlKeyException";
  }
  return option.value_or(T{});
}

/**
 * @brief a getter for retrieving arrays from a .toml config by key
 * @param key a table prefixed key which will be used to fetch an array of
 * values with. for example "table1.prop2"
 * @return a std::vector<T> built from the array of values associated with
 * the key. Throws an exception if the key didn't exist in the .toml file.
 */
template <class T>
std::vector<T> config::get_vector(std::string key) {
  auto option = tbl_->get_qualified_array_of<T>(key);
  if (!option) {
    std::cerr << "Error: key [" << key << "] not in config" << std::endl;
    throw "MissingTomlKeyException";
  }
  return option.value_or(std::vector<T>{});
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
