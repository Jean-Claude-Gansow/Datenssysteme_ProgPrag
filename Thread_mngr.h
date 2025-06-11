#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <string>
#include <cstdio>

class Thread_mngr {
public:
    // Konstruktor: Hardware-Infos sammeln
    Thread_mngr() {
        max_threads = std::thread::hardware_concurrency();
        max_threads = max_threads > 0 ? max_threads : 1;
        // Optional: weitere Infos sammeln
        // (z.B. CPU-Name, NUMA-Nodes, Cache-Größe, falls gewünscht)
    }

    // Getter für Hardware-Infos
    unsigned int get_max_threads() const { return max_threads; }
    unsigned int get_active_threads() const { return active_threads; }
    unsigned int get_step_size() const { return step_size; }

    // Setzen der aktiven Threads und Step Size
    void set_active_threads(unsigned int n) { active_threads = n <= max_threads ? n : max_threads; }
    void set_step_size(unsigned int s) { step_size = s; }

    // Führt eine Funktion mit den aktuellen Thread-Parametern aus
    void run_with_threads(std::function<void(unsigned int, unsigned int, unsigned int)> func) {
        func(active_threads, max_threads, step_size);
    }

    // Optional: Ausgabe der Hardware-Infos
    void print_info() const {printf("Thread-Manager: max_threads=%u, active_threads=%u, step_size=%u\n", max_threads, active_threads, step_size);}

private:
    unsigned int max_threads = 1;
    unsigned int active_threads = 1;
    unsigned int step_size = 1;
};