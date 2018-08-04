#include <iostream>

#include "symreg.hpp"

int main() {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x*x*x+x*x+x; },
    100,
    -100,
    100
  );

  using namespace symreg::MCTS;

  simulator::action_factory af(1);

  simulator::simulator sim(
    score::UCB1,
    loss::bind_loss_fn(loss::MAPE, ds),
    simulator::recursive_heuristic_child_picker{score::UCB1},
    af,
    10
  );

  MCTS mcts(1000, ds, sim);
  symreg::DNN neural_net(af.max_set_size());
  
  symreg::policy_iteration_driver driver(neural_net, mcts);
  driver.iterate();

  return 0;
}
