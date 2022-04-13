#pragma once
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <tuple>

#define MAX_DISTANCE 32768
#define MAX_LEN 258
#define BLOCK 327680

typedef std::array<uint8_t, 4> array4;

struct arr_hash {
    std::size_t operator() (const array4 &a) const noexcept {
        return a[0] * 1ULL + a[1] * 0x100ULL + a[2] * 0x10000ULL + a[3] * 0x1000000ULL;
    }
};

struct arr_equal {
    bool operator() (const array4 &a, const array4 &b) const noexcept {
        if (a[0] != b[0]) return false;
        if (a[1] != b[1]) return false;
        if (a[2] != b[2]) return false;
        if (a[3] != b[3]) return false;
        return true;
    }
};

static const uint8_t reverse_table[] = // the table is from https://blog.csdn.net/wanruiou/article/details/104049881
        {
                0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
                0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
                0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
                0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
                0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
                0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
                0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
                0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
                0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
                0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
                0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
                0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
                0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
                0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
                0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
                0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
                0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
                0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
                0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
                0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
                0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
                0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
                0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
                0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
                0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
                0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
                0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
                0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
                0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
                0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
                0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
                0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
        };

/**
 * @param value we guarantee that 0 <= value <= 255
 * @return <lit_code, lit_code_bit_width>
 * Lit Value    Bits        Codes
   ---------    ----        -----
     0 - 143     8          00110000 through 10111111
   144 - 255     9          110010000 through 111111111
   256 - 279     7          0000000 through 0010111
   280 - 287     8          11000000 through 11000111
 */
std::tuple<uint16_t, int> get_lit_code(int value);

/**
 * @param length we guarantee here that the 3 <= length <= 258
 * @return <code, extra_bit, extra_bit_width>
Code Bits Length(s) Code Bits Lengths   Code Bits Length(s)
---- ---- ------     ---- ---- -------   ---- ---- -------
 257   0     3       267   1   15,16     277   4   67-82
 258   0     4       268   1   17,18     278   4   83-98
 259   0     5       269   2   19-22     279   4   99-114
 260   0     6       270   2   23-26     280   4  115-130
 261   0     7       271   2   27-30     281   5  131-162
 262   0     8       272   2   31-34     282   5  163-194
 263   0     9       273   3   35-42     283   5  195-226
 264   0    10       274   3   43-50     284   5  227-257
 265   1  11,12      275   3   51-58     285   0    258
 266   1  13,14      276   3   59-66
 */
std::tuple<int, uint16_t, int> length_value(int length);

/**
 * @param length
 * @return <len_code, len_code_bit_width>
 */
std::tuple<uint16_t, int> get_len_code(int length);

/**
 * @param extra_bit
 * @param bit_width
 * @return the extra_bit after reversing it
 */
uint16_t reverse_extra_bit(uint16_t extra_bit, int bit_width);

/**
 * @param offset 1-32768
 * @return <pre_dis_code, extra_bit, bit_width>
 Code Bits Dist  Code Bits   Dist     Code Bits Distance
 ---- ---- ----  ---- ----  ------    ---- ---- --------
   0   0    1     10   4     33-48    20    9   1025-1536
   1   0    2     11   4     49-64    21    9   1537-2048
   2   0    3     12   5     65-96    22   10   2049-3072
   3   0    4     13   5     97-128   23   10   3073-4096
   4   1   5,6    14   6    129-192   24   11   4097-6144
   5   1   7,8    15   6    193-256   25   11   6145-8192
   6   2   9-12   16   7    257-384   26   12  8193-12288
   7   2  13-16   17   7    385-512   27   12 12289-16384
   8   3  17-24   18   8    513-768   28   13 16385-24576
   9   3  25-32   19   8   769-1024   29   13 24577-32768
 */
std::tuple<uint8_t, uint16_t, int> get_dis_code(int offset);

/**
 * @param out_code the output buffer array
 * @param cur_arr_pos
 * @param cur_bit_pos
 * @param to_write
 * @param bit_width
 */
void write_code(uint8_t *out_code, int &cur_arr_pos, int &cur_bit_pos, uint16_t to_write, int bit_width);

/**
 * This method just for 5 bits
 * @param out_code
 * @param cur_arr_pos
 * @param cur_bit_pos
 * @param to_write
 */
void write_code(uint8_t *out_code, int &cur_arr_pos, int &cur_bit_pos, uint8_t to_write);

/**
 *
 * @param content
 * @param real_len
 * @param buffer_start not update the buffer_start in this method
 * @param match_index the index of the matched string
 * @return match_len
 * we guarantee buffer_start+3 will not out of bound
 */
std::tuple<int, int> match(const uint8_t *content, uint64_t real_len, int buffer_start, int *index);

/**
 * @param in input file
 * @param out output file
 * @return compressed size
 */
uint32_t lz77(std::ifstream &in, std::ofstream &out);