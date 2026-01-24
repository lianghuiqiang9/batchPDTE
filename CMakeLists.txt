cmake_minimum_required(VERSION 3.16)
project(bpdte)

find_package(SEAL 4.1 REQUIRED)


set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -Wall")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cc")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_library(bpdte SHARED ${SOURCE_FILES})

target_include_directories(bpdte PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(bpdte PUBLIC SEAL::seal)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS bpdte LIBRARY DESTINATION lib)


set(TEST_SOURCES 
    cmp_main.cc 
    cmp_test.cc 
    lhe_depth_test.cc 
    lhe_test.cc 
    bpdte_main.cc 
    serial_test.cc
)

foreach(source_file ${TEST_SOURCES})

    get_filename_component(exe_name ${source_file} NAME_WE)
    
    if(EXISTS ${PROJECT_SOURCE_DIR}/${source_file})
        add_executable(${exe_name} ${source_file})
        
        target_include_directories(${exe_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
        
        target_link_libraries(${exe_name} bpdte SEAL::seal)
        
        set_target_properties(${exe_name} PROPERTIES 
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "${PROJECT_SOURCE_DIR}/lib"
        )
        
        message(STATUS "Created build target: ${exe_name}")
    endif()
endforeach()