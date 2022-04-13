#include <iostream>
#include <fstream>

#define BLOCK 32768
#define OUT_SIZE 65536


using namespace std;

/**
 *
 * @param value
 * @param bit_position
 * @return code
 * Lit Value    Bits        Codes
   ---------    ----        -----
     0 - 143     8          00110000 through 10111111
   144 - 255     9          110010000 through 111111111
   256 - 279     7          0000000 through 0010111
   280 - 287     8          11000000 through 11000111
 */
uint16_t get_lit_code(int value, int *bit_width) { // we guarantee that 0 <= value <= 255
    uint16_t code;
    if (value <= 143) {
        code = 0b00110000;
        for (int i = 0; i < value; ++i) {
            code += 0b00000001;
        }
        *bit_width = 8;
    }
    else {
        code = 0b110010000;
        for (int i = 144; i < value; ++i) {
            code += 0b000000001;
        }
        *bit_width = 9;
    }
    return code;
}

/**
 * @param length
 * @return value
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
int length_value(int length, uint8_t *extra_bit) { // we guarantee here that the 3 <= length <= 258
    if (length == 3) return 257;
    else if (length == 4) return 258;
    else if (length == 5) return 259;
    else if (length == 6) return 260;
    else if (length == 7) return 261;
    else if (length == 8) return 262;
    else if (length == 9) return 263;
    else if (length == 10) return 264;
    else if (length == 11 || length == 12) {
        *extra_bit = length-11;
        return 265;
    }
    else if (length == 13 || length == 14) {
        *extra_bit = length-13;
        return 266;
    }
    else if (length == 15 || length == 16) {
        *extra_bit = length-15;
        return 267;
    }
    else if (length == 17 || length == 18) {
        *extra_bit = length-17;
        return 268;
    }
    else if (length <= 22) {
        *extra_bit = length-19;
        return 269;
    }
    else if (length <= 26) {
        *extra_bit = length-23;
        return 270;
    }
    else if (length <= 30) {
        *extra_bit = length-27;
        return 271;
    }
    else if (length <= 34) {
        *extra_bit = length-31;
        return 272;
    }
    else if (length <= 42) {
        *extra_bit = length-35;
        return 273;
    }
    else if (length <= 50) {
        *extra_bit = length-43;
        return 274;
    }
    else if (length <= 58) {
        *extra_bit = length-51;
        return 275;
    }
    else if (length <= 66) {
        *extra_bit = length-59;
        return 276;
    }
    else if (length <= 82) {
        *extra_bit = length-67;
        return 277;
    }
    else if (length <= 98) {
        *extra_bit = length-83;
        return 278;
    }
    else if (length <= 114) {
        *extra_bit = length-99;
        return 279;
    }
    else if (length <= 130) {
        *extra_bit = length-115;
        return 280;
    }
    else if (length <= 162) {
        *extra_bit = length-131;
        return 281;
    }
    else if (length <= 194) {
        *extra_bit = length-163;
        return 282;
    }
    else if (length <= 226) {
        *extra_bit = length-195;
        return 283;
    }
    else if (length <= 257) {
        *extra_bit = length-227;
        return 284;
    }
    else return 285; // length == 258
}

/**
 *
 * @param length
 * @param bit_width
 * @return code of the length
 */
uint16_t get_len_code(int length, int *bit_width) { // we guarantee that 256 <= value <= 285
    uint8_t extra_bit;
    int value = length_value(length, &extra_bit);
    uint16_t code;
    if (value <= 279) {
        code = 0b0000000;
        for (int i = 256; i < value; ++i) {
            code += 0b0000001;
        }
        *bit_width = 7;
        if (value >= 265 && value <= 268) {
            code = (code << 1) | extra_bit;
            (*bit_width)++;
        }
        else if (value >= 269 && value <= 272) {
            code = (code << 2) | extra_bit;
            (*bit_width) += 2;
        }
        else if (value >= 273 && value <= 276) {
            code = (code << 3) | extra_bit;
            (*bit_width) += 3;
        }
        else if (value >= 277 && value <= 279) {
            code = (code << 4) | extra_bit;
            (*bit_width) += 4;
        }
    }
    else {
        code = 0b11000000;
        for (int i = 280; i < value; ++i) {
            code += 0b00000001;
        }
        *bit_width = 8;
        if (value == 280) {
            code = (code << 4) | extra_bit;
            (*bit_width) += 4;
        }
        else if (value >= 281 && value <= 284) {
            code = (code << 5) | extra_bit;
            (*bit_width) += 5;
        }
    }
    return code;
}

