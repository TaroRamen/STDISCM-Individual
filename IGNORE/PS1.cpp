
#include <cstdint>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <thread>
#include <windows.h>
#include <mutex>
#include <vector>
using namespace std;

bool prime = true;
int finished_threads = 0;
mutex print_mutex, add_mutex;

void check_div(int id, int variation, uint64_t n, uint64_t min, uint64_t max){
    for(uint64_t i = min; i < max; i++){

        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm{};
        localtime_s(&local_tm, &now_c);

        lock_guard<mutex> guard(print_mutex);
        cout << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << id << " checking " << i << endl;
        if(n % i == 0){
            cout << n << " is no longer a prime number." << endl;
            prime = false;
        }
        if(prime == false){
            i = max;
        }
    }
    lock_guard<mutex> guard(add_mutex);
    finished_threads++;
}

void check_fibonacci(uint64_t n, int variation, int thread_num) {
    uint64_t sqrt_n = (uint64_t)sqrt(n);
    uint64_t division_n = (uint64_t)(sqrt_n / thread_num) + 1; 
    cout << "Checking divisibility from 2 to " << sqrt_n << " using " << thread_num << " threads for every " << division_n << endl;
    
    Sleep(1000);

    vector<thread> threads;

    for(uint64_t i = 0; i < thread_num; i++){
        uint64_t min = 0 + division_n * i + 2;
        uint64_t max = division_n * (i + 1) + 2;
        threads.emplace_back(check_div, variation, i, n, min, max);
    }

    for(auto& th : threads){
        th.join();
    }
}

int main(){
    uint64_t n;
    int thread_num;
    int variation;
    cout << "Enter a number: ";
    while (!(cin >> n) || n <= 1) {
        cout << "Invalid input. Please enter a positive integer greater than 1: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Enter number of threads: ";
    while (!(cin >> thread_num) || thread_num <= 0) {
        cout << "Invalid input. Please enter a positive integer: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Variation ";
    while (!(cin >> variation) || variation < 1 || variation > 4) {
        cout << "Invalid input. Please enter a positive integer between 1 to 4: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    check_fibonacci(n, variation, thread_num);

    if(prime){
        cout << n << " is a prime number." << endl;
    } else {
        cout << n << " is not a prime number." << endl;
    }
}