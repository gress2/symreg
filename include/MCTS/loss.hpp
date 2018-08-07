#pragma once

namespace symreg
{
namespace MCTS
{
namespace loss_fn
{

using ast_ptr = std::shared_ptr<brick::AST::AST>;

// LOSS_FN INTERFACE

class loss_fn {
  public:
    virtual double loss(dataset& ds, ast_ptr& ast) = 0;
};

// MSE

class MSE : public loss_fn {
  public:
    double loss(dataset&, ast_ptr&); 
};

double MSE::loss(dataset& ds, ast_ptr& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
     auto& xi = ds.x[i];
     sum += std::pow(ast->eval(xi) - ds.y[i], 2); 
  }
  double mse = sum / ds.x.size();
  return mse;
}

// NRMSD

class NRMSD : public loss_fn {
  private:
    MSE mse_;
  public:
    double loss(dataset&, ast_ptr&);
};

double NRMSD::loss(dataset& ds, ast_ptr& ast) {
  double RMSD = sqrt(mse_.loss(ds, ast));
  double min = *std::min_element(ds.y.begin(), ds.y.end());
  double max = *std::max_element(ds.y.begin(), ds.y.end());
  return RMSD / (max - min);
}

// MAPE

class MAPE : public loss_fn {
  public:
    double loss(dataset&, ast_ptr&); 
};

double MAPE::loss(dataset& ds, ast_ptr& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    double y_hat = ast->eval(ds.x[i]);
    sum += (y_hat + ds.y[i] == 0) ? 0 : std::abs((ds.y[i] - y_hat) / (ds.y[i] + y_hat)); 
  }
  return sum / ds.x.size();
}

std::shared_ptr<loss_fn> get(std::string loss_fn_str) {
  if (loss_fn_str == "MSE") {
    return std::make_shared<MSE>();
  } else if (loss_fn_str == "NRMSD") {
    return std::make_shared<NRMSD>();
  } else if (loss_fn_str == "MAPE") {
    return std::make_shared<MAPE>();
  } else {
    return std::make_shared<MAPE>();
  }
}

} // loss
} // MCTS
} // symreg