/**
 * @param offset
 * @param bit_width of the extra_bit
 * @param extra_bit
 * @return code of the distance/offset's prefix
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
uint8_t get_dis_code(int offset, int *bit_width, uint16_t *extra_bit) { // distance 1-32768
    uint8_t pre_code;
    if (offset <= 4) {
        *bit_width = 0;
        *extra_bit = 0x00;
        pre_code = offset - 1;
    }
    else if (offset == 5 || offset == 6) {
        *bit_width = 1;
        *extra_bit = offset - 5;
        pre_code = 4;
    }
    else if (offset == 7 || offset == 8) {
        *bit_width = 1;
        *extra_bit = offset - 7;
        pre_code = 5;
    }
    else if (offset <= 12) {
        *bit_width = 2;
        *extra_bit = offset - 9;
        pre_code = 6;
    }
    else if (offset <= 16) {
        *bit_width = 2;
        *extra_bit = offset - 13;
        pre_code = 7;
    }
    else if (offset <= 24) {
        *bit_width = 3;
        *extra_bit = offset - 17;
        pre_code = 8;
    }
    else if (offset <= 32) {
        *bit_width = 3;
        *extra_bit = offset - 25;
        pre_code = 9;
    }
    else if (offset <= 48) {
        *bit_width = 4;
        *extra_bit = offset - 33;
        pre_code = 10;
    }
    else if (offset <= 64) {
        *bit_width = 4;
        *extra_bit = offset - 49;
        pre_code = 11;
    }
    else if (offset <= 96) {
        *bit_width = 5;
        *extra_bit = offset - 65;
        pre_code = 12;
    }
    else if (offset <= 128) {
        *bit_width = 5;
        *extra_bit = offset - 97;
        pre_code = 13;
    }
    else if (offset <= 192) {
        *bit_width = 6;
        *extra_bit = offset - 129;
        pre_code = 14;
    }
    else if (offset <= 256) {
        *bit_width = 6;
        *extra_bit = offset - 193;
        pre_code = 15;
    }
    else if (offset <= 384) {
        *bit_width = 7;
        *extra_bit = offset - 257;
        pre_code = 16;
    }
    else if (offset <= 512) {
        *bit_width = 7;
        *extra_bit = offset - 385;
        pre_code = 17;
    }
    else if (offset <= 768) {
        *bit_width = 8;
        *extra_bit = offset - 513;
        pre_code = 18;
    }
    else if (offset <= 1024) {
        *bit_width = 8;
        *extra_bit = offset - 769;
        pre_code = 19;
    }
    else if (offset <= 1536) {
        *bit_width = 9;
        *extra_bit = offset - 1025;
        pre_code = 20;
    }
    else if (offset <= 2048) {
        *bit_width = 9;
        *extra_bit = offset - 1537;
        pre_code = 21;
    }
    else if (offset <= 3072) {
        *bit_width = 10;
        *extra_bit = offset - 2049;
        pre_code = 22;
    }
    else if (offset <= 4096) {
        *bit_width = 10;
        *extra_bit = offset - 3073;
        pre_code = 23;
    }
    else if (offset <= 6144) {
        *bit_width = 11;
        *extra_bit = offset - 4097;
        pre_code = 24;
    }
    else if (offset <= 8192) {
        *bit_width = 11;
        *extra_bit = offset - 6145;
        pre_code = 25;
    }
    else if (offset <= 12288) {
        *bit_width = 12;
        *extra_bit = offset - 8193;
        pre_code = 26;
    }
    else if (offset <= 16384) {
        *bit_width = 12;
        *extra_bit = offset - 12289;
        pre_code = 27;
    }
    else if (offset <= 24576) {
        *bit_width = 13;
        *extra_bit = offset - 16385;
        pre_code = 28;
    }
    else {
        *bit_width = 13;
        *extra_bit = offset - 24577;
        pre_code = 29;
    }
    return pre_code;
}

/**
 *
 * @param content
 * @param window_start
 * @param buffer_start its distance to window_start is at most a WINDOW size
 * @param real_len
 * @param offset
 * @param literal the current char if not matches
 * @return the longest matching length
 */
