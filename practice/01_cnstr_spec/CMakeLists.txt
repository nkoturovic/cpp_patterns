get_filename_component(target_name ${CMAKE_CURRENT_LIST_DIR} NAME)
add_executable(${target_name} main.cpp)
target_link_libraries(${target_name}
    PRIVATE project_options project_warnings CONAN_PKG::fmt)
