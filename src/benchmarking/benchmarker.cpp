#include "benchmarker.hpp"
#include <sstream>

Benchmarker::Benchmarker(Logger* logger) : logger_(logger) {}

void Benchmarker::startEndToEnd() {
    end_to_end_start_ = std::chrono::high_resolution_clock::now();
}

void Benchmarker::endEndToEnd() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - end_to_end_start_);
    std::stringstream ss;
    ss << "End-to-End Simulation Loop Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}

void Benchmarker::recordDataProcessingLatency(std::chrono::microseconds duration) {
    std::stringstream ss;
    ss << "Data Processing Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}

void Benchmarker::recordUiUpdateLatency(std::chrono::microseconds duration) {
    std::stringstream ss;
    ss << "UI Update Latency: " << duration.count() << " μs";
    logger_->logInfo(ss.str());
}