#include "benchmarker.hpp"
#include <sstream>

/**
 * @brief Constructs the Benchmarker object
 * 
 * @param logger Pointer to a Logger object
 */
Benchmarker::Benchmarker(Logger* logger) : logger_(logger) {}

/**
 * @brief Starts the end-to-end latency measurement
 * 
 * This method records the start time for the end-to-end latency measurement.
 */
void Benchmarker::startEndToEnd() {
    end_to_end_start_ = std::chrono::high_resolution_clock::now();
}

/**
 * @brief Ends the end-to-end latency measurement
 * 
 * This method calculates and logs the end-to-end latency.
 */
void Benchmarker::endEndToEnd() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - end_to_end_start_);
    std::stringstream ss;
    ss << "End-to-End Simulation Loop Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}

/**
 * @brief Records the data processing latency
 * 
 * This method logs the data processing latency.
 * 
 * @param duration The duration of the data processing in microseconds
 */
void Benchmarker::recordDataProcessingLatency(std::chrono::microseconds duration) {
    std::stringstream ss;
    ss << "Data Processing Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}

/**
 * @brief Records the UI update latency
 * 
 * This method logs the UI update latency.
 * 
 * @param duration The duration of the UI update in microseconds
 */
void Benchmarker::recordUiUpdateLatency(std::chrono::microseconds duration) {
    std::stringstream ss;
    ss << "UI Update Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}