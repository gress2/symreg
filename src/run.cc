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
    [](double x) { return x*x*x+x*x+x; },
    100,
    -100,
    100
  );

  using namespace symreg::MCTS;

  simulator::simulator sim(
      score::UCB1,
      loss::bind_loss_fn(loss::MAPE, ds),
      simulator::recursive_heuristic_child_picker{score::UCB1},
      10
  );

  MCTS mcts(1000, ds, sim);
  mcts.iterate();
  auto top_n = mcts.get_top_n_asts();
  for (auto& ast : top_n) {
    std::string ast_str = ast->to_string();
    replaceAll(ast_str, "_x0", "x");
    std::cout << ast_str << std::endl;
    std::cout << (1 - symreg::MCTS::loss::MAPE(ds, ast)) << std::endl;
  }

  return 0;
}
