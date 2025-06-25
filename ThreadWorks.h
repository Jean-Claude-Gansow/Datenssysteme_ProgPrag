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

template <typename inBuf_t, typename outBuf_t>
inline size_t* calc_thread_line_offsets(const void* vbuffer,size_t length,size_t total_lines, size_t num_threads, size_t start_offset, outBuf_t** out, size_t outBufferPadding)
{
    inBuf_t* buffer = (inBuf_t*)vbuffer;
    size_t* raw_offsets = new size_t[num_threads +1];
    size_t* real_offsets = new size_t[num_threads + 1];

    // 1. Bereiche grob aufteilen
    size_t per_thread_bytes = length / num_threads;
    per_thread_bytes = length > 0 ? per_thread_bytes : total_lines/num_threads; //case its not a block buffer, but an organized one in which we do not need searching
    for (size_t t = 0; t < num_threads; ++t)
    {
        raw_offsets[t] = start_offset + t * per_thread_bytes;
    }

    real_offsets[0] = start_offset;
    for (size_t t = 0; t < num_threads; ++t) 
    {
        size_t pos = raw_offsets[t];
        if (pos == 0) pos = 1;
        while (pos < length && buffer[pos - 1] != '\n')
        {
            ++pos;
        }
        real_offsets[t] = pos;
    }
    real_offsets[num_threads] = length; //dont allow reads over the end of the file
    delete[] raw_offsets; // no longer needed

    // 3. Buffer reservieren (doppelte Größe)
    size_t expected_lines = total_lines / num_threads;
    size_t buffered_lines = expected_lines + (size_t)(((expected_lines / 100)+2)*outBufferPadding); // bei kleinen Dateien fällt der Puffer konstant zu klein aus, daher +2
    for (size_t t = 0; t < num_threads; ++t) {
        //printf("Thread %zu: Reserviere Puffer für %zu Zeilen\n", t, buffered_lines);
        out[t] = new outBuf_t[buffered_lines]; //allocated buffer per thread
        printf("Thread %zu: -> [%zu - %zu]\n", t, real_offsets[t], real_offsets[t+1]);
    }
    return real_offsets;
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

    size_t* real_offsets = calc_thread_line_offsets<char,T>((char*)file_content,content_size,total_lines,num_threads,start,thread_buffers,10);
    /*
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
    }*/

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

template <typename T>
inline void threaded_tokenization(T *buffer, size_t buffer_size, std::function<int(const char*, void*)> tokenize_entry, size_t num_threads,T **thread_buffers)
{
    size_t* offsets = calc_thread_line_offsets(buffer,0,buffer_size,num_threads,0,thread_buffers,0);
    // 4. Threads starten
    std::thread* threads = new std::thread[num_threads];
    for (size_t t = 0; t < num_threads; ++t) 
    {
        size_t block_start = offsets[t];
        size_t block_end = offsets[t + 1];
        T* buffer_ptr = thread_buffers[t];
        size_t buffer_size = block_end - block_start;


        threads[t] = std::thread([=, &tokenize_entry]()
        {
            size_t bufferline = block_start;
            size_t out_idx = 0;

            tokenize_entry()
        });
    }

    // 5. Warten auf alle Threads
    for (size_t t = 0; t < num_threads; ++t)
        threads[t].join();

    delete[] offsets;
    delete[] threads;

}
