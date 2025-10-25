
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
mutex max_mutex, add_mutex, print_mutex;
vector<uint64_t> min_list;
vector<uint64_t> cur_max_list;
vector<uint64_t> max_list;
uint64_t divisible = 1;

void print_thread(int thread_num){

    bool is_done = false;
    while(!is_done){
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm{};
        localtime_s(&local_tm, &now_c);

        for (int i = 0; i < thread_num; i++) {
            if(min_list[i] < max_list[i]){
                lock_guard<mutex> guard(print_mutex);
                uint64_t temp = cur_max_list[i];
                cout << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << i << " checked " << min_list[i] << " to " << temp << endl;
                min_list[i] = temp + 1;
            }
            else{
                lock_guard<mutex> guard(print_mutex);
                cout << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << i << " is done." << endl;
            }
        }

        Sleep(1000);

        if(finished_threads >= thread_num){
            is_done = true;
        }
    }

    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    tm local_tm{};
    localtime_s(&local_tm, &now_c);

    for (int i = 0; i < thread_num; i++) {
        if(min_list[i] < max_list[i]){
            lock_guard<mutex> guard(print_mutex);
            cout << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << i << " checked " << min_list[i] << " to " << max_list[i] << endl;
        }
        else{
            lock_guard<mutex> guard(print_mutex);
            cout << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << i << " is done." << endl;
        }
    }
}

void check_div(int id, int variation, uint64_t n, uint64_t min, uint64_t max){
    Sleep(100); //Trust the process type shit, it just works with the delay
    for(uint64_t i = min; i < max; i++){
        lock_guard<mutex> guard(max_mutex);
        cur_max_list[id] = i;
        uint64_t d1 = i * 6 + 1;
        uint64_t d2 = i * 6 - 1;
        if((n % d1 == 0) || (n % d2 == 0)){
            prime = false;
            divisible = i;
        }
        if(prime == false){
            break;
        }
    }
    lock_guard<mutex> guard(add_mutex);
    finished_threads++;
}

void check_fibonacci(uint64_t n, int variation, int thread_num) {
    uint64_t sqrt_n = (uint64_t)sqrt(n);
    uint64_t division_n = (uint64_t)((uint64_t)(sqrt_n / thread_num) / 6) + 1; 
    // cout << "Checking divisibility from 2 to " << sqrt_n << " using " << thread_num << " threads for every " << division_n << endl;

    vector<thread> threads;

    if(n % 2 == 0){
        prime = false;
        divisible = 0;
        return;
    }
    if(n % 3 == 0){
        prime = false;
        divisible = 0;
        return;
    }

    for(uint64_t i = 0; i < thread_num; i++){
        // Don't include 0 LMAO 1 yung magiging multiple HAHA
        uint64_t min = 0 + division_n * i + (i == 0 ? 1 : 0);
        min_list.push_back(min);
        cur_max_list.push_back(min);
        uint64_t max = division_n * (i + 1) ;
        max_list.push_back(max);
        // cout << "Thread " << i << " checking " << min << " to " << max - 1 << endl;
        threads.emplace_back(check_div, i, variation, n, min, max);
    }

    thread print_thread_1(print_thread, thread_num);
    print_thread_1.join();

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
    
    auto now_s = chrono::system_clock::now();
    time_t now_c_s = chrono::system_clock::to_time_t(now_s);
    tm local_tm_s{};
    localtime_s(&local_tm_s, &now_c_s);

    check_fibonacci(n, variation, thread_num);

    auto now_e = chrono::system_clock::now();
    time_t now_c_e = chrono::system_clock::to_time_t(now_e);
    tm local_tm_e{};
    localtime_s(&local_tm_e, &now_c_e);

    if(prime){
        cout << n << " is a prime number." << endl;
    } else {
        cout << n << " is not a prime number." << endl;
        if(divisible >= 1){
            if(n % (6*divisible+1) == 0){
                cout << n << " is divisible by " << 6*divisible+1 << endl;
            } else if(n % (6*divisible-1) == 0){
                cout << n << " is divisible by " << 6*divisible-1 << endl;
            }
        }
        else{
            if(n % 2 == 0){
                cout << n << " is divisible by 2" << endl;
            } else if(n % 3 == 0){
                cout << n << " is divisible by 3" << endl;
            }
        }
    }

    cout << put_time(&local_tm_s, "[%H:%M:%S] ") << "Start time" << endl;
    cout << put_time(&local_tm_e, "[%H:%M:%S] ") << "End time" << endl;
}