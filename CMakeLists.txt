cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/3rd/bin)

include(FetchContent)
FetchContent_Declare(SFML
    URL file://${CMAKE_SOURCE_DIR}/3rd/SFML-2.6.0.zip
    )
FetchContent_MakeAvailable(SFML)

add_executable(main)
target_sources(main PRIVATE
    src/main.cpp
    src/tetris.cpp
)
target_compile_features(main PRIVATE cxx_std_17)
# target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/3rd/include)
# target_link_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/3rd/lib)
target_link_libraries(main PRIVATE sfml-graphics sfml-window sfml-system sfml-audio sfml-network)