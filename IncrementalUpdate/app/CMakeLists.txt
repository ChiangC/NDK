# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

file(GLOB my_c_path src/main/cpp/bzip2/*.c)
add_library(
             BsPatch
             SHARED

             ${my_c_path}
             src/main/cpp/bspatch.c )

find_library(
              log-lib
              log )

target_link_libraries(
                       BsPatch
                       ${log-lib} )