int find_longest(const uint8_t *content, int *buffer_start, int real_len, int *offset, int *literal) {
    // if there is no matching,
    // the offset is 0, the literal is the current char and the longest_match is 0
    *offset = 0;
    *literal = content[*buffer_start];
    int longest_match = 0;

    int cur_match = 0;
    int w_pointer; // the position relative to window_start
    int b_pointer; // the position relative to buffer_start
    for (int i = 0; i < *buffer_start; ++i) {
        w_pointer = i;
        b_pointer = 0;
        cur_match = 0;
        while (w_pointer < *buffer_start && *buffer_start+b_pointer < real_len) {
            if (content[w_pointer] != content[*buffer_start+b_pointer]) break;
            w_pointer++;
            b_pointer++;
            cur_match++;
        }

        if (longest_match <= cur_match) {
            longest_match = cur_match;
            if (cur_match >= 3) {
                *offset = *buffer_start - i;
                *literal = -1; // this literal is invalid now, will not use in the future
            }
        }
    }
    return longest_match;
}

/**
 * @param content
 * @param window_start
 * @param buffer_start
 * @param real_len
 * @param out_code
 * @param cur_arr_pos
 * @param cur_bit_pos
 */
void encode(const uint8_t *content, int *buffer_start, int real_len,
                char *out_code, int *cur_arr_pos, int *cur_bit_pos) {
    int offset;
    int literal;
    int match_length = find_longest(content, buffer_start, real_len, &offset, &literal);
    if (match_length < 3) { // there is no matching
        int loop_times;
        if (match_length == 0) {
            loop_times = 1;
            (*buffer_start)++;
        }
        else loop_times = match_length;
        for (int i = 0; i < loop_times; ++i) {
            int bit_width;
            uint16_t lit_code = get_lit_code(literal, &bit_width);
            if (bit_width <= 8 - *cur_bit_pos) {
                out_code[*cur_arr_pos] |= (uint8_t) (lit_code << (8-*cur_bit_pos-bit_width));
                if (bit_width == 8 - *cur_bit_pos) {
                    (*cur_arr_pos)++;
                    *cur_bit_pos = 0;
                }
                else *cur_bit_pos += bit_width;
            }
            else {
                out_code[*cur_arr_pos] |= (uint8_t) (lit_code >> (bit_width-8+*cur_bit_pos));
                (*cur_arr_pos)++;
                out_code[*cur_arr_pos] = (uint8_t) (lit_code << (16-bit_width-*cur_bit_pos));
                *cur_bit_pos = bit_width-8+*cur_bit_pos;
            }
        }
    }
    else { // there is a matching (length, distance)
        int bit_width;
        uint16_t len_code = get_len_code(match_length, &bit_width);
        if (bit_width <= 8 - *cur_bit_pos) {
            out_code[*cur_arr_pos] |= (uint8_t) (len_code << (8-*cur_bit_pos-bit_width));
            if (bit_width == 8 - *cur_bit_pos) {
                (*cur_arr_pos)++;
                *cur_bit_pos = 0;
            }
            else *cur_bit_pos += bit_width;
        }
        else {
            out_code[*cur_arr_pos] |= (uint8_t) (len_code >> (bit_width-8+*cur_bit_pos));
            (*cur_arr_pos)++;
            out_code[*cur_arr_pos] = (uint8_t) (len_code << (16-bit_width-*cur_bit_pos));
            *cur_bit_pos = bit_width-8+*cur_bit_pos;
        }
        uint16_t extra_bit;
        // here the bit_width is for the extra_bit
        uint8_t dis_code = get_dis_code(offset, &bit_width, &extra_bit);
        if (3 - *cur_bit_pos >= 0) {
            out_code[*cur_arr_pos] |= (dis_code << (3-*cur_bit_pos));
            if (3 - *cur_bit_pos == 0) {
                (*cur_arr_pos)++;
                *cur_bit_pos = 0;
            }
            else *cur_bit_pos += 5;
        }
        else {
            out_code[*cur_arr_pos] |= (dis_code >> (*cur_bit_pos-3));
            (*cur_arr_pos)++;
            out_code[*cur_arr_pos] = (dis_code << (11-*cur_bit_pos));
            *cur_bit_pos = *cur_bit_pos-3;
        }

        if (bit_width <= 8 - *cur_bit_pos) {
            out_code[*cur_arr_pos] |= (uint8_t) (extra_bit << (8-*cur_bit_pos-bit_width));
            if (bit_width == 8 - *cur_bit_pos) {
                (*cur_arr_pos)++;
                *cur_bit_pos = 0;
            }
            else *cur_bit_pos += bit_width;
        }
        else {
            out_code[*cur_arr_pos] |= (uint8_t) (extra_bit >> (bit_width-8+*cur_bit_pos));
            (*cur_arr_pos)++;
            out_code[*cur_arr_pos] = (uint8_t) (extra_bit << (16-bit_width-*cur_bit_pos));
            *cur_bit_pos = bit_width-8+*cur_bit_pos;
        }
    }
    *buffer_start += match_length;
}

