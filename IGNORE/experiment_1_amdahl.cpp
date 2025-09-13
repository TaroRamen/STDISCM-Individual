
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

mutex print_mutex;

void loop_num(int id, int min, int max) {
    auto start = chrono::high_resolution_clock::now();

    int j = 0;
    for (int i = min; i < max; i++) {
        j++;
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

    lock_guard<mutex> guard(print_mutex);
    cout << "Thread " << id 
         << " from " << min 
         << " to " << max 
         << " total time " << duration << " microseconds." << endl;
}


void amdahl(int n, int variation, int thread_num) {

    int division_n = (int)(n / thread_num);

    vector<thread> threads;

    for(uint64_t i = 0; i < thread_num; i++){
        uint64_t min = 0 + division_n * i;
        uint64_t max = division_n * (i + 1) - 1;

        if(i == thread_num - 1){
            max = n;
        }

        threads.emplace_back(loop_num, i, min, max);
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
    amdahl(n, variation, thread_num);
}