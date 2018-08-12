#include <fstream>
#include <iostream>

#include "cpptoml.hpp"
#include "symreg.hpp"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "requires two arguments." << std::endl;
    return 1;
  }
    
  std::string loss_fn_str = argv[1];
  std::string ast_dir = argv[2];

  std::vector<std::string> fns = {
    "3000",
    "4*x-35",
    "3*x^3-x^2+100",
    "x^5",
    "50*x^2+100*x",
    "1",
    "-400*x-35*x^2",
    "0.5^x"
  };

  for (int i = 0; i < fns.size(); i++) {
    std::string target_fn = fns[i];
    auto target_ast = brick::AST::parse(target_fn);
    auto lambda = [&] (double n) {
      return target_ast->eval(n);
    };
    symreg::dataset ds = symreg::generate_dataset(lambda, 100, -100, 100);

    std::ifstream infile(ast_dir + "/asts_" + std::to_string(i));

    std::vector<std::string> asts;
    std::string ast;
    while (infile >> ast) {
      asts.push_back(ast); 
    }

    using priq_elem_type = std::pair<int, double>;

    static auto priq_cmp = [](const priq_elem_type& lhs, const priq_elem_type& rhs) {
      return lhs.second > rhs.second;  
    };

    static auto priq_elem_sign = [](const priq_elem_type& elem) {
      return elem.first;
    };

    symreg::fixed_priority_queue<priq_elem_type, decltype(priq_cmp), 
      decltype(priq_elem_sign)> priq(priq_cmp, priq_elem_sign, 1000);


    auto loss_fn = symreg::loss_fn::get(loss_fn_str);

    for (int j = 0; j < asts.size(); j++) {
      auto cur = brick::AST::parse(asts[j]);
      std::shared_ptr<brick::AST::AST> shared = std::move(cur);
      priq.push(std::make_pair(j, loss_fn->loss(ds, shared)));
    }

    std::vector<int> rankings(asts.size());

    auto dump = priq.dump();

    for (int k = 0; k < dump.size(); k++) {
      rankings[dump[k].first] = k; 
    }

    std::ofstream ofile(ast_dir + "/" + loss_fn_str + "_rankings_" + std::to_string(i));
    
    for (auto& e : rankings) {
      ofile << (e + 1) << std::endl;
    }
  }

  return 0;
}
