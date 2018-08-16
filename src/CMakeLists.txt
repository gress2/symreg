add_executable (tree_search tree_search.cc)
target_include_directories (tree_search PRIVATE ${include_dir})
target_link_libraries (tree_search brick_ast)
target_link_libraries (tree_search dlib::dlib)

add_executable (ast_generator ast_generator.cc)
target_include_directories (ast_generator PRIVATE ${include_dir})
target_link_libraries (ast_generator brick_ast)
target_link_libraries (ast_generator dlib::dlib)

add_executable (ranker ranker.cc)
target_include_directories (ranker PRIVATE ${include_dir})
target_link_libraries (ranker brick_ast)
target_link_libraries (ranker dlib::dlib)

add_executable (training_ex_generator training_ex_generator.cc)
target_include_directories (training_ex_generator PRIVATE ${include_dir})
target_link_libraries (training_ex_generator brick_ast)
