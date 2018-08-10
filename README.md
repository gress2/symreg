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
Based on a config that looks something like this:
```toml
[dataset]
function = "x^2-4*x+3"
xmin = -100
xmax = 100
n = 100

[mcts]
num_simulations = 1500
scorer = "UCB1"
loss_fn = "MAPE"
leaf_picker = "recursive_heuristic_child_picker<UCB1>"
early_term_thresh = 0.999
depth_limit = 10
top_N = 10

[actions]
binary = ["addition", "subtraction", "multiplication", "division", "exponentiation"] 
unary = ["negate"]
functions = []
vars = ["x"]
scalar_min = 1
scalar_max = 5

[logging]
file = "somelog.log"
```

### Configuring a tree search
The monte carlo tree search is highly configurable from a .toml file. Its configuration is broken into 5 parts:

##### Dataset configuration
| Property | Type | Description |
| -------- | ---- | ----------- |
| function | string | The model to generate points y = f(x) from |
| xmin | integer | the lower bound on x values |
| xmax | integer | the upper bound on x values |
| n | integer | how many points hould exist in the dataset |

The n points of the dataset will be distributed evenly along [xmin, xmax]

##### MCTS configuration
| Property | Type | Description |
| -------- | ---- | ----------- |
| num_simulation | int | the number of times leaves are chosen to be expanded/rolled out between moves |
| scorer | string | the name of the scoring function used to evaluate the exploration/exploitation value of a search node |
| loss_fn | string | the name of the loss function used to evaluate the goodness of fit of an AST to the dataset | 
| leaf_picker | string | the name/method of choosing leaves to be expanded/rolled out from during simulation. in the case of the recursive_heurstic_child_picker, a scorer should be enclosed in angle brackets. for example, "recursive_heuristic_child_picker\<UCB1\>"|
| early_term_thresh | float | if, at any time, the MCTS algorithm encounters an AST whose reward (1 - loss_fn) is at least the early termination threshold, all subsequent searching will be stopped |
| depth_limit | int | the depth limit limits the AST search space to those ASTs whose total number of nodes is greater than depth_limit |
| top_N | int | by default, each monte carlo tree search instance maintains a priority queue of the best ASTs it encounters (according to their loss on the datset). this parameter controls the maximum size of the priority queue, and, by extension, the maximum number of reported ASTs at the end of the search. | 

This section configures the action search space. In our case, this means we are configuring the types of AST nodes we can appear in our searched trees. 

##### Action configuration
| Property | Type | Description |
| -------- | ---- | ----------- |
| binary | array<string> | a list of binary operations which be used, e.g., ["addition", "division"] |
| unary | array<string> | a list of unary operations which may be used, e.g., ["negate"] |
| functions | array<string> | this implementation supports more just elementary operands, such as addition. any functions existing in the Brick library may be be listed here. for example, ["cos", "sin"] |
| vars | array<string> | for now, this should just be ["x"] |
| scalar_min | int | for now, the search includes a range of scalar nodes. this makes forming expressions such as "x+2" possible. this parameter marks is the lower bound for scalars |
| scalar_max | int | the upper bound for scalars appearing as AST nodes in the search |
  
##### Logging configuration
| Property | Type | Description |
| -------- | ---- | ----------- |
| logging_file | string | the name of the logging file to write to |
