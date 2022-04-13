#include <filesystem>
#include <iostream>
#include "compression.h"

using namespace std;

int main() {
    string file_name = "big"; // file name without suffix
    string suffix = ".txt"; // file suffix
    string file_cname = file_name + suffix; // file name with suffix
    string input_path = "C:/Users/Christina/CLionProjects/ZipCompression3.0/"+file_cname;
    ifstream input_file (input_path, ios::in);
    ofstream output_file ("C:/Users/Christina/CLionProjects/ZipCompression3.0/"+file_name+".zip", ios::out | ios::binary);

    // set local file header
    local_file_header file_header;
    set_time(file_header.last_mod_time, file_header.last_mod_date);
    file_header.file_name_length = file_cname.length();
    auto [crc_code, uncompressed_size] = crc32(input_file, filesystem::file_size(input_path));
    file_header.crc32 = crc_code;
    file_header.uncompressed_size = uncompressed_size;
    write_local_file_header(output_file, file_header, file_cname, true);

    file_header.compressed_size = lz77(input_file, output_file);

    // set central directory header
    central_directory_header cd_header;
    cd_header.file_header_info = file_header;
    write_cd_header(output_file, cd_header, file_cname);

    // set end of central directory record
    end_of_cd_record cdr;
    cdr.cd_size = 46 + file_cname.length();
    cdr.offset_cd_disk = 30 + file_cname.length() + file_header.compressed_size;
    write_end_of_cd_record(output_file, cdr);

    // rewrite the compressed size
    output_file.seekp(18);
    write_little_end(output_file, file_header.compressed_size);

    return 0;
}


void set_time(uint16_t &mod_time, uint16_t &mod_date) { // set time for local file header
    time_t tt = time(nullptr);
    tm *t = localtime(&tt);
    mod_time = 0;
    mod_time |= t->tm_sec; // 5 bits
    mod_time |= t->tm_min << 5; // 6 bits
    mod_time |= t->tm_hour << 11; // 5 bits

    mod_date = 0;
    mod_date |= t->tm_mday; // 5 bits
    mod_date |= (t->tm_mon + 1) << 5; // 4 bits
    mod_date |= (t->tm_year - 80) << 9; // 7 bits
}

void write_little_end(ofstream &file, const uint32_t b) {
    char b_char[4];
    for (int i = 0; i < 4; ++i) {
         b_char[i] = (char) (b >> (i << 3));
    }
    file.write(b_char, 4);
}

void write_little_end(ofstream &file, const uint16_t b) {
    char b_char[2];
    for (int i = 0; i < 2; ++i) {
        b_char[i] = (char) (b >> (i << 3));
    }
    file.write(b_char, 2);
}

// from https://blog.csdn.net/joeblackzqq/article/details/37910839
tuple<uint32_t, uint32_t> crc32(ifstream &file, uint32_t size) {
    char *content = new char[size];
    if (file) {
        file.read(content, size);
        size = file.gcount();
    }
    uint32_t i, crc;
    crc = 0xFFFFFFFF;
    for (i = 0; i < size; i++) {
        crc = crc32tab[(crc ^ content[i]) & 0xff] ^ (crc >> 8);
    }
    delete [] content;
    return {crc^0xFFFFFFFF, size};
}



void write_local_file_header(ofstream &file, local_file_header &file_header, string &file_cname, bool is_local_header) {
    if (is_local_header) write_little_end(file, file_header.signature);
    write_little_end(file, file_header.version_extract);
    write_little_end(file, file_header.bit_flag);
    write_little_end(file, file_header.compression_method);
    write_little_end(file, file_header.last_mod_time);
    write_little_end(file, file_header.last_mod_date);
    write_little_end(file, file_header.crc32);
    write_little_end(file, file_header.compressed_size);
    write_little_end(file, file_header.uncompressed_size);
    write_little_end(file, file_header.file_name_length);
    write_little_end(file, file_header.extra_field_length);
    if (is_local_header) {
        const char *b_char = file_cname.data();
        file.write(b_char, (long long) file_cname.length());
    }
}

void write_cd_header(ofstream &file, central_directory_header &cd_header, string &file_cname) {
    write_little_end(file, cd_header.signature);
    write_little_end(file, cd_header.version_made);
    write_local_file_header(file, cd_header.file_header_info, file_cname, false);
    write_little_end(file, cd_header.file_comment_length);
    write_little_end(file, cd_header.disk_number_start);
    write_little_end(file, cd_header.internal_file_attributes);
    write_little_end(file, cd_header.external_file_attributes);
    write_little_end(file, cd_header.local_header_offset);
    const char *b_char = file_cname.data();
    file.write(b_char, (long long) file_cname.length());
}

void write_end_of_cd_record(ofstream &file, end_of_cd_record &cdr) {
    write_little_end(file, cdr.signature);
    write_little_end(file, cdr.disk_number);
    write_little_end(file, cdr.disk_cd);
    write_little_end(file, cdr.disk_entries);
    write_little_end(file, cdr.total_entries);
    write_little_end(file, cdr.cd_size);
    write_little_end(file, cdr.offset_cd_disk);
    write_little_end(file, cdr.file_comment_length);
}