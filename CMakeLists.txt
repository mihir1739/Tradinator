cmake_minimum_required(VERSION 3.14)

project(Tradinator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set Qt6 path (adjust if needed for your environment)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/Users/mihsingh/Qt/6.9.0/macos")

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${RAPIDJSON_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})

# Qt6 setup
qt_standard_project_setup()

# Source files
set(SOURCES
    src/benchmarking/benchmarker.cpp
    src/core/models/orderbook.cpp
    src/core/web/websocket.cpp
    src/utils/logger.cpp
    src/ui/mainwindow.cpp
    src/main.cpp
)

# Header files (for IDEs and clarity)
set(HEADERS
    src/benchmarking/benchmarker.hpp
    src/core/models/orderbook.hpp
    src/core/web/websocket.hpp
    src/utils/logger.hpp
    src/ui/mainwindow.hpp
)

# Create executable
add_executable(Tradinator ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(Tradinator PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Boost::system
    rapidjson
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    Eigen3::Eigen
)

# RapidJSON is header-only, ensure include path is set
target_include_directories(Tradinator PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

# Enable Qt MOC for signal/slot processing
set_target_properties(Tradinator PROPERTIES
    AUTOMOC ON
    AUTOUIC OFF  # No .ui files are used
)

# Set output directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)