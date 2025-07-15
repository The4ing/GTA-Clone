cmake_minimum_required (VERSION 3.26)

project (oop2_ex04)
set (MY_AUTHORS "Or_Ram_Atar-Romi_sinizkey")

include (cmake/CompilerSettings.cmake)

add_executable (${CMAKE_PROJECT_NAME})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address>)
if (NOT MSVC)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address>)
endif()

add_subdirectory (include)
add_subdirectory (src)
add_subdirectory (resources)

# 💡 Copy resources folder into build directory
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include (cmake/Zip.cmake)

set (SFML_LOCATION "C:/SFML/SFML-2.6.1")
set (SFML_DIR "${SFML_LOCATION}/lib/cmake/SFML")

# הוסף את רכיב audio כאן!
find_package(SFML 2.6 COMPONENTS graphics audio window system REQUIRED)

set_target_properties(sfml-main sfml-system sfml-window sfml-graphics sfml-audio sfml-network PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

target_link_libraries(${CMAKE_PROJECT_NAME} sfml-graphics sfml-audio sfml-window sfml-system sfml-main)

include (cmake/SFML.cmake)
