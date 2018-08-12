#include <fstream>
#include <iostream>
#include <vector>

#include "symreg.hpp"

// function that returns
// Pearson correlation coefficient.
float correlation_coefficient(std::vector<int>& x, std::vector<int>& y) {
  assert(x.size() == y.size());
  int n = x.size();

  float sum_x = 0;
  float sum_y = 0; 
  float sum_xy = 0;
  float ss_x = 0; 
  float ss_y = 0;
 
  for (int i = 0; i < n; i++) {
    // sum of elements of array X.
    sum_x = sum_x + x[i];
 
    // sum of elements of array Y.
    sum_y = sum_y + y[i];
 
    // sum of x[i] * y[i].
    sum_xy = sum_xy + x[i] * y[i];
 
    // sum of square of array elements.
    ss_x = ss_x + x[i] * x[i];
    ss_y = ss_y + y[i] * y[i];
  }
 
  // use formula for calculating
  // correlation coefficient.
  float corr = (float)(n * sum_xy - sum_x * sum_y) / 
          sqrt((n * ss_x -
             sum_x * sum_x) * 
             (n * ss_y -
             sum_y * sum_y));
 
  return corr;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    return 1;
  }
  std::ifstream f1(argv[1]);
  std::ifstream f2(argv[2]);

  std::vector<int> r1;
  std::vector<int> r2;

  int rank; 
  while (f1 >> rank) {
    r1.push_back(rank);
  }
  while (f2 >> rank) {
    r2.push_back(rank);
  }

  std::cout << correlation_coefficient(r1, r2) << std::endl;

  return 0;
}
