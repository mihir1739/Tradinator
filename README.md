# High-Performance Trading Simulator

## Overview
The Trading Simulator is a high-performance application designed to leverage real-time market data for estimating transaction costs and market impact. It provides a user-friendly interface for traders and analysts to simulate trading strategies and visualize results.

## Features
- Real-time market data integration
- Transaction cost estimation
- Market impact analysis
- Interactive user interface with multiple widgets
- Simulation results visualization

## Project Structure
```
tradinator
├── src
│   ├── main.cpp         
│   ├── core
    │    ├── models
    │    └── web
│   └── ui
├── include
├── benchmarking
├── test
├── resources
├── conanfile.txt
├── CMakeLists.txt
└── README.md
```

## Getting Started

### Prerequisites
- CMake
- Qt Framework
- Conan package manager

### Installation
1. Clone the repository:
   ```
   git clone <repository-url>
   cd tradinator
   ```

2. Install dependencies using Conan:
   ```
   conan install . --build=missing
   ```

3. Build the project using CMake:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

### Running the Application
After building the project, you can run the application from the build directory:
```
./tradinator
```

## Usage
- Input market data parameters through the user interface.
- Run simulations to analyze trading strategies.
- View transaction costs and market impact results in real-time.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for any enhancements or bug fixes.

## Optimisations
* Used smart pointers(as `std::unique_ptr`) for more efficient memory management and ownership control.
* Preallocations for bid and ask `std::vector` to avoid dynamic resizing for each iteration.
* Used `beast::flat_buffer` to avoid memory allocation on each websocket message.
* Added websocket compression in the client to enable reduced data transmission.
* Use RapidJSON’s in-situ parsing to avoid copying JSON strings during JSON Parsing.
* Stored bids/asks in a sorted container (`std::map`) to minimize cache misses during updates.
* QT's main loop is running `io_context` as well, eliminating the need for a separate thread.
* `std::map` ensures $ O(log n) $ updates and lookups for price levels, improving performance over linear searches.

## License
This project is licensed under the MIT License. See the LICENSE file for details.