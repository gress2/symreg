#pragma once

namespace symreg
{
namespace MCTS
{
namespace score 
{

static auto UCB1 = [](double child_val, int child_n, int parent_n) { 
  return child_val + sqrt(2 * log(parent_n) / child_n); 
};

}
}
}
