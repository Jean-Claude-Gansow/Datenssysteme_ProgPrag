#include <vector>
#include <thread>
#include <cmath>
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
inline std::vector<size_t> calc_thread_line_offsets(const char *buffer, size_t buffer_size, size_t num_threads, size_t start_offset = 0)
{
    // 1. Zeilen zählen
    size_t total_lines = 0;
    for (size_t i = start_offset; i < buffer_size; ++i)
        if (buffer[i] == '\n')
            ++total_lines;

    size_t lines_per_thread = total_lines / num_threads;
    std::vector<size_t> offsets;
    offsets.push_back(start_offset);

    size_t curr_offset = start_offset;
    size_t curr_line = 0;

    for (size_t t = 1; t < num_threads; ++t)
    {
        size_t lines_to_skip = lines_per_thread;
        while (curr_offset < buffer_size && lines_to_skip > 0)
        {
            if (buffer[curr_offset] == '\n')
                --lines_to_skip;
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

// Templated Worker-Funktion für einen Bereich im Puffer
template <typename T>
inline void process_lines_worker(
    const char *buffer, size_t start, size_t end,
    std::function<int(const char *, void *)> parse_line,
    T *output_buffer, const char *format, size_t start_index = 0)
{
    size_t line_start = start;
    size_t out_idx = start_index;

    while (line_start < end)
    {
        // Zeiger auf Output-Struct berechnen
        T *out_ptr = output_buffer + out_idx;

        int read = parse_line(&buffer[line_start], static_cast<void *>(out_ptr));
        line_start += read;

        // Trennzeichen überspringen, falls vorhanden
        if (buffer[line_start] == '\0' || buffer[line_start] == '\n' || buffer[line_start] == '\r')
            ++line_start;

        // print_row(*out_ptr, format);
        ++out_idx;
    }
}

template <typename T>
inline void threaded_line_split(const char* file_content, const char* format,  size_t content_size, size_t num_threads, size_t start, size_t total_lines, std::function<int(const char*, void*)> parse_line, T** thread_buffers,size_t* thread_counts) 
{    

    printf("Starte line_split...\n");

    printf("Dateigröße: %zu, Start: %zu, Gesamtzeilen: %zu\n", content_size, start, total_lines);

    
    if (num_threads <= 1) 
    {

        printf("Nur ein Thread, starte Single-Thread-Verarbeitung...\n");
        
        thread_buffers[0] = new T[total_lines]; // Reserve space for the first thread

        // Single-thread fallback
        thread_counts[0] = 0;
        size_t line_start = start;
        while (line_start < content_size) 
        {
            T* out_ptr = thread_buffers[0] + thread_counts[0];
            int read = parse_line(&file_content[line_start], static_cast<void*>(out_ptr));

            if (read <= 0 || (line_start + read) > content_size) break;

            line_start += read;
            if (line_start < content_size && (file_content[line_start] == '\0' || file_content[line_start] == '\n' || file_content[line_start] == '\r'))
                ++line_start;

            ++thread_counts[0];
            //printf("Thread 0: Verarbeitet Zeichen %zu, Output-Index %zu\n", line_start, thread_counts[0]);
        }
        return;
    }

    printf("Anzahl Threads: %zu, Start: %zu, Gesamtzeilen: %zu\n", num_threads, start, total_lines);

    size_t* raw_offsets = new size_t[num_threads +1];
    size_t* real_offsets = new size_t[num_threads + 1];

    // 1. Bereiche grob aufteilen
    size_t per_thread_bytes = content_size / num_threads;
    for (size_t t = 0; t < num_threads; ++t)
    {
        raw_offsets[t] = start + t * per_thread_bytes;
        //printf("Thread %zu: [%zu ~ %zu]\n", t, raw_offsets[t],raw_offsets[t] + per_thread_bytes);
    }

    // 2. An Zeilenanfänge anpassen
    
    printf("Anpassen der Zeilenanfänge...\n");

    real_offsets[0] = start;
    for (size_t t = 0; t < num_threads; ++t) 
    {
        size_t pos = raw_offsets[t];
        if (pos == 0) pos = 1;
        while (pos < content_size && file_content[pos - 1] != '\n')
        {
            ++pos;
        }
        real_offsets[t] = pos;
    }
    real_offsets[num_threads] = content_size; //dont allow reads over the end of the file
    delete[] raw_offsets; // no longer needed

    // 3. Buffer reservieren (doppelte Größe)
    size_t expected_lines = total_lines / num_threads;
    size_t buffered_lines = expected_lines + (size_t)((expected_lines / 10)+2); // bei kleinen Dateien fällt der Puffer konstant zu klein aus, daher +2
    for (size_t t = 0; t < num_threads; ++t) {
        //printf("Thread %zu: Reserviere Puffer für %zu Zeilen\n", t, buffered_lines);
        thread_buffers[t] = new T[buffered_lines]; //allocated buffer per thread
        thread_counts[t] = 0;
        printf("Thread %zu: -> [%zu - %zu]\n", t, real_offsets[t], real_offsets[t+1]);
    }

    // 4. Threads starten
    std::thread* threads = new std::thread[num_threads];
    for (size_t t = 0; t < num_threads; ++t) 
    {
        size_t block_start = real_offsets[t];
        size_t block_end = real_offsets[t + 1];
        size_t* count_ptr = &thread_counts[t];
        T* buffer_ptr = thread_buffers[t];
        size_t buffer_size = block_end - block_start;


        threads[t] = std::thread([=, &parse_line]()
        {
            size_t line_start = block_start;
            size_t out_idx = 0;

            while (line_start < block_end) 
            {
                T* out_ptr = buffer_ptr + out_idx;
                int read = parse_line(&file_content[line_start], static_cast<void*>(out_ptr));

                if (read <= 0 || (line_start + read) > block_end) break;

                line_start += read;
                if (line_start < block_end && (file_content[line_start] == '\0' || file_content[line_start] == '\n' || file_content[line_start] == '\r')) // skip line endings
                    ++line_start;

                ++out_idx; //after each line processed go to next writing position
                //printf("Thread %ld: %zu -> buf [%zu]\n", t,line_start, thread_counts[t]++);
            }

            *count_ptr = out_idx;
        });
    }

    // 5. Warten auf alle Threads
    for (size_t t = 0; t < num_threads; ++t)
        threads[t].join();

    delete[] real_offsets;
    delete[] threads;
}
