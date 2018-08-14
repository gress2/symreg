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
class MAE : public loss_fn {
  private:
    constexpr static double max_loss_ = 1e100;
  public:
    double loss(std::vector<double>&, std::vector<double>&);
    double loss(dataset&, ast_ptr&); 
};

double MAE::loss(std::vector<double>& a, std::vector<double>& b) {
  double sum = 0;
  for (std::size_t i = 0; i < a.size(); i++) {
    sum += std::abs(a[i] - b[i]);
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
double MAE::loss(dataset& ds, ast_ptr& ast) {
  std::vector<double>& a = ds.y;
  std::vector<double> b;
  for (std::size_t i = 0; i < ds.x.size(); i++) {
    b.push_back(ast->eval(ds.x[i]));
  } 
  return loss(a, b);
}

/**
 * @brief mean squared error
 */ 
class MSE : public loss_fn {
  private:
    constexpr static double max_loss_ = 1e100;
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
    constexpr static double max_loss_ = 1e100;
    MSE mse_;
  public:
    double loss(dataset&, ast_ptr&);
    double loss(std::vector<double>&, std::vector<double>&);
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

double NRMSD::loss(std::vector<double>& y, std::vector<double>& y_hat) {
  double RMSD = sqrt(mse_.loss(y, y_hat));
  double min = *std::min_element(y.begin(), y.end());
  double max = *std::max_element(y.begin(), y.end());
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
    double loss(std::vector<double>&, std::vector<double>&);
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

double MAPE::loss(std::vector<double>& y, std::vector<double>& y_hat) {
  double sum = 0;
  for (std::size_t i = 0; i < y.size(); i++) {
    sum += (y_hat[i] + y[i] == 0) ? 0 :
      std::abs((y[i] - y_hat[i]) / (y[i] + y_hat[i]));
  }
  double res = sum / y.size();
  limit_loss(res, max_loss_);
  return res;
}

class colling : public loss_fn {
  private:
    NRMSD nrmsd_;
    constexpr static double max_loss_ = 1e100;
  public:
    double loss(dataset&, ast_ptr&);
};

double colling::loss(dataset& ds, ast_ptr& ast) {
  std::vector<double>& y = ds.y;
  std::vector<double>& x = ds.x;
  int step_size = x[1] - x[0]; 
  std::vector<double> d_y = util::numerical_derivative(y, step_size);
  std::vector<double> y_hat;
  for (std::size_t i = 0; i < x.size(); i++) {
    y_hat.push_back(ast->eval(x[i]));
  }
  std::vector<double> d_y_hat = util::numerical_derivative(y_hat, step_size);
  auto l = .5 * nrmsd_.loss(y, y_hat) + .5 * nrmsd_.loss(d_y, d_y_hat);
  limit_loss(l, max_loss_);
  return l;
};
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
  } else if (loss_fn_str == "MASE") {
    return std::make_shared<MAE>();
  } else if (loss_fn_str == "colling") {
    return std::make_shared<colling>(); 
  } else {
    return std::make_shared<MAPE>();
  }
}

} // loss
} // symreg

