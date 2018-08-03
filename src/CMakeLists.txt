add_executable (run run.cc)
target_include_directories (run PRIVATE ${include_dir})
target_link_libraries (run brick_ast)
target_link_libraries (run dlib::dlib)
