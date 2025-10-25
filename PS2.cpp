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

int main(){
    int initial_tanks = 7;
    int initial_healers = 7;
    int initial_dps = 21;
    dungeon_count = 3;

    count = vector<int>(3, 0);
    for(int i = 0; i < dungeon_count; i++){
        dungeon_free_list.push_back(i);
        dungeon_party_list.push_back("EMPTY");
    }
}