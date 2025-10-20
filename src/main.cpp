#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <cstdint>
#include "../include/bitops.hpp"

using namespace std::chrono;

// Configuration
static constexpr int kRunSeconds = 5;
static constexpr int kSensorPeriodMs = 200;
static constexpr int kLedPeriodMs    = 500;
static uint32_t GPIO_REG = 0; // Simulated GPIO register
static std::atomic<bool> running{true};  // Global running flag

void sensorTask() {
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist(20.0f, 40.0f);

    while (running.load()) {
        float v = dist(rng);
        std::cout << "[SENSOR] sample=" << v << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(kSensorPeriodMs));
    }
}

void ledTask() {
    bool led = false;
    while (running.load()) {
        tog_bit(GPIO_REG, 0); // Toggle bit 0
        led = !led;
        std::cout << "[LED] " << (led ? "ON " : "OFF") << " REG=0x" 
                  << std::hex << GPIO_REG << std::dec << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(kLedPeriodMs));
    }
}

int main() {
    std::cout << "=== Stage 0: minimal tasks + bitops ===\n";
    std::thread t1(sensorTask); // Sensor task
    std::thread t2(ledTask);   // LED task

    std::this_thread::sleep_for(std::chrono::seconds(kRunSeconds));
    running.store(false); // Signal threads to stop

    t1.join(); // Wait for sensor task to finish
    t2.join(); // Wait for LED task to finish
    std::cout << "Bye!\n";
    return 0;
}
