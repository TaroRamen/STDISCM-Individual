#include <cstdint>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <conio.h>
#include <windows.h>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>
#include <random>
using namespace std;
#define LINE "-------------------------------------------------------------------------------------------------------------"

bool is_done = false;
queue<int> tank_queue;
queue<int> healer_queue;
queue<int> dps_queue;
vector<int> count; 
queue<int> dungeon_free_list;
vector<int> dungeon_in_progress_list; 
vector<string> dungeon_party_list; 
vector<int> dungeon_time_list; 
vector<int> dungeon_served_count;
vector<int> dungeon_served_time;
mutex tank_mutex, healer_mutex, dps_mutex, dungeon_mutex;
int t1_global = 1, t2_global = 1, t3_global = 1, t4_global = 1;
bool bonus_global = false;
int t_max_global = 1, h_max_global = 1, d_max_global = 1;
int dungeon_count = 0;
bool generators_started = false;
int dungeon_print_buffer = 0;
int print_pointer = 0;
string input = "";
HANDLE h_console;

int rand_range(int min_val, int max_val) {
    static random_device rd;
    static mt19937 rng(rd());
    uniform_int_distribution<int> dist(min_val, max_val);
    return dist(rng);
}


// ================= TANK QUEUE HELPERS =================
void add_tank(int id) {
    lock_guard<mutex> guard(tank_mutex);
    tank_queue.push(id);
}

int get_tank_size() {
    lock_guard<mutex> guard(tank_mutex);
    return (int)tank_queue.size();
}

void create_tank() {
    lock_guard<mutex> guard(tank_mutex);
    if (count[0] < 99999) {
        count[0]++;
        tank_queue.push(count[0]);
    } else {
        count[0] = -1;
    }
}

int get_tank() {
    lock_guard<mutex> guard(tank_mutex);
    if (!tank_queue.empty()) {
        int id = tank_queue.front();
        tank_queue.pop();
        return id;
    }
    return -1;
}

int get_tank_at(int idx) {
    lock_guard<mutex> guard(tank_mutex);
    if (idx >= 0 && idx < (int)tank_queue.size()) {
        // Need to iterate through queue to get element at index
        queue<int> temp = tank_queue;
        for (int i = 0; i < idx; i++) {
            temp.pop();
        }
        return temp.front();
    }
    return -1;
}


// ================= HEALER QUEUE HELPERS =================
void add_healer(int id) {
    lock_guard<mutex> guard(healer_mutex);
    healer_queue.push(id);
}

int get_healer_size() {
    lock_guard<mutex> guard(healer_mutex);
    return (int)healer_queue.size();
}

void create_healer() {
    lock_guard<mutex> guard(healer_mutex);
    if (count[1] < 99999) {
        count[1]++;
        healer_queue.push(count[1]);
    } else {
        count[1] = -1;
    }
}

int get_healer() {
    lock_guard<mutex> guard(healer_mutex);
    if (!healer_queue.empty()) {
        int id = healer_queue.front();
        healer_queue.pop();
        return id;
    }
    return -1;
}

int get_healer_at(int idx) {
    lock_guard<mutex> guard(healer_mutex);
    if (idx >= 0 && idx < (int)healer_queue.size()) {
        queue<int> temp = healer_queue;
        for (int i = 0; i < idx; i++) {
            temp.pop();
        }
        return temp.front();
    }
    return -1;
}


// ================= DPS QUEUE HELPERS =================
void add_dps(int id) {
    lock_guard<mutex> guard(dps_mutex);
    dps_queue.push(id);
}

int get_dps_size() {
    lock_guard<mutex> guard(dps_mutex);
    return (int)dps_queue.size();
}

void create_dps() {
    lock_guard<mutex> guard(dps_mutex);
    if (count[2] < 99999) {
        count[2]++;
        dps_queue.push(count[2]);
    } else {
        count[2] = -1;
    }
}

int get_dps() {
    lock_guard<mutex> guard(dps_mutex);
    if (!dps_queue.empty()) {
        int id = dps_queue.front();
        dps_queue.pop();
        return id;
    }
    return -1;
}

int get_dps_at(int idx) {
    lock_guard<mutex> guard(dps_mutex);
    if (idx >= 0 && idx < (int)dps_queue.size()) {
        queue<int> temp = dps_queue;
        for (int i = 0; i < idx; i++) {
            temp.pop();
        }
        return temp.front();
    }
    return -1;
}


// ---------- generator helpers used by main to pre-fill queues ----------
void create_tank_thread() {
    while (!is_done) {
        for(int i = 0; i < rand_range(1,t_max_global); i++){
            create_tank();
        }
        this_thread::sleep_for(chrono::seconds(rand_range(t3_global, t4_global)));
    }
}

