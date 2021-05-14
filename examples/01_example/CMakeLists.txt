add_executable(01_example main.cpp)
target_link_libraries(01_example
    PRIVATE project_options project_warnings CONAN_PKG::fmt)
