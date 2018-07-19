#ifndef SYMREG_MCTS_UTIL_HPP_
#define SYMREG_MCTS_UTIL_HPP_

namespace symreg
{
namespace MCTS
{
namespace util
{

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
}

#endif
