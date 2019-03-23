cmake_minimum_required(VERSION 3.1.1 FATAL_ERROR)

############
# INCLUDES #
############
include(FetchContent)

#######
# GIT #
#######

# Git config
find_package(Git)
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND Git_FOUND)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-list --count --topo-order v${VERSION_FULL}..HEAD
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
endif()

if(WIN32)
	# Resource VersionInfo
	set(PROJECT_PRODUCT_NAME "Packet Library")
	set(PROJECT_COMPANY_NAME "Rodrigo Holztrattner Reis")
endif()

################
# DEPENDENCIES #
################

macro(PrintFetchMessage target_string)
	string(CONCAT end_string "Fetching " ${target_string})
	message(STATUS "")
	message(STATUS "################################")
	message(STATUS "# ${end_string}")
	message(STATUS "################################")
	message(STATUS "")
endmacro()

# CTTI (compite time type information for C++)
FetchContent_Declare(
  ctti
  GIT_REPOSITORY https://github.com/RodrigoHolztrattner/ctti
)

# Nlohmann JSON (modern JSON for C++)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json
)

# Concurrent queue
FetchContent_Declare(
  concurrentqueue
  GIT_REPOSITORY https://github.com/cameron314/concurrentqueue#moodycamelconcurrentqueue
)

FetchContent_GetProperties(ctti)
if(NOT ctti_POPULATED)
  PrintFetchMessage("CTTI")
  FetchContent_Populate(ctti)
  add_subdirectory(${ctti_SOURCE_DIR} ${ctti_BINARY_DIR})
endif()

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  PrintFetchMessage("Nlohmann JSON")
  FetchContent_Populate(json)
endif()

FetchContent_GetProperties(concurrentqueue)
if(NOT concurrentqueue_POPULATED)
  PrintFetchMessage("moodycamel::ConcurrentQueue")
  FetchContent_Populate(concurrentqueue)
endif()

###########
# PROJECT #
###########

set(PROJECT_NAME "Packet")
project(${PROJECT_NAME} VERSION 1.0.0 DESCRIPTION "blob")

add_library(${PROJECT_NAME} STATIC
Packet/PacketCondensedModeFileLoader.cpp
Packet/PacketCOndenser.cpp
Packet/PacketEditModeFileLoader.cpp
Packet/PacketReferenceManager.cpp
Packet/PacketScanner.cpp
Packet/PacketSystem.cpp
Packet/Resource/PacketResource.cpp
Packet/Resource/PacketResourceDeleter.cpp
Packet/Resource/PacketResourceFactory.cpp
Packet/Resource/PacketResourceHolder.cpp
Packet/Resource/PacketResourceInstance.cpp
Packet/Resource/PacketResourceLoader.cpp
Packet/Resource/PacketResourceManager.cpp
Packet/Resource/PacketResourceStorage.cpp
Packet/Resource/PacketResourceWatcher.cpp
Packet/ThirdParty/FileWatcher/FileWatcher.cpp
Packet/ThirdParty/FileWatcher/FileWatcherLinux.cpp
Packet/ThirdParty/FileWatcher/FileWatcherOSX.cpp
Packet/ThirdParty/FileWatcher/FileWatcherWin32.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER Packet/Packet.h)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Packet/ThirdParty/FileWatcher")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Packet/ThirdParty/FileWatcher")
target_include_directories(${PROJECT_NAME} PUBLIC ${ctti_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${json_SOURCE_DIR}/single_include)
target_include_directories(${PROJECT_NAME} PUBLIC ${concurrentqueue_SOURCE_DIR})

###########
# INSTALL #
###########

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT library)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON)    
	

add_definitions(-DUNICODE 
	-D_UNICODE)

#########
# TESTS #
#########

# Catch2 (for tests)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
)
FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
  PrintFetchMessage("Catch2")
  FetchContent_Populate(catch2)
endif()

set(TEST_PROJECT_NAME "Tests")
project(${TEST_PROJECT_NAME} VERSION 1.0.0 DESCRIPTION "blob")

SET(CMAKE_CXX_FLAGS "/Zi /sdl /MP /Oi /Gy /permissive- /GR- /TP /EHsc")
SET(CMAKE_CXX_FLAGS_DEBUG "/ZI /sdl /MP /JMC /permissive- /GR- /TP /EHsc /MDd /Od")

add_executable(${TEST_PROJECT_NAME} Tests/TestMain.cpp)
target_include_directories(${TEST_PROJECT_NAME} PUBLIC ${catch2_SOURCE_DIR}/single_include)

# Link agains our packet library
target_link_libraries(${TEST_PROJECT_NAME} "Packet")

# Request C++ 17
set_target_properties(${TEST_PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON)         