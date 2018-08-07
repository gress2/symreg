#pragma once

#include <random>

namespace symreg
{
static std::random_device rd;
static std::mt19937 mt(rd());
} // symreg

#include "dataset.hpp"
#include "fixed_size_priority_queue.hpp"
#include "dnn.hpp"
#include "policy_iteration_driver.hpp"
#include "MCTS/MCTS.hpp"
#include "MCTS/search_node.hpp"
