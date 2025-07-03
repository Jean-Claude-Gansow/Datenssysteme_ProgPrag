#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <stdio.h>
#include <string>
#include <atomic>

// Debug levels
// 0 - No debugging
// 1 - Basic debugging (essential information)
// 2 - Detailed debugging (more verbose output)
// 3 - Full debugging (all possible output)

// Performance control
// Define this to limit debug output in critical sections (like inner loops)
#define LIMIT_DEBUG_OUTPUT

// When defined, debug messages will be rate-limited in critical code sections
#ifdef LIMIT_DEBUG_OUTPUT
  // Only print every Nth message in critical sections
  #define DEBUG_RATE_LIMIT 100000
  
  // Global counters für rate-limited debugging
  static std::atomic<unsigned long> g_debug_counter{0};
  static std::atomic<unsigned long> g_match_counter{0};
  static std::atomic<unsigned long> g_token_counter{0};
  static std::atomic<unsigned long> g_part_counter{0};
  
  // Makro für regelmäßige Fortschrittsaktualisierung
  #define SHOW_PROGRESS_EVERY 5000000
  #define DEBUG_SHOW_PROGRESS() do { \
    unsigned long counter = ++g_debug_counter; \
    if (counter % SHOW_PROGRESS_EVERY == 0) { \
      printf("\n[PROGRESS] Operations: %lu million, Matching: %lu, Tokenizing: %lu, Partitioning: %lu\n", \
        counter/1000000, g_match_counter.load(), g_token_counter.load(), g_part_counter.load()); \
    } \
  } while(0)
#endif

// Uncomment one of these to enable debugging at different levels
// #define DEBUG_LEVEL 1
// #define DEBUG_LEVEL 2
// #define DEBUG_LEVEL 3

// Feature-specific debug flags
#ifdef DEBUG_LEVEL
  #if DEBUG_LEVEL >= 1
    #define DEBUG_MATCHING
    #define DEBUG_PARTITIONING
  #endif
  
  #if DEBUG_LEVEL >= 2
    #define DEBUG_TOKENIZATION
    #define DEBUG_MATCHING_DETAILED
  #endif
  
  #if DEBUG_LEVEL >= 3
    #define DEBUG_MEMORY
    #define DEBUG_MATCHING_FULL
  #endif
#endif

// Debug macros
#ifdef DEBUG_MATCHING
  #ifdef LIMIT_DEBUG_OUTPUT
    #define DEBUG_MATCH(fmt, ...) do { \
      unsigned long counter = ++g_match_counter; \
      if ((counter % DEBUG_RATE_LIMIT) == 0) { \
        printf("[MATCH %lu] " fmt, counter, ##__VA_ARGS__); \
      } \
    } while(0)
    #define DEBUG_MATCH_IMPORTANT(fmt, ...) do { \
      unsigned long counter = g_match_counter.load(); \
      printf("[MATCH-IMP %lu] " fmt, counter, ##__VA_ARGS__); \
    } while(0)
  #else
    #define DEBUG_MATCH(fmt, ...) printf("[MATCH] " fmt, ##__VA_ARGS__)
    #define DEBUG_MATCH_IMPORTANT(fmt, ...) printf("[MATCH-IMP] " fmt, ##__VA_ARGS__)
  #endif
#else
  #define DEBUG_MATCH(fmt, ...)
  #define DEBUG_MATCH_IMPORTANT(fmt, ...)
#endif

#ifdef DEBUG_MATCHING_DETAILED
  #ifdef LIMIT_DEBUG_OUTPUT
    #define DEBUG_MATCH_DETAILED(fmt, ...) do { \
      unsigned long counter = g_match_counter.load(); \
      if ((counter % (DEBUG_RATE_LIMIT * 10)) == 0) { \
        printf("[MATCH-DETAIL %lu] " fmt, counter, ##__VA_ARGS__); \
      } \
    } while(0)
  #else
    #define DEBUG_MATCH_DETAILED(fmt, ...) printf("[MATCH-DETAIL] " fmt, ##__VA_ARGS__)
  #endif
#else
  #define DEBUG_MATCH_DETAILED(fmt, ...)
#endif

#ifdef DEBUG_MATCHING_FULL
  #ifdef LIMIT_DEBUG_OUTPUT
    #define DEBUG_MATCH_FULL(fmt, ...) do { \
      unsigned long counter = g_match_counter.load(); \
      if ((counter % (DEBUG_RATE_LIMIT * 100)) == 0) { \
        printf("[MATCH-FULL %lu] " fmt, counter, ##__VA_ARGS__); \
      } \
    } while(0)
  #else
    #define DEBUG_MATCH_FULL(fmt, ...) printf("[MATCH-FULL] " fmt, ##__VA_ARGS__)
  #endif
#else
  #define DEBUG_MATCH_FULL(fmt, ...)
#endif

#ifdef DEBUG_PARTITIONING
  #ifdef LIMIT_DEBUG_OUTPUT
    #define DEBUG_PART(fmt, ...) do { \
      unsigned long counter = ++g_part_counter; \
      if ((counter % DEBUG_RATE_LIMIT) == 0) { \
        printf("[PART %lu] " fmt, counter, ##__VA_ARGS__); \
      } \
    } while(0)
    #define DEBUG_PART_IMPORTANT(fmt, ...) do { \
      unsigned long counter = g_part_counter.load(); \
      printf("[PART-IMP %lu] " fmt, counter, ##__VA_ARGS__); \
    } while(0)
  #else
    #define DEBUG_PART(fmt, ...) printf("[PART] " fmt, ##__VA_ARGS__)
    #define DEBUG_PART_IMPORTANT(fmt, ...) printf("[PART-IMP] " fmt, ##__VA_ARGS__)
  #endif
