#ifndef SYMREG_MCTS_MT_HPP_
#define SYMREG_MCTS_MT_HPP_

#include <random>

namespace symreg
{
namespace MCTS
{
static std::random_device rd;
static std::mt19937 mt(rd());
}
}

#endif
