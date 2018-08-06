#pragma once

#include <random>

namespace symreg
{
namespace MCTS
{
static std::random_device rd;
static std::mt19937 mt(rd());
}
}