#else
  #define DEBUG_PART(fmt, ...)
  #define DEBUG_PART_IMPORTANT(fmt, ...)
#endif

#ifdef DEBUG_TOKENIZATION
  #ifdef LIMIT_DEBUG_OUTPUT
    #define DEBUG_TOKEN(fmt, ...) do { \
      unsigned long counter = ++g_token_counter; \
      if ((counter % DEBUG_RATE_LIMIT) == 0) { \
        printf("[TOKEN %lu] " fmt, counter, ##__VA_ARGS__); \
      } \
    } while(0)
    #define DEBUG_TOKEN_IMPORTANT(fmt, ...) do { \
      unsigned long counter = g_token_counter.load(); \
      printf("[TOKEN-IMP %lu] " fmt, counter, ##__VA_ARGS__); \
    } while(0)
  #else
    #define DEBUG_TOKEN(fmt, ...) printf("[TOKEN] " fmt, ##__VA_ARGS__)
    #define DEBUG_TOKEN_IMPORTANT(fmt, ...) printf("[TOKEN-IMP] " fmt, ##__VA_ARGS__)
  #endif
#else
  #define DEBUG_TOKEN(fmt, ...)
  #define DEBUG_TOKEN_IMPORTANT(fmt, ...)
#endif

#ifdef DEBUG_MEMORY
  #define DEBUG_MEM(fmt, ...) printf("[MEM] " fmt, ##__VA_ARGS__)
#else
  #define DEBUG_MEM(fmt, ...)
#endif

// Rate-limited debug macros
#ifdef LIMIT_DEBUG_OUTPUT
  #define DEBUG_MATCH_RATE_LIMITED(fmt, ...) \
    do { \
      if (++g_match_counter % DEBUG_RATE_LIMIT == 0) \
        printf("[MATCH-RATE] " fmt, ##__VA_ARGS__); \
    } while (0)

  #define DEBUG_TOKEN_RATE_LIMITED(fmt, ...) \
    do { \
      if (++g_token_counter % DEBUG_RATE_LIMIT == 0) \
        printf("[TOKEN-RATE] " fmt, ##__VA_ARGS__); \
    } while (0)

  #define DEBUG_PART_RATE_LIMITED(fmt, ...) \
    do { \
      if (++g_part_counter % DEBUG_RATE_LIMIT == 0) \
        printf("[PART-RATE] " fmt, ##__VA_ARGS__); \
    } while (0)
#else
  #define DEBUG_MATCH_RATE_LIMITED(fmt, ...)
  #define DEBUG_TOKEN_RATE_LIMITED(fmt, ...)
  #define DEBUG_PART_RATE_LIMITED(fmt, ...)
#endif

// Category names for better debug output
inline const char* category_name(int category) {
    const char* names[] = {
        "brand",
        "model",
        "rom",
        "ram",
        "cpu_brand",
        "cpu_fam",
        "cpu_series",
        "gpu_brand",
        "gpu_fam",
        "gpu_series",
        "display_resolution",
        "display_size"
    };
    
    if (category >= 0 && category < 12) {
        return names[category];
    }
    return "unknown";
}

// Pretty printing function for debug output
inline void print_separator(const char* title = nullptr) {
    printf("\n");
    for(int i = 0; i < 50; i++) printf("-");
    if(title) printf(" %s ", title);
    for(int i = 0; i < 50; i++) printf("-");
    printf("\n");
}

// Function to print debug configuration status
inline void print_debug_configuration() {
    print_separator("DEBUG CONFIGURATION");
    
    #ifdef DEBUG_LEVEL
        printf("Debug level: %d\n", DEBUG_LEVEL);
    #else
        printf("Debug level: OFF\n");
    #endif
    
    #ifdef LIMIT_DEBUG_OUTPUT
        printf("Debug rate limiting: ENABLED (1 in %d messages)\n", DEBUG_RATE_LIMIT);
    #else
        printf("Debug rate limiting: DISABLED\n");
    #endif
    
    printf("Debug categories:\n");
    
    #ifdef DEBUG_MATCHING
        printf("- Matching: ENABLED\n");
    #else
        printf("- Matching: DISABLED\n");
    #endif
    
    #ifdef DEBUG_MATCHING_DETAILED
        printf("- Detailed matching: ENABLED\n");
    #else
        printf("- Detailed matching: DISABLED\n");
    #endif
    
    #ifdef DEBUG_MATCHING_FULL
        printf("- Full matching: ENABLED\n");
    #else
        printf("- Full matching: DISABLED\n");
    #endif
    
    #ifdef DEBUG_PARTITIONING
        printf("- Partitioning: ENABLED\n");
    #else
        printf("- Partitioning: DISABLED\n");
    #endif
    
    #ifdef DEBUG_TOKENIZATION
        printf("- Tokenization: ENABLED\n");
    #else
        printf("- Tokenization: DISABLED\n");
    #endif
    
    #ifdef DEBUG_MEMORY
        printf("- Memory: ENABLED\n");
    #else
        printf("- Memory: DISABLED\n");
    #endif
    
    print_separator();
}

#endif // DEBUG_UTILS_H
