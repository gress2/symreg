#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "cpptoml.hpp"
#include "symreg.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error: must pass a .toml config file path" << std::endl;
    return 1;
  }

  std::set<std::string> used;

  symreg::util::config cfg(cpptoml::parse_file(argv[1]));
  symreg::dataset ds = symreg::generate_dataset(cfg);
  symreg::MCTS::MCTS<symreg::DNN> mcts(ds, nullptr, cfg);
  mcts.iterate();

  auto exs = mcts.get_training_examples();

  for (auto& ex : exs) {
    std::cout << ex.state << std::endl;
    for (auto& p : ex.pi) {
      std::cout << p << " "; 
    } 
    std::cout << std::endl;
    std::cout << ex.reward << std::endl;
  }

  std::cout << mcts.get_result()->to_string();

  return 0;
}
