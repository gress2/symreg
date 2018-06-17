macro (setup_test test_name test_file)
  add_executable (${test_name} ${test_file})
  target_include_directories (${test_name} PRIVATE ${include_dir})
  target_link_libraries (${test_name} brick_ast gtest_main)
  add_test (${test_name} ${TEST_RUNTIME_OUTPUT_DIRECTORY}/${test_name})
endmacro ()

setup_test (MCTS_tests MCTS.cc)

