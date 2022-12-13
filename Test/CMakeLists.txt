message(STATUS "Modules =  ${JSON_MODULES}")


add_executable(JSON_TEST "test.cpp" ${JSON_MODULES})