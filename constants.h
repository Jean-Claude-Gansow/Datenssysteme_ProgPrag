#ifndef CONSTANTS_H
#define CONSTANTS_H

// Breakout-Labels für goto-Anweisungen

static const unsigned char token_lut[128] = {
    // 0-47 (20er zeilen)
    0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, ' ', 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, ',', 32,
    // 48-57 '0'-'9'
    87, 88, 89, 90, 91, 92, 93, 94, 95, 96, // auf Werte nach kleinbuchstaben mappen
    // 58-64
    32, 32, 32, 32, 32, 32, 32,
    // 65-90 'A'-'Z' to 'a'-'z'
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', '0', '1', '2', '3',
    // 91-96
    '4', '5', '6', '7', '8', '9', //for transforming the mapped numbers back to numbers in case of a string comparisson
    // 97-122 'a'-'z'
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    // 123-127
    32, 32, 32, 32, 32};

static const unsigned char lut[256] =
    { // 0-47 (20er zeilen)
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, ' ',
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, ',', 32,
        // 48-57 '0'-'9'
        87, 88, 89, 90, 91, 92, 93, 94, 95, 96, //'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // auf Werte nach kleinbuchstaben mappen
        //58-64
        32, 32, 32, 32, 32, 32, 32,
        // 65-90 'A'-'Z' to 'a'-'z'
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        // 91-96
        32, 32, 32, 32, 32, 32,
        // 97-122 'a'-'z'
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        // 123-255
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
        32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
        32};
#endif

// copilot generated comment:
// This lookup table can be used to convert characters '0'-'9' and 'A'-'Z' to lowercase 'a'-'z'
// and to ignore other characters by mapping them to whitespace. This is useful for data cleaning or normalization tasks.
// The table is indexed by the ASCII value of the character, and it provides a quick way to convert
// characters without needing to use conditionals or loops. For example, to convert a character `c`,
// you can simply use `lut[(unsigned char)c]`, which will return the corresponding lowercase character
// or 0 if the character is not in the specified range. This approach is efficient and avoids branching,
// making it suitable for performance-critical applications where character normalization is required.
// The table is defined as a static constant array, which means it is stored in read-only memory
// and can be accessed quickly without the overhead of function calls or additional memory allocations.
// The use of the range-based initialization syntax (`[0 ... 47] = 0`) allows for concise initialization
// of multiple elements in the array, making the code cleaner and easier to maintain.
