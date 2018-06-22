add_library(symreg_MCTS
  MCTS.cc
  search_node.cc
)

add_dependencies(symreg_MCTS brick_ast)
target_link_libraries(symreg_MCTS brick_ast)
