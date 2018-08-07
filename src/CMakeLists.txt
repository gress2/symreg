add_executable (tree_search tree_search.cc)
target_include_directories (tree_search PRIVATE ${include_dir})
target_link_libraries (tree_search brick_ast)
target_link_libraries (tree_search dlib::dlib)
