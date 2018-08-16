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
  std::size_t num_written = 0;

  symreg::util::config cfg(cpptoml::parse_file(argv[1]));
  std::string fn = cfg.get<std::string>("dataset.function");
  symreg::dataset ds = symreg::generate_dataset(cfg);
  std::unique_ptr<symreg::MCTS::MCTS<symreg::DNN>> mcts = 
    std::make_unique<symreg::MCTS::MCTS<symreg::DNN>>(ds, nullptr, cfg);
  
  while (num_written < 9) {
    for (int i = 0; i < 3; i++) {
      mcts->reset();
      mcts->iterate();
      auto exs = mcts->get_training_examples();
      for (auto& ex : exs) {
        std::cout << ex.f_hat << std::endl;
        for (auto& p : ex.pi) {
          std::cout << p << " "; 
        } 
        std::cout << std::endl;
        std::cout << ex.reward << std::endl;
      }
      num_written++;    
    }
    // grab a "random" ast encountered in the last search
    auto top_n = mcts->get_top_n_asts();
    for (int j = 0; j < top_n.size(); j++) {
      auto cur_ast_str = top_n[j]->to_string();
      if (!used.count(cur_ast_str)) {
        fn = cur_ast_str;
      }
    }

    cfg.set("dataset.function", fn);
    ds = symreg::generate_dataset(cfg);
    mcts = std::make_unique<symreg::MCTS::MCTS<symreg::DNN>>(ds, nullptr, cfg);
  }

  return 0;
}
