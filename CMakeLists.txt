# CMakeLists.txt for Raylib projects
# Credit: https://gist.github.com/erikyuzwa/98305851fc6f948805a92768c7e82705

cmake_minimum_required(VERSION 4.3)
project(nibbler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the command that downloads libraries
include(FetchContent)

# define a function for adding git dependencies
function(include_dependency libName gitURL gitTag)
    # setup the declare
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )

    FetchContent_MakeAvailable(${libName})
endfunction()

# add raylib support
set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    message(STATUS "Getting ${LIB1} from Github")
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 6.0)
else()
    message(STATUS "Using local ${LIB1}")
endif()

add_executable(nibbler main.cpp)

# set the include directory
target_include_directories(nibbler PRIVATE ${raylib_INCLUDE_DIRS})

# link all libraries to the project
target_link_libraries(nibbler PRIVATE ${LIB1})
