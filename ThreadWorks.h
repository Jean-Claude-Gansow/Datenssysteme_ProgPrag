#include <vector>
#include <thread>
#include <cstddef>
#include <functional>
#include "FileInput.h"
#include "DataTypes.h"
#include "Parser_mngr.h"
#include "Utillity.h"

// Sucht Zeilenanfänge für Thread-Bereiche im Puffer
// buffer: Zeilenpuffer (z.B. mmap-File)
// buffer_size: Größe des Puffers
// num_threads: gewünschte Threadanzahl
// start_offset: Offset im Puffer, ab dem gesucht wird (meist 0)
// Gibt einen Vektor mit Startpositionen für jeden Thread zurück (Größe: num_threads+1)
inline std::vector<size_t> calc_thread_line_offsets(const char* buffer, size_t buffer_size, size_t num_threads, size_t start_offset = 0) {
    // 1. Zeilen zählen
    size_t total_lines = 0;
    for (size_t i = start_offset; i < buffer_size; ++i)
        if (buffer[i] == '\n') ++total_lines;

    size_t lines_per_thread = total_lines / num_threads;
    std::vector<size_t> offsets;
    offsets.push_back(start_offset);

    size_t curr_offset = start_offset;
    size_t curr_line = 0;

    for (size_t t = 1; t < num_threads; ++t) {
        size_t lines_to_skip = lines_per_thread;
        while (curr_offset < buffer_size && lines_to_skip > 0) {
            if (buffer[curr_offset] == '\n') --lines_to_skip;
            ++curr_offset;
        }
        // curr_offset zeigt jetzt auf das Zeichen nach dem n-ten Zeilenumbruch
        // Optional: auf nächsten echten Zeilenanfang springen (falls '\r\n')
        while (curr_offset < buffer_size && (buffer[curr_offset] == '\n' || buffer[curr_offset] == '\r'))
            ++curr_offset;
        offsets.push_back(curr_offset);
    }
    offsets.push_back(buffer_size); // Ende

    return offsets;
}

// Worker-Funktion: Bearbeitet einen Bereich (hier nur als Platzhalter)
inline void process_lines(const char* buffer, size_t start, size_t end, size_t thread_id) {
    // Hier kommt dein Parser-Code für den Bereich [start, end)
    // printf("Thread %zu: Bereich [%zu, %zu)\n", thread_id, start, end);
}

// Templated Worker-Funktion für einen Bereich im Puffer
template<typename T>
inline void process_lines_worker(
    const char* buffer, size_t start, size_t end,
    std::function<int(const char*, void*)> parse_line,
    T* output_buffer, const char* format, size_t start_index = 0)
{
    size_t line_start = start;
    size_t out_idx = start_index;

    while (line_start < end) {
        // Zeiger auf Output-Struct berechnen
        T* out_ptr = output_buffer + out_idx;

        int read = parse_line(&buffer[line_start], static_cast<void*>(out_ptr));
        line_start += read;

        // Trennzeichen überspringen, falls vorhanden
        if (buffer[line_start] == '\0' || buffer[line_start] == '\n' || buffer[line_start] == '\r')
            ++line_start;

        print_row(*out_ptr, format);
        ++out_idx;
    }
}

// Rekursive Thread-Aufteilung
template<typename T>
inline void threaded_line_split(
    const char* buffer, size_t buffer_size, size_t start_offset, size_t end_offset,
    size_t num_threads, size_t total_lines,
    std::function<int(const char*, void*)> parse_line,
    T* output_buffer, const char* format,
    size_t start_index = 0, size_t thread_id = 0)
{
    if (num_threads <= 1) {
        process_lines_worker<T>(buffer, start_offset, end_offset, parse_line, output_buffer, format, start_index);
        return;
    }

    size_t lines_in_block = 0;
    for (size_t i = start_offset; i < end_offset; ++i)
        if (buffer[i] == '\n') ++lines_in_block;

    size_t threads_left = num_threads;
    size_t curr_offset = start_offset;
    std::vector<std::thread> threads;
    size_t output_idx = start_index;

    for (size_t t = 0; t < num_threads; ++t) {
        size_t lines_for_this = lines_in_block / (threads_left--);
        if (t < lines_in_block % num_threads) ++lines_for_this;

        size_t block_start = curr_offset;
        size_t skipped = 0;
        while (curr_offset < end_offset && skipped < lines_for_this) {
            if (buffer[curr_offset] == '\n') ++skipped;
            ++curr_offset;
        }
        for (; curr_offset < end_offset && (buffer[curr_offset] == '\n' || buffer[curr_offset] == '\r'); ++curr_offset);

        if (t == num_threads - 1) {
            curr_offset = end_offset;
        }

        if (lines_for_this > 0) {
            threads.emplace_back(
                threaded_line_split<T>,
                buffer, buffer_size, block_start, curr_offset,
                1, lines_for_this, parse_line, output_buffer, format,
                output_idx, thread_id * 100 + t
            );
            output_idx += lines_for_this;
        }
    }

    for (auto& th : threads) th.join();
}