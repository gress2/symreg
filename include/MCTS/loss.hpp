#pragma once

namespace symreg
{
namespace MCTS
{
namespace loss_fn
{

using ast_ptr = std::shared_ptr<brick::AST::AST>;

// LOSS_FN INTERFACE

template <class T>
class loss_fn {
  public:
    double loss(dataset& ds, ast_ptr& ast);
};

template <class T>
double loss_fn<T>::loss(dataset& ds, ast_ptr& ast) {
  return static_cast<T*>(this)->calc_loss(ds, ast);
}

// MSE

class MSE : public loss_fn<MSE> {
  public:
    double calc_loss(dataset&, ast_ptr&); 
};

double MSE::calc_loss(dataset& ds, ast_ptr& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
     auto& xi = ds.x[i];
     sum += std::pow(ast->eval(xi) - ds.y[i], 2); 
  }
  double mse = sum / ds.x.size();
  return mse;
}

// NRMSD

class NRMSD : public loss_fn<NRMSD> {
  private:
    MSE mse_;
  public:
    double calc_loss(dataset&, ast_ptr&);
};

double NRMSD::calc_loss(dataset& ds, ast_ptr& ast) {
  double RMSD = sqrt(mse_.calc_loss(ds, ast));
  double min = *std::min_element(ds.y.begin(), ds.y.end());
  double max = *std::max_element(ds.y.begin(), ds.y.end());
  return RMSD / (max - min);
}

// MAPE

class MAPE : public loss_fn<MAPE> {
  public:
    double calc_loss(dataset&, ast_ptr&); 
};

double MAPE::calc_loss(dataset& ds, ast_ptr& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    double y_hat = ast->eval(ds.x[i]);
    sum += (y_hat + ds.y[i] == 0) ? 0 : std::abs((ds.y[i] - y_hat) / (ds.y[i] + y_hat)); 
  }
  return sum / dx.x.size();
}

} // loss
} // MCTS
} // symreg

