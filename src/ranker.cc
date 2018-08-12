#include <fstream>
#include <iostream>

#include "cpptoml.hpp"
#include "symreg.hpp"

int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cerr << "requires three arguments." << std::endl;
    return 1;
  }

  std::string loss_fn_str = argv[1];

  std::string target_fn = argv[2];
  auto target_ast = brick::AST::parse(target_fn);
  auto lambda = [&] (double n) {
    return target_ast->eval(n);
  };
  symreg::dataset ds = symreg::generate_dataset(lambda, 100, -100, 100);

  std::ifstream infile(argv[3]);

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
    return elem.second;
  };

  symreg::fixed_priority_queue<priq_elem_type, decltype(priq_cmp), 
    decltype(priq_elem_sign)> priq(priq_cmp, priq_elem_sign, 1000);


  auto loss_fn = symreg::loss_fn::get(loss_fn_str);

  for (int i = 0; i < asts.size(); i++) {
    auto cur = brick::AST::parse(asts[i]);
    std::shared_ptr<brick::AST::AST> shared = std::move(cur);
    priq.push(std::make_pair(i, loss_fn->loss(ds, shared)));
  }

  auto dump = priq.dump();
  for (auto& e : dump) {
    std::cout << e.first << std::endl;
  }

  return 0;
}
