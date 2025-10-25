
#include <cstdint>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>
#include <limits>
#include <thread>
#include <windows.h>
#include <mutex>
#include <vector>
#include <fstream>
using namespace std;

int finished_threads = 0;
mutex output_mutex, print_mutex, flag_mutex;
vector<uint16_t> prime_list;
vector<string> output_list;
vector<int> thread_list;
vector<bool> flag_list;
bool global_is_prime = true;
bool global_is_done = false;
vector<uint64_t> check_list;
uint64_t global_num = 0;

void print_thread(int thread_num){

    bool is_done = false;
    while(!is_done){
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm{};
        localtime_s(&local_tm, &now_c);

        {
            lock_guard<mutex> guard(output_mutex);
            if(!output_list.empty()){
                lock_guard<mutex> guard2(print_mutex);
                cout << put_time(&local_tm, "[%H:%M:%S] ") << output_list.front() << endl;
                output_list.erase(output_list.begin());
            }
        }

        {
            lock_guard<mutex> guard(output_mutex);
            if(global_is_done && output_list.empty()){
                is_done = true;
            }
        }
    }
}

void check_div(int id){
    Sleep(100); //Trust the process type shit, it just works with the delay
    while(!global_is_done){
        lock_guard<mutex> guard(flag_mutex);
        if(!flag_list[id]){
            {
                lock_guard<mutex> guard2(output_mutex);
                output_list.push_back("Thread " + to_string(id) + " checking " + to_string(check_list[id]));
            }
            if(global_num % check_list[id] == 0 && global_num != check_list[id]){
                global_is_prime = false;
                {
                    lock_guard<mutex> guard2(output_mutex);
                    output_list.push_back("Thread " + to_string(id) + " found " + to_string(global_num) + " is not prime. Can be divided by " + to_string(check_list[id]));
                }
            }
            flag_list[id] = true;
        }
    }
}

void scheduler_thread(uint64_t n, int thread_num){
    uint64_t n_6;
    int thread_i = 0;
    int mode = -2;
    int j = 1;
    bool is_done = true;
    for(uint64_t i = 2; i < n; i++){
        n_6 = (uint64_t)sqrt(i) / 6 + 1;

        global_num = i;
        global_is_prime = true;
        is_done = false;
        j = 1;
        mode = -2;

        while(!is_done){
            if(flag_list[thread_i]){
                if(mode == 0){
                    check_list[thread_i] = 6*j-1;
                    mode = 1;
                }
                else if(mode == 1){
                    check_list[thread_i] = 6*j+1;
                    j++;
                    mode = 0;
                }
                else if(mode == -2){
                    check_list[thread_i] = 2;
                    mode = -1;
                }
                else if(mode == -1){
                    check_list[thread_i] = 3;
                    mode = 0;
                }
                flag_list[thread_i] = false;
            }
            thread_i ++;
            if(thread_i >= thread_num){
                thread_i = 0;
            }
            if(j > n_6){
                is_done = true;
                {
                    lock_guard<mutex> guard(output_mutex);
                    output_list.push_back(to_string(i) + " is prime.");
                    prime_list.push_back((uint16_t)i);
                }
            }
            if(!global_is_prime){
                is_done = true;
            }
        }
        
    }
    global_is_done = true;
}

void check_all(uint64_t n, int thread_num) {
    
    vector<thread> threads;

    for(int i = 0; i < thread_num; i++){
        check_list.push_back(0);
        flag_list.push_back(true);
        threads.emplace_back(check_div, i);
    }

    thread scheduler_thread_1(scheduler_thread, n, thread_num);
    thread print_thread_1(print_thread, thread_num);
    
    scheduler_thread_1.join();

    for(auto& th : threads){
        th.join();
    }

    print_thread_1.join();

}

int main(){
    uint64_t n;
    int thread_num;
    int variation;

    string input;
    cout << "Enter a number: ";
    while (true) {
        getline(cin, input);
        stringstream ss(input);

        if (ss >> n && n > 1 && ss.eof()) {
            break;
        }
        cout << "Invalid input. Please enter a positive integer greater than 1: ";
    }

    cout << "Enter number of threads: ";
    while (true) {
        getline(cin, input);
        stringstream ss(input);

        if (ss >> thread_num && thread_num > 0 && ss.eof()) {
            break;
        }
        cout << "Invalid input. Please enter a positive integer: ";
    }
    
    auto now_s = chrono::system_clock::now();
    time_t now_c_s = chrono::system_clock::to_time_t(now_s);
    tm local_tm_s{};
    localtime_s(&local_tm_s, &now_c_s);

    check_all(n, thread_num);

    auto now_e = chrono::system_clock::now();
    time_t now_c_e = chrono::system_clock::to_time_t(now_e);
    tm local_tm_e{};
    localtime_s(&local_tm_e, &now_c_e);

    cout << put_time(&local_tm_s, "[%H:%M:%S] ") << "Start time" << endl;
    cout << put_time(&local_tm_e, "[%H:%M:%S] ") << "End time" << endl;

    ofstream outfile("prime_21.txt"); // Ignore, sumakit ulo ko mag double check sa console so Im outputting the primes to a file
    if (!outfile) {
        cerr << "Error: Could not open file for writing." << endl;
        return 1;
    }
    for(uint64_t prime_num : prime_list){
        outfile << prime_num << endl;
    }
    outfile.close();
}