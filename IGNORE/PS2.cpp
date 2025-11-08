
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
#define LINE "------------------------------------------------------------------------------------------------------------------"

bool is_done = false;
vector<int> tank_queue;
vector<int> healer_queue;
vector<int> dps_queue;
vector<int> count;
vector<int> dungeon_free_list;
vector<int> dungeon_in_progress_list;
vector<string> dungeon_party_list;
vector<int> dungeon_time_list;
mutex tank_mutex, healer_mutex, dps_mutex, dungeon_mutex;
int t1, t2;
int dungeon_count = 0;

void add_tank(int id) {
    lock_guard<mutex> guard(tank_mutex);
    tank_queue.push_back(id);
}

void add_healer(int id) {
    lock_guard<mutex> guard(healer_mutex);
    healer_queue.push_back(id);
}

void add_dps(int id) {
    lock_guard<mutex> guard(dps_mutex);
    dps_queue.push_back(id);
}

int get_tank_size() {
    lock_guard<mutex> guard(tank_mutex);
    return tank_queue.size();
}

int get_healer_size() {
    lock_guard<mutex> guard(healer_mutex);
    return healer_queue.size();
}

int get_dps_size() {
    lock_guard<mutex> guard(dps_mutex);
    return dps_queue.size();
}

int get_tank() {
    lock_guard<mutex> guard(tank_mutex);
    if (!tank_queue.empty()) {
        int id = tank_queue.back();
        tank_queue.pop_back();
        return id;
    }
    return -1;
}

int get_healer() {
    lock_guard<mutex> guard(healer_mutex);
    if (!healer_queue.empty()) {
        int id = healer_queue.back();
        healer_queue.pop_back();
        return id;
    }
    return -1;
}

int get_dps() {
    lock_guard<mutex> guard(dps_mutex);
    if (!dps_queue.empty()) {
        int id = dps_queue.back();
        dps_queue.pop_back();
        return id;
    }
    return -1;
}

void create_tank_thread(){
    if(count[0] < 9999){
        count[0]++;
        add_tank(count[0]);
    }
    else{
        count[0] = -1;
    }
}

void create_healer_thread(){
    if(count[1] < 9999){
        count[1]++;
        add_healer(count[1]);
    }
    else{
        count[1] = -1;
    }
}

void create_dps_thread(){
    if(count[2] < 9999){
        count[2]++;
        add_dps(count[2]);
    }
    else{
        count[2] = -1;
    }
}

int get_most_available_dungeon(){
    lock_guard<mutex> guard(dungeon_mutex);
    if(!dungeon_free_list.empty()){
        int id = dungeon_free_list[0];
        dungeon_free_list.erase(dungeon_free_list.begin());
        return id;
    }
    return -1;
}

void dungeon_assign(){

}

void dungeon_free(){

}

void print_thread(){
    stringstream ss;
    while(!is_done){
        cout << LINE << endl;
        ss.str("");
        ss << "TNK QUEUE - " << setw(4) << setfill('0') << get_tank_size() << " | ";
        cout << ss.str();
        for(int i = 11; i > -1; i--){
            if(get_tank_size() > i){
                cout << "T" << setw(4) << setfill('0') << tank_queue[i] << " | ";
            }
            else{
                cout << "       ";
            }
        }
        cout << LINE << endl;
        ss.str("");
        ss << "HLR QUEUE - " << setw(4) << setfill('0') << get_healer_size() << " | ";
        cout << ss.str();
        for(int i = 11; i > -1; i--){
            if(get_healer_size() > i){
                cout << "H" << setw(4) << setfill('0') << healer_queue[i] << " | ";
            }
            else{
                cout << "       ";
            }
        }
        cout << LINE << endl;
        ss.str("");
        ss << "DPS QUEUE - " << setw(4) << setfill('0') << get_dps_size() << " | ";
        cout << ss.str();
        for(int i = 11; i > -1; i--){
            if(get_dps_size() > i){
                cout << "D" << setw(4) << setfill('0') << dps_queue[i] << " | ";
            }
            else{
                cout << "       ";
            }
        }
    }
}

void match_thread(){
    
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
    int instance_count;
    int tank_count;
    int healer_count;
    int dps_count;
    int min_instance_time;
    int max_instance_time;

    auto config = read_config("config.txt");

    // Validate n
    if (config.find("n") != config.end() && is_valid_uint(config["n"])) {
        try {
            instance_count = stoull(config["n"]);
            if (instance_count <= 1) {
                cerr << "Error: n must be greater than 1." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for n." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'n' in config file!" << endl;
        return 1;
    }

    // Validate t
    if (config.find("t") != config.end() && is_valid_uint(config["t"])) {
        try {
            tank_count = stoull(config["t"]);
            if (tank_count <= 1) {
                cerr << "Error: t must be greater than 1." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for t." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't' in config file!" << endl;
        return 1;
    }

    // Validate h
    if (config.find("h") != config.end() && is_valid_uint(config["h"])) {
        try {
            healer_count = stoull(config["h"]);
            if (healer_count < 0) { // Shouldn’t happen, but for completeness
                cerr << "Error: h cannot be negative." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for h." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'h' in config file!" << endl;
        return 1;
    }

    // Validate d
    if (config.find("d") != config.end() && is_valid_uint(config["d"])) {
        try {
            dps_count = stoull(config["d"]);
            if (dps_count < 0) {
                cerr << "Error: d cannot be negative." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for d." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'd' in config file!" << endl;
        return 1;
    }

    // Validate t1
    if (config.find("t1") != config.end() && is_valid_uint(config["t1"])) {
        try {
            min_instance_time = stoull(config["t1"]);
            if (min_instance_time == 0) {
                cerr << "Error: t1 must be greater than 0." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for t1." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't1' in config file!" << endl;
        return 1;
    }

    // Validate t2
    if (config.find("t2") != config.end() && is_valid_uint(config["t2"])) {
        try {
            max_instance_time = stoull(config["t2"]);
            if (max_instance_time < min_instance_time) {
                cerr << "Error: t2 must be greater than or equal to t1." << endl;
                return 1;
            }
        } catch (const exception& e) {
            cerr << "Error: Invalid number format for t2." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't2' in config file!" << endl;
        return 1;
    }
    
    for(int i = 0; i < dungeon_count; i++){
        dungeon_free_list.push_back(i);
        dungeon_party_list.push_back("EMPTY");
    }

    thread start = thread(match_thread);
    thread print = thread(print_thread);
    start.join();
    is_done = true;
    print.join();
}