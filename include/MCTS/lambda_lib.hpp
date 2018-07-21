#ifndef SYMREG_MCTS_LAMBDA_LIB_HPP_
#define SYMREG_MCTS_LAMBDA_LIB_HPP_

namespace symreg
{
namespace MCTS
{

static auto UCB1 = [](double child_val, int child_n, int parent_n) { 
  return child_val + sqrt(2 * log(parent_n) / child_n); 
};

static auto MSE = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
     auto& xi = ds.x[i];
     sum += std::pow(ast->eval(xi) - ds.y[i], 2); 
  }
  double mse = sum / ds.x.size();
  return mse;
}; 

static auto NRMSD = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
  double RMSD = sqrt(MSE(ds, ast));
  double min = *std::min_element(ds.y.begin(), ds.y.end());
  double max = *std::max_element(ds.y.begin(), ds.y.end());
  auto nrmsd = RMSD / (max - min);
  return nrmsd;
};

template <typename LossFn>
auto bind_loss_fn(LossFn& loss, dataset& ds) {
  return [&] (std::shared_ptr<brick::AST::AST> ast) {
    return loss(ds, ast);
  }; 
}

}
}

#endif
