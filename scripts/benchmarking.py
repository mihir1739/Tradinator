#!/usr/bin/env python3

import re
import os
import sys
from pathlib import Path
from statistics import mean, median, stdev

def analyze_latency(log_file_path, output_path):
    # Regular expressions to extract latency information
    data_processing_pattern = r'\[INFO\] Data Processing Latency: (\d+) μs'
    ui_update_pattern = r'\[INFO\] UI Update Latency: (\d+) μs'
    end_to_end_pattern = r'\[INFO\] End-to-End Simulation Loop Latency: (\d+) μs'
    
    # Lists to store latency values
    data_processing_latencies = []
    ui_update_latencies = []
    end_to_end_latencies = []
    
    # Process the log file
    with open(log_file_path, 'r') as file:
        for line in file:
            # Extract Data Processing Latency
            match = re.search(data_processing_pattern, line)
            if match:
                data_processing_latencies.append(int(match.group(1)))
                continue
                
            # Extract UI Update Latency
            match = re.search(ui_update_pattern, line)
            if match:
                ui_update_latencies.append(int(match.group(1)))
                continue
                
            # Extract End-to-End Simulation Loop Latency
            match = re.search(end_to_end_pattern, line)
            if match:
                end_to_end_latencies.append(int(match.group(1)))
    
    # Calculate statistics
    stats = {
        "Data Processing Latency": {
            "avg": mean(data_processing_latencies),
            "median": median(data_processing_latencies),
            "min": min(data_processing_latencies),
            "max": max(data_processing_latencies),
            "std_dev": stdev(data_processing_latencies),
            "count": len(data_processing_latencies)
        },
        "UI Update Latency": {
            "avg": mean(ui_update_latencies),
            "median": median(ui_update_latencies),
            "min": min(ui_update_latencies),
            "max": max(ui_update_latencies),
            "std_dev": stdev(ui_update_latencies),
            "count": len(ui_update_latencies)
        },
        "End-to-End Simulation Loop Latency": {
            "avg": mean(end_to_end_latencies),
            "median": median(end_to_end_latencies),
            "min": min(end_to_end_latencies),
            "max": max(end_to_end_latencies),
            "std_dev": stdev(end_to_end_latencies),
            "count": len(end_to_end_latencies)
        }
    }
    
    # Write results to output file
    with open(output_path, 'a') as file:
        file.write("TRADINATOR PERFORMANCE BENCHMARK SUMMARY\n")
        file.write("======================================\n\n")
        
        for metric, data in stats.items():
            file.write(f"{metric} Statistics:\n")
            file.write(f"  Average: {data['avg']:.2f} μs\n")
            file.write(f"  Median: {data['median']:.2f} μs\n")
            file.write(f"  Minimum: {data['min']} μs\n")
            file.write(f"  Maximum: {data['max']} μs\n")
            file.write(f"  Standard Deviation: {data['std_dev']:.2f} μs\n")
            file.write(f"  Sample Count: {data['count']}\n\n")
        
        # Additional insights
        overhead = stats["End-to-End Simulation Loop Latency"]["avg"] - (
            stats["Data Processing Latency"]["avg"] + stats["UI Update Latency"]["avg"])
        
        file.write("Performance Insights:\n")
        file.write(f"  System Overhead: {overhead:.2f} μs\n")
        file.write(f"  Data Processing / Total Time: {(stats['Data Processing Latency']['avg'] / stats['End-to-End Simulation Loop Latency']['avg'] * 100):.2f}%\n")
        file.write(f"  UI Update / Total Time: {(stats['UI Update Latency']['avg'] / stats['End-to-End Simulation Loop Latency']['avg'] * 100):.2f}%\n")

if __name__ == "__main__":
    # Find project root directory (assuming script is in 'scripts' folder)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # Define paths relative to project root
    log_file_path = project_root / "build" / "trade_simulator.log"
    output_path = project_root / "benchmarks.txt"
    
    # Verify log file exists
    if not log_file_path.exists():
        print(f"Error: Log file not found at {log_file_path}")
        print("Please specify the correct log file path as an argument:")
        print(f"Usage: {sys.argv[0]} [log_file_path]")
        
        # Allow user to specify log file path as an argument
        if len(sys.argv) > 1:
            log_file_path = Path(sys.argv[1])
            if not log_file_path.exists():
                print(f"Error: Specified log file {log_file_path} not found.")
                sys.exit(1)
        else:
            sys.exit(1)
    
    analyze_latency(log_file_path, output_path)
    print(f"Benchmark analysis completed. Results written to {output_path}")