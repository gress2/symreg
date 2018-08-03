add_executable (run run.cc)
target_include_directories (run PRIVATE ${include_dir})
target_link_libraries (run brick_ast)
target_link_libraries (run dlib::dlib)

add_executable (run2 run2.cc)
target_include_directories (run2 PRIVATE ${include_dir})
target_link_libraries (run2 brick_ast)
target_link_libraries (run2 dlib::dlib)
