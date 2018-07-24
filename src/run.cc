#include <iostream>

#include "symreg.hpp"

bool replace(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = str.find(from);
  if(start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

int main() {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x*x+x*x*x*x; },
    100,
    0,
    200
  );

  auto leaf_picker = symreg::MCTS::simulator::recursive_heuristic_child_picker(symreg::MCTS::UCB1);

  symreg::MCTS::MCTS mcts{16, 5000, ds, symreg::MCTS::UCB1, symreg::MCTS::NRMSD, leaf_picker};

  auto num_runs = 1;
  for (int i = 0; i < num_runs; i++) {
    mcts.iterate();
    auto top_n = mcts.get_top_n_asts();
    for (auto& ast : top_n) {
      std::string ast_str = ast->to_string();
      replaceAll(ast_str, "_x0", "x");
      std::cout << ast_str << std::endl;
      std::cout << (1 - symreg::MCTS::NRMSD(ds, ast)) << std::endl;
    }
    mcts.reset();
  }

  return 0;
}
