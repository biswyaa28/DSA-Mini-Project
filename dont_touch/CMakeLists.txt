cmake_minimum_required(VERSION 3.16)
project(railway_webui_task1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
  HINTS /opt/homebrew/include /usr/local/include /usr/include
)

find_package(Threads REQUIRED)

add_executable(railway_app ../src/main.cpp ../src/api_server.cpp)
target_include_directories(railway_app PRIVATE ../include external ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(railway_app PRIVATE railway_core Threads::Threads)

add_library(railway_core
  ../src/station_directory.cpp
  ../src/rail_graph.cpp
  ../src/pricing_engine.cpp
  ../src/booking_engine.cpp
  ../src/routing_engine.cpp
  ../src/mst_engine.cpp
  ../src/sort_search.cpp
  ../src/report_engine.cpp
  ../src/storage_manager.cpp
)
target_include_directories(railway_core PUBLIC ../include ${NLOHMANN_INCLUDE_DIR})

enable_testing()
add_executable(test_models ../tests/test_models.cpp)
target_include_directories(test_models PRIVATE ../include)
add_test(NAME test_models COMMAND test_models)

add_executable(test_graph_basics ../tests/test_graph_basics.cpp)
target_include_directories(test_graph_basics PRIVATE ../include)
target_link_libraries(test_graph_basics PRIVATE railway_core)
add_test(NAME test_graph_basics COMMAND test_graph_basics)

add_executable(test_booking_pricing ../tests/test_booking_pricing.cpp)
target_include_directories(test_booking_pricing PRIVATE ../include)
target_link_libraries(test_booking_pricing PRIVATE railway_core)
add_test(NAME test_booking_pricing COMMAND test_booking_pricing)

add_executable(test_routing_mst ../tests/test_routing_mst.cpp)
target_include_directories(test_routing_mst PRIVATE ../include)
target_link_libraries(test_routing_mst PRIVATE railway_core)
add_test(NAME test_routing_mst COMMAND test_routing_mst)

add_executable(test_sort_search_reports ../tests/test_sort_search_reports.cpp)
target_include_directories(test_sort_search_reports PRIVATE ../include ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(test_sort_search_reports PRIVATE railway_core)
add_test(NAME test_sort_search_reports COMMAND test_sort_search_reports)

add_executable(test_api_smoke ../tests/test_api_smoke.cpp ../src/api_server.cpp)
target_include_directories(test_api_smoke PRIVATE ../include external ${NLOHMANN_INCLUDE_DIR})
target_link_libraries(test_api_smoke PRIVATE railway_core Threads::Threads)
add_test(NAME test_api_smoke COMMAND test_api_smoke)