void create_healer_thread() {
    while (!is_done) {
        for(int i = 0; i < rand_range(1,h_max_global); i++){
            create_healer();
        }
        this_thread::sleep_for(chrono::seconds(rand_range(t3_global, t4_global)));
    }
}

void create_dps_thread() {
    while (!is_done) {
        for(int i = 0; i < rand_range(1,d_max_global); i++){
            create_dps();
        }
        this_thread::sleep_for(chrono::seconds(rand_range(t3_global, t4_global)));
    }
}

void queue_watcher_thread() {

    while (!is_done) {
        if (!generators_started && 
            (
                get_tank_size() == 0 
                || get_healer_size() == 0 
                || get_dps_size() < 3
            )
        ) {
            thread(create_tank_thread).detach();
            thread(create_healer_thread).detach();
            thread(create_dps_thread).detach();

            generators_started = true;
        }

        this_thread::sleep_for(chrono::seconds(10));
    }
}

// ---------- dungeon helpers ----------
int get_most_available_dungeon(){
    lock_guard<mutex> guard(dungeon_mutex);
    if(!dungeon_free_list.empty()){
        int id = dungeon_free_list.front();
        dungeon_free_list.pop();
        dungeon_in_progress_list[id] = 1;
        return id;
    }
    return -1;
}

void dungeon_free(int instance_id, int time_served_seconds) {
    lock_guard<mutex> guard(dungeon_mutex);
    dungeon_served_time[instance_id] += time_served_seconds;
    dungeon_party_list[instance_id] = "EMPTY";
    dungeon_time_list[instance_id] = 0;
    dungeon_in_progress_list[instance_id] = 0;
    dungeon_free_list.push(instance_id);
}

void dungeon_assign(int instance_id, int assigned_time_seconds) {
    int tank_id = get_tank();
    int healer_id = get_healer();
    int d1 = get_dps();
    int d2 = get_dps();
    int d3 = get_dps();

    {
        lock_guard<mutex> guard(dungeon_mutex);
        stringstream ss;
        ss << "T" << setw(5) << setfill('0') << tank_id
           << " H" << setw(5) << setfill('0') << healer_id
           << " D" << setw(5) << setfill('0') << d1
           << " D" << setw(5) << setfill('0') << d2
           << " D" << setw(5) << setfill('0') << d3;
        dungeon_party_list[instance_id] = ss.str();
        dungeon_time_list[instance_id] = assigned_time_seconds;
        dungeon_served_count[instance_id] += 1;
    }

    // Launch dedicated instance thread for this dungeon
    thread([instance_id, assigned_time_seconds]() {
        int remaining = assigned_time_seconds;
        while (remaining > 0) {
            {
                lock_guard<mutex> guard(dungeon_mutex);
                dungeon_time_list[instance_id] = remaining;
            }
            this_thread::sleep_for(chrono::seconds(1));
            remaining--;
        }
        dungeon_free(instance_id, assigned_time_seconds);
    }).detach();
}

// ---------- scheduler (match_thread) ----------
void match_thread() {

    while (!is_done) {
        int tanks = get_tank_size();
        int healers = get_healer_size();
        int dpss = get_dps_size();

        if (tanks >= 1 && healers >= 1 && dpss >= 3) {
            int instance_id = -1;
            {
                lock_guard<mutex> guard(dungeon_mutex);
                if (!dungeon_free_list.empty()) {
                    instance_id = dungeon_free_list.front();
                    dungeon_free_list.pop();
                    dungeon_in_progress_list[instance_id] = 1;
                }
            }
            if (instance_id >= 0) {
                dungeon_assign(instance_id, rand_range(t1_global, t2_global));
            }
        }
        else{
            if (!bonus_global) {
                bool any_active = false;
                {
                    lock_guard<mutex> guard(dungeon_mutex);
                    for (int status : dungeon_in_progress_list) {
                        if (status == 1) {
                            any_active = true;
                            break;
                        }
                    }
                }
                if (!any_active) {
                    this_thread::sleep_for(chrono::seconds(1));
                    is_done = true;
                }
            }
        }
    }
}

// ---------- print thread (periodic display) ----------
void MoveCursorTo(COORD coord) { SetConsoleCursorPosition(h_console, coord); }
void ClearCurrentLine() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count_written;
    
    GetConsoleScreenBufferInfo(h_console, &csbi);
    COORD line_start = { 0, csbi.dwCursorPosition.Y };
    FillConsoleOutputCharacter(h_console, ' ', csbi.dwSize.X, line_start, &count_written);
}

