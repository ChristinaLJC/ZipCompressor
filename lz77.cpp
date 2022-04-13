#include "lz77.h"

using namespace std;

unordered_map<array4, int, arr_hash, arr_equal> dict;
//int index[BLOCK];

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
tuple<uint16_t, int> get_lit_code(int value) {
    uint16_t code;
    int bit_width = 0;
    if (value <= 143) {
        code = 0b00110000;
        for (int i = 0; i < value; ++i) {
            code += 0b00000001;
        }
        bit_width = 8;
    }
    else {
        code = 0b110010000;
        for (int i = 144; i < value; ++i) {
            code += 0b000000001;
        }
        bit_width = 9;
    }
    return {code, bit_width};
}

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
tuple<int, uint16_t, int> length_value(int length) {
    int value = 0;
    uint16_t extra_bit = 0;
    int bit_width = 0;
    if (length == 3) value = 257;
    else if (length == 4) value = 258;
    else if (length == 5) value = 259;
    else if (length == 6) value = 260;
    else if (length == 7) value = 261;
    else if (length == 8) value = 262;
    else if (length == 9) value = 263;
    else if (length == 10) value = 264;
    else if (length == 11 || length == 12) {
        extra_bit = length-11;
        bit_width = 1;
        value = 265;
    }
    else if (length == 13 || length == 14) {
        extra_bit = length-13;
        bit_width = 1;
        value = 266;
    }
    else if (length == 15 || length == 16) {
        extra_bit = length-15;
        bit_width = 1;
        value = 267;
    }
    else if (length == 17 || length == 18) {
        extra_bit = length-17;
        bit_width = 1;
        value = 268;
    }
    else if (length <= 22) {
        extra_bit = length-19;
        bit_width = 2;
        value = 269;
    }
    else if (length <= 26) {
        extra_bit = length-23;
        bit_width = 2;
        value = 270;
    }
    else if (length <= 30) {
        extra_bit = length-27;
        bit_width = 2;
        value = 271;
    }
    else if (length <= 34) {
        extra_bit = length-31;
        bit_width = 2;
        value = 272;
    }
    else if (length <= 42) {
        extra_bit = length-35;
        bit_width = 3;
        value = 273;
    }
    else if (length <= 50) {
        extra_bit = length-43;
        bit_width = 3;
        value = 274;
    }
    else if (length <= 58) {
        extra_bit = length-51;
        bit_width = 3;
        value = 275;
    }
    else if (length <= 66) {
        extra_bit = length-59;
        bit_width = 3;
        value = 276;
    }
    else if (length <= 82) {
        extra_bit = length-67;
        bit_width = 4;
        value = 277;
    }
    else if (length <= 98) {
        extra_bit = length-83;
        bit_width = 4;
        value = 278;
    }
    else if (length <= 114) {
        extra_bit = length-99;
        bit_width = 4;
        value = 279;
    }
    else if (length <= 130) {
        extra_bit = length-115;
        bit_width = 4;
        value = 280;
    }
    else if (length <= 162) {
        extra_bit = length-131;
        bit_width = 5;
        value = 281;
    }
    else if (length <= 194) {
        extra_bit = length-163;
        bit_width = 5;
        value = 282;
    }
    else if (length <= 226) {
        extra_bit = length-195;
        bit_width = 5;
        value = 283;
    }
    else if (length <= 257) {
        extra_bit = length-227;
        bit_width = 5;
        value = 284;
    }
    else value = 285; // length == 258
    return {value, extra_bit, bit_width};
}

/**
 * @param length
 * @return <len_code, len_code_bit_width>
 */
tuple<uint16_t, int> get_len_code(int length) {
    // we guarantee that 256 <= value <= 285
    auto [value, extra_bit, bit_width] = length_value(length);
    extra_bit = reverse_extra_bit(extra_bit, bit_width);
    uint16_t code;
    if (value <= 279) {
        code = 0b0000000;
        for (int i = 256; i < value; ++i) {
            code += 0b0000001;
        }
        bit_width += 7; // add the bit_width of pre_code

        // the following is binding the extra_bit with the code
        if (value >= 265 && value <= 268) {
            code = (code << 1) | extra_bit;
        }
        else if (value >= 269 && value <= 272) {
            code = (code << 2) | extra_bit;
        }
        else if (value >= 273 && value <= 276) {
            code = (code << 3) | extra_bit;
        }
        else if (value >= 277 && value <= 279) {
            code = (code << 4) | extra_bit;
        }
    }
    else {
        code = 0b11000000;
        for (int i = 280; i < value; ++i) {
            code += 0b00000001;
        }
        bit_width += 8; // add the bit_width of pre_code

        if (value == 280) {
            code = (code << 4) | extra_bit;
        }
        else if (value >= 281 && value <= 284) {
            code = (code << 5) | extra_bit;
        }
    }
    return {code, bit_width};
}

