#include <iostream>

#include "symreg.hpp"

int main() {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x * x * x; },
    100,
    0,
    200
  );

  auto leaf_picker = symreg::MCTS::simulator::recursive_heuristic_child_picker(symreg::MCTS::UCB1);

  symreg::MCTS::MCTS mcts{5, 1500, ds, symreg::MCTS::UCB1, symreg::MCTS::NRMSD, leaf_picker};

  auto num_runs = 1;

  for (int i = 0; i < num_runs; i++) {
    mcts.iterate();
    auto res = mcts.get_result();
    std::cout << res->to_string() << std::endl;
    mcts.reset();
  }

  return 0;
}
