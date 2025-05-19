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
         ├── models
         └── web
│   ├── core
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

## License
This project is licensed under the MIT License. See the LICENSE file for details.