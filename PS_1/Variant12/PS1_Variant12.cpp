
#include <cstdint>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <windows.h>
#include <mutex>
#include <vector>
#include <unordered_map>
using namespace std;

int finished_threads = 0;
mutex prime_mutex, print_mutex, add_mutex;
vector<string> output_list_with_timestamps;
vector<uint64_t> prime_list;
vector<int> thread_list;
int prime_size = 0;
int print_index = 0;

void print_thread(int thread_num){

    bool is_done = false;
    while(!is_done){
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm{};
        localtime_s(&local_tm, &now_c);

        lock_guard<mutex> guard(prime_mutex);
        if(print_index < prime_size){
            ostringstream oss;
            oss << put_time(&local_tm, "[%H:%M:%S] ") << "Thread " << thread_list[print_index] << " found prime : " << prime_list[print_index] << endl;
            output_list_with_timestamps.push_back(oss.str());
            print_index++;            
        }

        if(finished_threads >= thread_num && print_index >= prime_size){
            is_done = true;
        }
    }
}

void check_prime(int id, uint64_t min, uint64_t max){
    Sleep(100); //Trust the process type shit, it just works with the delay

    bool prime = true;
    uint64_t sqrt_i, n;

    for(uint64_t i = min; i < max; i++){
        prime = true;
        n = (uint64_t)sqrt(i) / 6 + 1;
        if((i % 2 == 0 || i % 3 == 0) && (i != 2 && i != 3)){
            prime = false;
        }
        else{
            for(uint64_t j = 1; j <= n; j++){
                uint64_t d1 = j * 6 + 1;
                uint64_t d2 = j * 6 - 1;
                if((i % d1 == 0 || i % d2 == 0) && (i != d1 && i != d2)){
                    prime = false;
                    break;
                }
            }
        }
        if(prime){
            lock_guard<mutex> guard(prime_mutex);
            prime_list.push_back(i);
            thread_list.push_back(id);
            prime_size++;
        }
    }
    lock_guard<mutex> guard(add_mutex);
    finished_threads++;
}

void check_all(uint64_t n, int thread_num) {
    
    uint64_t division_n = (uint64_t)((n / thread_num)) + 1;
    vector<thread> threads;

    for(uint64_t i = 0; i < thread_num; i++){
        uint64_t min = 0 + division_n * i + (i == 0 ? 1 : 0);
        uint64_t max = division_n * (i + 1) ;
        if(max > n){
            max = n;
        }
        // cout << "Thread " << i << " checking " << min << " to " << max - 1 << endl;
        threads.emplace_back(check_prime, i, min, max);
    }

    thread print_thread_1(print_thread, thread_num);

    for(auto& th : threads){
        th.join();
    }

    print_thread_1.join();

}

unordered_map<string, string> read_config(const string& filename) {
    unordered_map<string, string> config;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        size_t eq_pos = line.find('=');
        if (eq_pos != string::npos) {
            string key = line.substr(0, eq_pos);
            string value = line.substr(eq_pos + 1);

            auto trim = [](string& s) {
                size_t start = s.find_first_not_of(" \t\r\n");
                size_t end = s.find_last_not_of(" \t\r\n");
                if (start == string::npos) { s.clear(); return; }
                s = s.substr(start, end - start + 1);
            };

            trim(key);
            trim(value);
            config[key] = value;
        }
    }
    return config;
}

bool is_valid_uint(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

int main(){
    uint64_t n;
    int thread_num;
    int variation;

    auto config = read_config("config.txt");

    // Validate num_threads
    if (config.find("num_threads") != config.end() && is_valid_uint(config["num_threads"])) {
        thread_num = stoi(config["num_threads"]);
        if (thread_num <= 0) {
            cerr << "Error: num_threads must be greater than 0." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'num_threads' in config file!" << endl;
        return 1;
    }

    // Validate max_num
    if (config.find("max_num") != config.end() && is_valid_uint(config["max_num"])) {
        try {
            n = stoull(config["max_num"]);
            if (n <= 1) {
                cerr << "Error: max_num must be greater than 1." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for max_num." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'max_num' in config file!" << endl;
        return 1;
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

    for(const string& output_line : output_list_with_timestamps) {
        cout << output_line;
    }
}