uint16_t reverse_extra_bit(uint16_t extra_bit, int bit_width) {
    if (bit_width <= 8) {
        auto valid = (uint8_t) extra_bit;
        valid = reverse_table[valid];
        extra_bit = valid >> (8 - bit_width);
    }
    else {
        auto valid1 = (uint8_t) extra_bit; // right 8
        auto valid2 = (uint8_t) (extra_bit >> 8); // left 8
        valid1 = reverse_table[valid1];
        valid2 = reverse_table[valid2];
        extra_bit = (uint16_t) valid1 << 8;
        extra_bit |= (uint16_t) valid2;
        extra_bit = extra_bit >> (16 - bit_width);
    }
    return extra_bit;
}

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
tuple<uint8_t, uint16_t, int> get_dis_code(int offset) {
    uint8_t pre_code;
    uint16_t extra_bit;
    int bit_width;
    if (offset <= 4) {
        bit_width = 0;
        extra_bit = 0x00;
        pre_code = offset - 1;
    }
    else if (offset == 5 || offset == 6) {
        bit_width = 1;
        extra_bit = offset - 5;
        pre_code = 4;
    }
    else if (offset == 7 || offset == 8) {
        bit_width = 1;
        extra_bit = offset - 7;
        pre_code = 5;
    }
    else if (offset <= 12) {
        bit_width = 2;
        extra_bit = offset - 9;
        pre_code = 6;
    }
    else if (offset <= 16) {
        bit_width = 2;
        extra_bit = offset - 13;
        pre_code = 7;
    }
    else if (offset <= 24) {
        bit_width = 3;
        extra_bit = offset - 17;
        pre_code = 8;
    }
    else if (offset <= 32) {
        bit_width = 3;
        extra_bit = offset - 25;
        pre_code = 9;
    }
    else if (offset <= 48) {
        bit_width = 4;
        extra_bit = offset - 33;
        pre_code = 10;
    }
    else if (offset <= 64) {
        bit_width = 4;
        extra_bit = offset - 49;
        pre_code = 11;
    }
    else if (offset <= 96) {
        bit_width = 5;
        extra_bit = offset - 65;
        pre_code = 12;
    }
    else if (offset <= 128) {
        bit_width = 5;
        extra_bit = offset - 97;
        pre_code = 13;
    }
    else if (offset <= 192) {
        bit_width = 6;
        extra_bit = offset - 129;
        pre_code = 14;
    }
    else if (offset <= 256) {
        bit_width = 6;
        extra_bit = offset - 193;
        pre_code = 15;
    }
    else if (offset <= 384) {
        bit_width = 7;
        extra_bit = offset - 257;
        pre_code = 16;
    }
    else if (offset <= 512) {
        bit_width = 7;
        extra_bit = offset - 385;
        pre_code = 17;
    }
    else if (offset <= 768) {
        bit_width = 8;
        extra_bit = offset - 513;
        pre_code = 18;
    }
    else if (offset <= 1024) {
        bit_width = 8;
        extra_bit = offset - 769;
        pre_code = 19;
    }
    else if (offset <= 1536) {
        bit_width = 9;
        extra_bit = offset - 1025;
        pre_code = 20;
    }
    else if (offset <= 2048) {
        bit_width = 9;
        extra_bit = offset - 1537;
        pre_code = 21;
    }
    else if (offset <= 3072) {
        bit_width = 10;
        extra_bit = offset - 2049;
        pre_code = 22;
    }
    else if (offset <= 4096) {
        bit_width = 10;
        extra_bit = offset - 3073;
        pre_code = 23;
    }
    else if (offset <= 6144) {
        bit_width = 11;
        extra_bit = offset - 4097;
        pre_code = 24;
    }
    else if (offset <= 8192) {
        bit_width = 11;
        extra_bit = offset - 6145;
        pre_code = 25;
    }
    else if (offset <= 12288) {
        bit_width = 12;
        extra_bit = offset - 8193;
        pre_code = 26;
    }
    else if (offset <= 16384) {
        bit_width = 12;
        extra_bit = offset - 12289;
        pre_code = 27;
    }
    else if (offset <= 24576) {
        bit_width = 13;
        extra_bit = offset - 16385;
        pre_code = 28;
    }
    else {
        bit_width = 13;
        extra_bit = offset - 24577;
        pre_code = 29;
    }
    if (bit_width >= 2) extra_bit = reverse_extra_bit(extra_bit, bit_width);
    return {pre_code, extra_bit, bit_width};
}

