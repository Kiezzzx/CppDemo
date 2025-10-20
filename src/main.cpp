#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <cstdint>
#include <mutex>
#include <deque>
#include <limits>
#include "../include/bitops.hpp"
#include "../include/ts_queue.hpp"

using namespace std::chrono;

// Configuration
static constexpr int kRunSeconds = 5;
static constexpr int kSensorPeriodMs = 200;
static constexpr int kLedPeriodMs    = 500;
static constexpr int kQueueCapacity  = 10;  
static constexpr float kTempThreshold = 35.0f;  
static constexpr float kTempMin = 20.0f;  
static constexpr float kTempMax = 40.0f;  
static constexpr int kMovingAvgWindow = 5;  
static uint32_t GPIO_REG = 0; 
static std::atomic<bool> running{true}; 

// Sensor data struct
struct SensorData {
    float temperature;
    std::chrono::steady_clock::time_point timestamp;
};

// Global data queue
static TSQueue<SensorData> dataQueue{kQueueCapacity};

// Mutex for thread-safe console output
static std::mutex coutMutex;

void sensorTask() {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(kTempMin, kTempMax);

    while (running.load()) {
        float n = dist(rng);
        
        // Create sensor data and push to queue
        SensorData data{n, std::chrono::steady_clock::now()};
        dataQueue.push(data);
        
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[SENSOR] temp=" << n << "degC\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kSensorPeriodMs));
    }
}

// Data processor task (consumer)
void dataProcessorTask() {
    static std::deque<float> tempWindow;  // Sliding window for moving average
    static float windowSum = 0.0f;        
    static int totalSamples = 0;          
    
    while (true) {
        SensorData data = dataQueue.pop();
        if (!running.load()) break;  // Check for exit condition
        
        totalSamples++;
        
        // Add new temperature to window
        tempWindow.push_back(data.temperature);
        windowSum += data.temperature;
        
        // Remove oldest value if window is full
        if (tempWindow.size() > kMovingAvgWindow) {
            windowSum -= tempWindow.front();
            tempWindow.pop_front();
        }
        
        // Calculate moving average
        float movingAvg = windowSum / tempWindow.size();
        
        // Data analysis and output
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[PROCESSOR] temp=" << data.temperature 
                      << "degC, moving_avg=" << movingAvg 
                      << "degC, window=" << tempWindow.size()
                      << ", total=" << totalSamples << "\n";
            
            // Simple anomaly detection
            if (data.temperature > kTempThreshold) {
                std::cout << "[WARNING] High temperature detected!\n";
            }
        }
    }
}

void ledTask() {
    bool led = false;
    while (running.load()) {
        tog_bit(GPIO_REG, 0); // Toggle bit 0
        led = !led;
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[LED] " << (led ? "ON " : "OFF") << " REG=0x" 
                      << std::hex << GPIO_REG << std::dec << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kLedPeriodMs));
    }
}

int main() {
    std::cout << "Starting three threads: Sensor(Producer) → Queue → DataProcessor(Consumer) + LED\n\n";
    
    std::thread t1(sensorTask);        
    std::thread t2(dataProcessorTask); 
    std::thread t3(ledTask);           

    std::this_thread::sleep_for(std::chrono::seconds(kRunSeconds));
    running.store(false); // Signal all threads to stop

    // Send a sentinel value to wake up the data processor
    dataQueue.push(SensorData{std::numeric_limits<float>::quiet_NaN(), steady_clock::now()});

    std::cout << "\nWaiting for all threads to finish...\n";
    t1.join(); 
    t2.join(); 
    t3.join(); 
    std::cout << "All threads finished\n";
    return 0;
}