void print_thread() {
    vector<string> output_list;
    stringstream ss;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h_console, &csbi);

    int max_height = csbi.srWindow.Bottom - csbi.srWindow.Top - 3;
    dungeon_print_buffer = max_height - 12;
    if (dungeon_print_buffer > dungeon_count) {
        dungeon_print_buffer = dungeon_count;
    }

    while (!is_done) {
        output_list.clear();
        
        // --- Tank Queue ---
        ss.str(""); ss.clear();
        ss << "TNK QUEUE - " << setw(5) << setfill('0') << get_tank_size() << " | ";
        for (int i = 9; i > -1; i--) {
            int id = get_tank_at(i);
            if (id >= 0) ss << "T" << setw(5) << setfill('0') << id << " | ";
            else ss << "         ";
        }
        output_list.push_back(LINE);
        output_list.push_back(ss.str());

        // --- Healer Queue ---
        ss.str(""); ss.clear();
        ss << "HLR QUEUE - " << setw(5) << setfill('0') << get_healer_size() << " | ";
        for (int i = 9; i > -1; i--) {
            int id = get_healer_at(i);
            if (id >= 0) ss << "H" << setw(5) << setfill('0') << id << " | ";
            else ss << "         ";
        }
        output_list.push_back(LINE);
        output_list.push_back(ss.str());

        // --- DPS Queue ---
        ss.str(""); ss.clear();
        ss << "DPS QUEUE - " << setw(5) << setfill('0') << get_dps_size() << " | ";
        for (int i = 9; i > -1; i--) {
            int id = get_dps_at(i);
            if (id >= 0) ss << "D" << setw(5) << setfill('0') << id << " | ";
            else ss << "         ";
        }
        output_list.push_back(LINE);
        output_list.push_back(ss.str());
        output_list.push_back(LINE);

        // --- Dungeon Instances ---
        {
            lock_guard<mutex> guard(dungeon_mutex);
            int start = print_pointer;
            int end = min(print_pointer + dungeon_print_buffer, dungeon_count);
            for (int i = start; i < end; ++i) {
                ss.str(""); ss.clear();
                ss << "Dungeon " << setw(3) << setfill(' ') << i << " : ";
                if (dungeon_in_progress_list[i]) {
                    ss << "ACTIVE | remaining: " << setw(3) << setfill(' ') << dungeon_time_list[i]
                       << "s | party: " << dungeon_party_list[i];
                } else {
                    ss << "EMPTY  | served: " << dungeon_served_count[i]
                       << " | total time: " << dungeon_served_time[i] << "s";
                }
                output_list.push_back(ss.str());
            }
        }

        output_list.push_back(LINE);
        output_list.push_back("Commands: 'exit' to quit | Up/Down arrows to scroll instances");
        output_list.push_back(input + "_");

        // --- Clear and Redraw ---
        for (int i = 0; i < output_list.size(); ++i) {
            MoveCursorTo({0, (short)i});
            ClearCurrentLine();
            cout << output_list[i] << "\n";  
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

// ---------- input thread ----------
void input_thread(){

    while(!is_done){
        bool is_command_done = false;
        if(_kbhit()){
            char key = _getch();
            if (key == '\r' || key == '\n') {
                is_command_done = true;
            }
            else if(key == 8){
                if((input).length() != 0){
                    input.pop_back();
                }
            }
            else if (key == 0 || key == -32) { // arrow/function key
                key = _getch(); // get actual code
                if (key == 72) { // Up
                    if (print_pointer > 0) print_pointer--;
                } else if (key == 80) { // Down
                    if (print_pointer + dungeon_print_buffer < dungeon_count) print_pointer++;
                }
            }
            else{
                input += key;
            }
        }

        if(is_command_done){
            
            if(input == "exit"){
                is_done = true;
            }
            is_command_done = false;
            input = "";
        }
    }
}

// ---------- config / util ----------
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
    system("cls");

    int tank_count = 0;
    int healer_count = 0;
    int dps_count = 0;

    h_console = GetStdHandle(STD_OUTPUT_HANDLE);

    auto config = read_config("config.txt");

    // Validate n
    if (config.find("n") != config.end() && is_valid_uint(config["n"])) {
        try {
            dungeon_count = stoull(config["n"]);
            if (dungeon_count <= 0) {
                cerr << "Error: n must be greater than 0." << endl;
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
            if (tank_count < 0) {
                cerr << "Error: t cannot be negative." << endl;
                return 1;
            }    
            if (tank_count > 100000) {
                cerr << "Error: t cannot be greater than 100000." << endl;
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
            if (healer_count < 0) {
                cerr << "Error: h cannot be negative." << endl;
                return 1;
            }
            if (healer_count > 100000) {
                cerr << "Error: h cannot be greater than 100000." << endl;
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
            if (dps_count > 100000) {
                cerr << "Error: d cannot be greater than 100000." << endl;
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
            t1_global = stoull(config["t1"]);
            if (t1_global <= 0) {
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
            t2_global = stoull(config["t2"]);
            if (t2_global < t1_global) {
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

    // Validate bonus
    if (config.find("bonus") != config.end()) {
        string val = config["bonus"];
        for (auto & c : val) c = tolower(c); // make lowercase
        if (val == "true") bonus_global = true;
        else if (val == "false") bonus_global = false;
        else {
            cerr << "Error: Invalid value for bonus. Must be true or false." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Missing 'bonus' in config file!" << endl;
        return 1;
    }

    // Validate t3
    if (config.find("t3") != config.end() && is_valid_uint(config["t3"])) {
        t3_global = stoi(config["t3"]);
        if (t3_global <= 0) {
            cerr << "Error: t3 must be greater than 0." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't3' in config file!" << endl;
        return 1;
    }

    // Validate t4
    if (config.find("t4") != config.end() && is_valid_uint(config["t4"])) {
        t4_global = stoi(config["t4"]);
        if (t4_global < t3_global) {
            cerr << "Error: t4 must be greater than or equal to t3." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't4' in config file!" << endl;
        return 1;
    }

    // Validate t_max
    if (config.find("t_max") != config.end() && is_valid_uint(config["t_max"])) {
        t_max_global = stoi(config["t_max"]);
        if (t_max_global <= 0) {
            cerr << "Error: t_max must be greater than 0." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 't_max' in config file!" << endl;
        return 1;
    }

    // Validate h_max
    if (config.find("h_max") != config.end() && is_valid_uint(config["h_max"])) {
        h_max_global = stoi(config["h_max"]);
        if (h_max_global <= 0) {
            cerr << "Error: h_max must be greater than 0." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'h_max' in config file!" << endl;
        return 1;
    }

    // Validate d_max
    if (config.find("d_max") != config.end() && is_valid_uint(config["d_max"])) {
        d_max_global = stoi(config["d_max"]);
        if (d_max_global <= 0) {
            cerr << "Error: d_max must be greater than 0." << endl;
            return 1;
        }
    } else {
        cerr << "Error: Invalid or missing 'd_max' in config file!" << endl;
        return 1;
    }

    count = vector<int>(3, 0);

    // Clear queue (if needed) - queues are empty by default
    while (!dungeon_free_list.empty()) dungeon_free_list.pop();
    
    dungeon_in_progress_list.assign(dungeon_count, 0);
    dungeon_party_list.assign(dungeon_count, string("EMPTY"));
    dungeon_time_list.assign(dungeon_count, 0);
    dungeon_served_count.assign(dungeon_count, 0);
    dungeon_served_time.assign(dungeon_count, 0);

    for(int i = 0; i < dungeon_count; i++){
        dungeon_free_list.push(i);
    }

    for (int i = 0; i < tank_count; ++i) {
        create_tank();
    }
    for (int i = 0; i < healer_count; ++i) {
        create_healer();
    }
    for (int i = 0; i < dps_count; ++i) {
        create_dps();
    }

    thread start = thread(match_thread);
    thread print = thread(print_thread);
    thread input_listener = thread(input_thread);

    if(bonus_global){
        thread(queue_watcher_thread).detach();
    }
    start.join();
    print.join();
    input_listener.join();

    // Print summary to console and file
    ofstream summary_file("summary.txt");
    if (!summary_file.is_open()) {
        cerr << "Error: Could not open summary.txt for writing!" << endl;
        return 0;
    }

    cout << "\n\nSUMMARY (per instance):\n";
    summary_file << "SUMMARY (per instance):\n";

    for (int i = 0; i < dungeon_count; ++i) {
        cout << "Instance " << i << " - served parties: " << dungeon_served_count[i]
            << " | total time served: " << dungeon_served_time[i] << "s" << endl;

        summary_file << "Instance " << i << " - served parties: " << dungeon_served_count[i]
                    << " | total time served: " << dungeon_served_time[i] << "s" << endl;
    }

    int total_parties = 0, total_time = 0;
    for (int i = 0; i < dungeon_count; ++i) {
        total_parties += dungeon_served_count[i];
        total_time += dungeon_served_time[i];
    }

    cout << "\nTotal parties served: " << total_parties 
        << " | total time served across all instances: " << total_time << "s\n";

    summary_file << "\nTotal parties served: " << total_parties 
                << " | total time served across all instances: " << total_time << "s\n";

    int remaining_tanks = get_tank_size();
    int remaining_healers = get_healer_size();
    int remaining_dps = get_dps_size();

    cout << "\nRemaining Players in Queues:\n";
    cout << "Tanks: " << remaining_tanks << " | Healers: " << remaining_healers
        << " | DPS: " << remaining_dps << endl;

    summary_file << "\nRemaining Players in Queues:\n";
    summary_file << "Tanks: " << remaining_tanks << " | Healers: " << remaining_healers
                << " | DPS: " << remaining_dps << endl;

    summary_file.close();

    return 0;
}