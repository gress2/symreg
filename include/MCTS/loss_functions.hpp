#pragma once

namespace symreg
{
namespace MCTS
{
namespace loss
{

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

static auto MAPE = [](dataset& ds, std::shared_ptr<brick::AST::AST>& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    double y_hat = ast->eval(ds.x[i]);
    sum += (y_hat + ds.y[i] == 0) ? 0 : std::abs((ds.y[i] - y_hat) / (ds.y[i] + y_hat)); 
  }
  sum /= ds.x.size();
  return sum;
};

template <typename LossFn>
auto bind_loss_fn(LossFn& loss, dataset& ds) {
  return [&] (std::shared_ptr<brick::AST::AST> ast) {
    return loss(ds, ast);
  }; 
}


}
}
}

