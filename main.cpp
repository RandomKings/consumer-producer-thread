#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <fstream>
#include <random>

const int LOWER_NUM = 1;
const int UPPER_NUM = 10000;
const int BUFFER_SIZE = 100;
const int MAX_COUNT = 10000;

std::stack<int> buffer;
std::mutex mutex;
std::condition_variable can_produce;
std::condition_variable can_consume;
int produced_numbers = 0;

void producer() {
    std::ofstream all_file("all.txt");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(LOWER_NUM, UPPER_NUM);

    for (int i = 0; i < MAX_COUNT; ++i) {
        std::unique_lock<std::mutex> lock(mutex);
        can_produce.wait(lock, [] { return buffer.size() < BUFFER_SIZE; });

        int num = distrib(gen);
        buffer.push(num); // Push to the stack
        produced_numbers++;
        
        all_file << num << std::endl;
        can_consume.notify_one();
    }
}

void consumer(bool is_even) {
    std::ofstream file(is_even ? "even.txt" : "odd.txt");

    while (true) {
        std::unique_lock<std::mutex> lock(mutex);
        can_consume.wait(lock, [&] { return !buffer.empty() || produced_numbers >= MAX_COUNT; });

        if (produced_numbers >= MAX_COUNT && buffer.empty()) {
            break;
        }

        if (!buffer.empty() && ((buffer.top() % 2 == 0) == is_even)) {
            file << buffer.top() << std::endl;
            buffer.pop(); // Pop from the stack
            can_produce.notify_one();
        }
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread_even(consumer, true);
    std::thread consumer_thread_odd(consumer, false);

    producer_thread.join();
    consumer_thread_even.join();
    consumer_thread_odd.join();

    std::cout << "Program completed successfully." << std::endl;
    return 0;
}