void write_code(uint8_t *out_code, int &cur_arr_pos, int &cur_bit_pos, uint16_t to_write, int bit_width) {
    if (cur_bit_pos == 0) out_code[cur_arr_pos] = 0;

    if (bit_width <= 8 - cur_bit_pos) {
        out_code[cur_arr_pos] |= to_write << (8-cur_bit_pos-bit_width);
        if (bit_width == 8 - cur_bit_pos) {
            cur_arr_pos++;
            cur_bit_pos = 0;
        }
        else cur_bit_pos += bit_width;
    }
    else if (bit_width <= 16 - cur_bit_pos) {
        out_code[cur_arr_pos] |= to_write >> (bit_width-8+cur_bit_pos);
        cur_arr_pos++;
        out_code[cur_arr_pos] = to_write << (16-bit_width-cur_bit_pos);
        if (bit_width == 16 - cur_bit_pos) {
            cur_arr_pos++;
            cur_bit_pos = 0;
        }
        else cur_bit_pos = bit_width-8+cur_bit_pos;
    }
    else {
        out_code[cur_arr_pos] |= to_write >> (bit_width-8+cur_bit_pos);
        cur_arr_pos++;
        out_code[cur_arr_pos] = to_write >> (bit_width-16+cur_bit_pos);
        cur_arr_pos++;
        out_code[cur_arr_pos] = to_write << (24-bit_width-cur_bit_pos);
        cur_bit_pos = bit_width-16+cur_bit_pos;
    }
}

/**
 * This method just for 5 bits
 * @param out_code
 * @param cur_arr_pos
 * @param cur_bit_pos
 * @param to_write
 */
void write_code(uint8_t *out_code, int &cur_arr_pos, int &cur_bit_pos, uint8_t to_write) {
    if (cur_bit_pos == 0) out_code[cur_arr_pos] = 0;

    if (cur_bit_pos <= 3) {
        out_code[cur_arr_pos] |= to_write << (3-cur_bit_pos);
        if (cur_bit_pos == 3) {
            cur_arr_pos++;
            cur_bit_pos = 0;
        }
        else cur_bit_pos += 5;
    }
    else {
        out_code[cur_arr_pos] |= to_write >> (cur_bit_pos-3);
        cur_arr_pos++;
        out_code[cur_arr_pos] = to_write << (11-cur_bit_pos);
        cur_bit_pos = cur_bit_pos-3;
    }
}

/**
 *
 * @param content
 * @param real_len
 * @param buffer_start not update the buffer_start in this method
 * @param match_index the index of the matched string
 * @return match_len
 * we guarantee buffer_start+3 will not out of bound
 */
tuple<int, int> match(const uint8_t *content, uint64_t real_len, int buffer_start, int *index) {
    array4 to_match = {content[buffer_start], content[buffer_start+1],
                           content[buffer_start+2], content[buffer_start+3]};
    int match_len = 0;
    int match_index = 0;
    bool matching = false;
    if (auto f = dict.find(to_match); f != dict.end()) {
        int cur_target = f->second;

        // we have at least one valid matching
        if (buffer_start-cur_target <= MAX_LEN) matching = true;

        while (matching) {
            int target_index = cur_target;
            int buffer_index = buffer_start;
            // the distance is too long, we do not use this matching pair
            if (buffer_index-target_index > MAX_LEN) break;
            int cur_match = 0;
            while (buffer_index < real_len && target_index < buffer_start) {
                if (content[target_index] != content[buffer_index]) break;
                target_index++;
                buffer_index++;
                cur_match++;
            }
            if (match_len < cur_match) {
                match_index = cur_target;
                match_len = cur_match;
            }
            if (index[cur_target] == cur_target) break;
            cur_target = index[cur_target];
        }

        // update hash, if matching is false the match_len will be 0, the loop will not execute
        for (int i = 0; i < match_len; ++i) {
            array4 insert = {content[buffer_start+i-3], content[buffer_start+i-2],
                             content[buffer_start+i-1], content[buffer_start+i]};
            if (auto f2 = dict.find(insert); f2 != dict.end()) {
                index[buffer_start+i-3] = f2->second;
                dict[insert] = buffer_start+i-3;
            }
            else {
                dict[insert] = buffer_start+i-3;
                index[buffer_start+i-3] = buffer_start+i-3;
            }
        }
    }

    if (!matching && buffer_start >= 3) {
        array4 insert = {content[buffer_start-3], content[buffer_start-2],
                         content[buffer_start-1], content[buffer_start]};
        if (auto f2 = dict.find(insert); f2 != dict.end()) {
            index[buffer_start-3] = f2->second;
            dict[insert] = buffer_start-3;
        }
        else {
            dict[insert] = buffer_start-3;
            index[buffer_start-3] = buffer_start-3;
        }
    }

    return {match_len, match_index};
}

