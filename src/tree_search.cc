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

  std::cout << "Searching for expressions fitting data generated from f(x) = ";
  std::cout << cfg.get<std::string>("dataset.function") << std::endl << std::endl;

  auto top_n = mcts.get_top_n_asts();
  int i = top_n.size();
  std::size_t num_explored = mcts.get_num_explored();
  std::cout << "After exploring " << num_explored << " ASTs, here are the best ";
  std::cout << i << " expressions I found:" << std::endl; 

  for (auto& ast : top_n) {
    std::cout << "[" << i << "] expr: " << ast->to_string();
    std::cout << " MSE: " << loss.loss(ds, ast) << std::endl; 
    i--;
  }

  return 0;
}

