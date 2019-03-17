# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.13.0-rc1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(concurrentqueue-populate NONE)

include(ExternalProject)
ExternalProject_Add(concurrentqueue-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/cameron314/concurrentqueue"
                    SOURCE_DIR          "C:/Users/rodri/Documents/Packet/build/_deps/concurrentqueue-src"
                    BINARY_DIR          "C:/Users/rodri/Documents/Packet/build/_deps/concurrentqueue-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
)
