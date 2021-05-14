add_executable(02_example main.cpp)
target_link_libraries(02_example
    PRIVATE project_options project_warnings CONAN_PKG::fmt)
