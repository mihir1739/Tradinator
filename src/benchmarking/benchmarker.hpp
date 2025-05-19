#ifndef BENCHMARKER_HPP
#define BENCHMARKER_HPP

#include <chrono>
#include <string>
#include "../utils/logger.hpp"

class Benchmarker {
public:
    explicit Benchmarker(Logger* logger);
    
    // Start/end methods for end-to-end latency
    void startEndToEnd();
    void endEndToEnd();
    
    // Record specific latencies
    void recordDataProcessingLatency(std::chrono::microseconds duration);
    void recordUiUpdateLatency(std::chrono::microseconds duration);

private:
    Logger* logger_;
    std::chrono::high_resolution_clock::time_point end_to_end_start_;
};

#endif // BENCHMARKER_HPP