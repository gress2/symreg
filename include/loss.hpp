#pragma once

#include <limits>

namespace symreg
{
namespace loss_fn
{

using ast_ptr = std::shared_ptr<brick::AST::AST>;

/**
 * @brief a loss function interface. determines the
 * goodness of fit of an AST to a dataset
 */
class loss_fn {
  public:
    void limit_loss(double&, const double&);
    virtual double loss(dataset& ds, ast_ptr& ast) = 0;
};

void loss_fn::limit_loss(double& loss, const double& max_loss) {
  if (std::isnan(loss) || std::isinf(loss)) {
    loss = max_loss;
  }
}

/**
 * @brief mean squared error
 */ 
class MSE : public loss_fn {
  private:
    constexpr static double max_loss_ = 1e10;
  public:
    double loss(std::vector<double>&, std::vector<double>&);
    double loss(dataset&, ast_ptr&); 
};

double MSE::loss(std::vector<double>& a, std::vector<double>& b) {
  double sum = 0;
  for (std::size_t i = 0; i < a.size(); i++) {
    sum += std::pow(a[i] - b[i], 2);
  }
  auto res = sum / a.size();
  limit_loss(res, max_loss_);
  return res;
}

/**
 * @brief calculates the mean squared error
 * of a dataset evaluated across an AST.
 * @param ds a reference to a datset
 * @param ast a complete ast which will be used
 * to evaluate dataset.x points
 * @return the mean squared error
 */
double MSE::loss(dataset& ds, ast_ptr& ast) {
  std::vector<double>& a = ds.y;
  std::vector<double> b;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    b.push_back(ast->eval(ds.x[i]));
  } 
  return loss(a, b);
}

/**
 * @brief normalized root mean squared deviation
 */
class NRMSD : public loss_fn {
  private:
    constexpr static double max_loss_ = 1e10;
    MSE mse_;
  public:
    double loss(dataset&, ast_ptr&);
};

/**
 * @brief calculates the normalized root mean squared 
 * deviation of a dataset evaluated across an AST.
 * @param ds a reference to a datset
 * @param ast a complete ast which will be used
 * to evaluate dataset.x points
 * @return the NRMSD 
 */
double NRMSD::loss(dataset& ds, ast_ptr& ast) {
  double RMSD = sqrt(mse_.loss(ds, ast));
  double min = *std::min_element(ds.y.begin(), ds.y.end());
  double max = *std::max_element(ds.y.begin(), ds.y.end());
  double res = RMSD / (max - min);
  limit_loss(res, max_loss_);
  return res;
}

/**
 * @brief mean absolute percentage error
 */
class MAPE : public loss_fn {
  private:
    constexpr static double max_loss_ = 1;
  public:
    double loss(dataset&, ast_ptr&); 
};

/**
 * @brief calculates the mean absolute percentage 
 * error of a dataset evaluated across an AST.
 * @param ds a reference to a datset
 * @param ast a complete ast which will be used
 * to evaluate dataset.x points
 * @return the MAPE 
 */
double MAPE::loss(dataset& ds, ast_ptr& ast) {
  double sum = 0;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    double y_hat = ast->eval(ds.x[i]);
    sum += (y_hat + ds.y[i] == 0) ? 0 : std::abs((ds.y[i] - y_hat) / (ds.y[i] + y_hat)); 
  }
  double res = sum /= ds.x.size();
  limit_loss(res, max_loss_);
  return res;
}

/**
 * @brief given a string representation of a loss function,
 * returns a shared pointer to a corresponding function instance
 * @param loss_fn_str a string representation of a loss function.
 * for example, "MSE"
 * @return a shared pointer to a loss function
 */
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
} // symreg