uint8_t reverse_table(uint8_t x) {
    static const uint8_t table[] = // the table is from https://blog.csdn.net/wanruiou/article/details/104049881
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
    return table[x];
}

uint32_t lz77(ifstream &in, char *out_code, int &cur_arr_pos) {
    in.seekg(0, ios::beg);
    uint32_t compressed_size = 0;
    int real_len;
    int cur_bit_pos = 3; // from right to left
    out_code[0] = 0b01000000;
    do {
        char *block = new char[BLOCK];
        if (in) {
            in.read(block, BLOCK);
        }
        uint8_t *content = (uint8_t *) block;
        real_len = in.gcount();
        int buffer_start = 0;
        while (buffer_start < real_len) {
            // encode the letter
            encode(content, &buffer_start, real_len,
                   out_code, &cur_arr_pos, &cur_bit_pos);
            /*if (cur_arr_pos >= OUT_SIZE-5) {
                for (int i = 0; i < cur_arr_pos; ++i) {
                    char temp[1] = {(char) reverse_table(out_code[i])};
                    out.write(temp, 1);
                }
                compressed_size += cur_arr_pos;
                if (cur_bit_pos < 5) {
                    out_code[0] |= (out_code[cur_arr_pos] >> 3);
                    if (cur_bit_pos == 4) {
                        cur_arr_pos = 1;
                        cur_bit_pos = 0;
                    }
                    else {
                        cur_arr_pos = 0;
                        cur_bit_pos += 3;
                    }
                }
                else {
                    out_code[0] |= (out_code[cur_arr_pos] >> 3);
                    out_code[1] |= (out_code[cur_arr_pos] << 5);
                    cur_arr_pos = 1;
                    cur_bit_pos = cur_bit_pos-5;
                }
            }*/
        }
        delete [] block;
    } while (real_len == BLOCK);
    out_code[0] |= 0b11000000;
    /*for (int i = 0; i < cur_arr_pos+1; ++i) {
        char temp[1] = {(char) reverse_table(out_code[i])};
        out.write(temp, 1);
    }*/
    if (cur_bit_pos == 0) cur_arr_pos--;
    compressed_size += cur_arr_pos + 1;
    return compressed_size;
}

