#include <iostream>

#include "cpptoml.hpp"
#include "symreg.hpp"

int main(int argc, char* argv[]) {
  
  if (argc < 2) {
    std::cerr << "Error: Must pass a .toml config file path" << std::endl;
    return 1;
  }

  symreg::util::config cfg(cpptoml::parse_file(argv[1]));
  symreg::dataset ds = symreg::generate_dataset(cfg);
  symreg::MCTS::MCTS<symreg::DNN> mcts(ds, nullptr, cfg);
  mcts.iterate();

  symreg::loss_fn::MSE loss;

  auto top_n = mcts.get_top_n_asts();
  for (auto& ast : top_n) {
    std::cout << ast->to_string() << std::endl;
    std::cout << (loss.loss(ds, ast)) << std::endl;
  }

  return 0;
}

