# symreg [![Build Status](https://travis-ci.org/gress2/symreg.svg?branch=master)](https://travis-ci.org/gress2/symreg)
Symbolic regression using MCTS

### Building
For first time set up you will need to do
```bash
git submodule update --init --recursive
```
to get necessary project dependencies. From there, building is typical for cmake/make:
```bash
mkdir build # if the directory doesn't already exist
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Running tree search to find symbolic expressions fitting a dataset
The easiest way to run a single tree search is to invoke the tree_search binary, passing a relative location to a .toml configuration file as an argument. For example:
```bash
# from build directory
./Release/bin/tree_search ../config/tree_search.toml
```
This should result in output looking something like this
```
Searching for expressions fitting data generated from f(x) = x^2-4*x+3

After exploring 8069 ASTs, here are the best 10 expressions I found:
[10] expr: ((((x-1)*x)+-5)+(4+5)) MSE: 30001
[9] expr: ((3+2)+(-x+(x^2))) MSE: 29998
[8] expr: ((5+(1/4))+((x^2)+-x)) MSE: 29997.6
[7] expr: ((3+3)+(-x+(x^2))) MSE: 29997
[6] expr: ((2+-x)+(5+(x^2))) MSE: 29998
[5] expr: ((4+4)+((x^2)+-x)) MSE: 30001
[4] expr: ((5+4)+(-x+(x^2))) MSE: 30006
[3] expr: (4+((x^3)/(3+x))) MSE: 3377.84
[2] expr: (((x-4)*x)+x) MSE: 3349
[1] expr: ((5+((x-5)*x))+x) MSE: 4
```