uint32_t lz77(ifstream &in, ofstream &out) {
    uint32_t compressed_size = 0; // return compressed_size

    in.clear();
    in.seekg(0, ios::beg);

    char *input_block = new char[BLOCK]; // each time input BLOCK size chars
    auto *content = (uint8_t *) input_block;
    uint64_t real_len = 0;
    // we guarantee that the out_code length of each block is smaller than the array size
    auto *out_code = new uint8_t[2*BLOCK]{};
    uint8_t temp_code = 0x00; // it is the code for last block which has not been written into zip
    int temp_bit_width = 0;
    int cur_arr_pos = 0;
    int cur_bit_pos = 0;
    bool isLast = false;
    int *index = new int[BLOCK];
    do {
        write_code(out_code, cur_arr_pos, cur_bit_pos, temp_code, temp_bit_width);
        if (in) {
            in.read(input_block, BLOCK);
            real_len = in.gcount();
        }

        in.peek();
        if (in.good()) write_code(out_code, cur_arr_pos, cur_bit_pos, 0b010, 3);
        else {  // the last block
            write_code(out_code, cur_arr_pos, cur_bit_pos, 0b110, 3);
            isLast = true;
        }

        int buffer_start = 0;
        while (buffer_start < real_len) {
            // reach the boundary, no need to update dict
            if (real_len-buffer_start < 4) {
                auto [lit_code, bit_width] = get_lit_code(content[buffer_start]);
                write_code(out_code, cur_arr_pos, cur_bit_pos, lit_code, bit_width);
                buffer_start++;
                continue;
            }

            // todo
            if (cur_arr_pos > 62098) {
                int i = 0;
            }

            auto [match_len, match_index] = match(content, real_len, buffer_start, index);
            if (match_len) {
                auto [len_code, bit_width1] = get_len_code(match_len);
                write_code(out_code, cur_arr_pos, cur_bit_pos, len_code, bit_width1);
                auto [dis_code, extra_bit, bit_width2] = get_dis_code(buffer_start-match_index);
                write_code(out_code, cur_arr_pos, cur_bit_pos, dis_code);
                write_code(out_code, cur_arr_pos, cur_bit_pos, extra_bit, bit_width2);
                buffer_start += match_len;
            }
            else {
                auto [lit_code, bit_width] = get_lit_code(content[buffer_start]);
                write_code(out_code, cur_arr_pos, cur_bit_pos, lit_code, bit_width);
                buffer_start++;
            }
        }
        // write end of block
        write_code(out_code, cur_arr_pos, cur_bit_pos, 0b0000000, 7);

        // the encoding for this block is finished
        if (isLast) cur_arr_pos++;
        else if (cur_bit_pos != 0) {
            temp_code = out_code[cur_arr_pos];
            temp_bit_width = cur_bit_pos;
        }

        // todo

//        int zerocount = 0;

        // reverse byte
        for (int i = 0; i < cur_arr_pos; ++i) {
            out_code[i] = (char) reverse_table[out_code[i]];

            /*if (!out_code[i]) zerocount++;
            else zerocount = 0;
            if (zerocount > 10) {
                cout << i;
                throw zerocount;
            }*/
        }
        // write data
        out.write(reinterpret_cast<char *>(out_code), cur_arr_pos);

        compressed_size += cur_arr_pos;
        cur_arr_pos = 0;
        cur_bit_pos = 0;
        dict.clear();
    } while (!isLast);

    delete [] input_block;
    delete [] out_code;
    delete [] index;

    return compressed_size;
}

