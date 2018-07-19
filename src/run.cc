#include <iostream>

#include "symreg.hpp"

int main() {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x*x*x; },
    100,
    0,
    200
  );

  auto leaf_picker = symreg::MCTS::simulator::recursive_heuristic_child_picker(symreg::MCTS::UCB1);

  symreg::MCTS::MCTS mcts{5, 500, ds, symreg::MCTS::UCB1, symreg::MCTS::NRMSD, leaf_picker};
  mcts.iterate();

  auto res = mcts.build_result();

  std::cout << res->to_string() << std::endl;
  std::cout << symreg::MCTS::NRMSD(ds, res) << std::endl;
  
  return 0;
}
