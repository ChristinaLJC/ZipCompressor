# Project 1 - Zip Compressor - Report

ChristinaLJC

## Part1 - Analysis

Here are some analyses about zip compression according to RFC1951 and my comprehension. (Some contents of "Analysis" section are extracted from RFC1951, and the reference link is in the "References" section.)

### General Format-ZIP file (RFC)

Each file placed into a ZIP file must be preceded by a "local file header" record for that file. Each "local file header" MUST be accompanied by a corresponding "central directory header" record within the central directory section of the ZIP file. 

Data encryption MAY be used to protect files within a ZIP file. 

File data MAY be followed by a "data descriptor" for the file.

A ZIP file must contain an "end of central directory record" (only one).

Compression must not be applied to a "local file header", an "encryption header", or an "end of central directory record".

Overall .ZIP file format:

```
      [local file header 1]
      [encryption header 1] (maybe)
      [file data 1]
      [data descriptor 1] (maybe)
      . 
      .
      .
      [local file header n]
      [encryption header n]
      [file data n]
      [data descriptor n]
      [archive decryption header] 
      [archive extra data record] 
      [central directory header 1] (must if local file header 1 exists)
      .
      .
      .
      [central directory header n]
      [zip64 end of central directory record]
      [zip64 end of central directory locator] 
      [end of central directory record] (must)
```

### Local File Header (RFC)

The file header consists of fixed-length sections and extensions (may not exist). A header with an extension is identified by placing certain bits of the fixed-length part of the header.

```
      local file header signature     4 bytes  (0x04034b50)
      version needed to extract       2 bytes
      general purpose bit flag        2 bytes
      compression method              2 bytes
      last mod file time              2 bytes
      last mod file date              2 bytes
      crc-32                          4 bytes
      compressed size                 4 bytes
      uncompressed size               4 bytes
      file name length                2 bytes
      extra field length              2 bytes

      file name (variable size)
      extra field (variable size)
```

**CRC32**: Cyclic Redundancy Check.

### File Data

Determined by Deflate.

#### Block

Compression result outputs in blocks, that is, each time a certain amount of data is processed, the compression result of this part of data outputs once. Each block contains information about its own block.

Each block is compressed using a combination of the LZ77 algorithm and Huffman coding. The Huffman trees for each block are independent of those for previous or subsequent blocks; the LZ77 algorithm may use a reference to a duplicated string occurring in a previous block, up to 32K input bytes before. (RFC)

Encoded data blocks in the "deflate" format consist of sequences of symbols drawn from three conceptually distinct alphabets: either literal bytes, from the alphabet of byte values (0..255), or <length, backward distance> pairs, where the length is drawn from (3..258) and the distance is drawn from (1..32,768). In fact, the literal and length alphabets are merged into a single alphabet (0..285), where values 0..255 represent literal bytes, the value 256 indicates end-of-block, and values 257..285 represent length codes. (RFC)

##### Block Header

The head of the block is represented only by 3 bits.

The 1st bit: set as 1 if and only if this is the last block of the data set.

The 2nd and 3rd bits: specifies how the data are compressed (the 2 bits read from left to right).

- 00 - no compression

- 01 - compressed with fixed Huffman codes

- 10 - compressed with dynamic Huffman codes

- 11 - reserved (error)

  There are two methods of Huffman coding, namely Compression with fixed Huffman Codes (static Huffman) and Compression with Dynamic Huffman Codes. The former has a fixed coding table for different characters, which can be encoded directly according to the original characters in the compression table. The latter encodes the compressed content according to a code table based on the actual compressed content. In addition to that, there's something called "storage," which means no compression.

##### Block Body (Static Huffman)

It is a bit stream, not a byte stream. The head of the block is followed by a static Huffman encoded bitstream.

The Huffman code lengths for the literal/length alphabet are (from RFC):

```
Lit Value    Bits        Codes

---------    ----        -----

 0 - 143      8          00110000 through 10111111

 144 - 255    9          110010000 through 111111111

 256 - 279    7          0000000 through 0010111

 280 - 287    8          11000000 through 11000111
```

Lit Value is literal/length, 0-255 is ASCII, 256-287 is for some other meanings, and 286 and 287 will never be used. Bits is the length of a code word, and codes is the corresponding code words.  

Distance codes 0-31 are represented by (fixed-length) 5-bit codes, with possible additional bits.

##### End of Block

It is the code of 256. Here is where I make mistakes at first. I ignore the end of block, leading that some files I can compress successfully, while others do not.

#### Deflate

##### LZ77 and Window

LZ77 use (match length + offset).

The window refers to a block of memory with a size of 32KB. After the compression starts, the first step is to read a window of data to be compressed into this window, if there is not so much data to be compressed, then read all the current data to be compressed into this window. It is divided into two parts by function, lookup buffer and ahead buffer.

At the beginning of compression, only the ahead buffer is in the window, but not the lookup buffer. As compression progresses, the lookup buffer grows larger (but up to 32KB), while the ahead buffer grows smaller.

### Central Directory Header (RFC)

```
        central file header signature   4 bytes  (0x02014b50)
        version made by                 2 bytes
        version needed to extract       2 bytes
        general purpose bit flag        2 bytes
        compression method              2 bytes
        last mod file time              2 bytes
        last mod file date              2 bytes
        crc-32                          4 bytes
        compressed size                 4 bytes
        uncompressed size               4 bytes
        file name length                2 bytes
        extra field length              2 bytes
        file comment length             2 bytes
        disk number start               2 bytes
        internal file attributes        2 bytes
        external file attributes        4 bytes
        relative offset of local header 4 bytes

        file name (variable size)
        extra field (variable size)
        file comment (variable size)
```

### End of Central Directory Record (RFC)

```
      end of central dir signature    4 bytes  (0x06054b50)
      number of this disk             2 bytes
      number of the disk with the
      start of the central directory  2 bytes
      total number of entries in the
      central directory on this disk  2 bytes
      total number of entries in
      the central directory           2 bytes
      size of the central directory   4 bytes
      offset of start of central
      directory with respect to
      the starting disk number        4 bytes
      .ZIP file comment length        2 bytes
      .ZIP file comment       (variable size)
```

Offset of start of central directory with respect to the starting disk nupmber: it is actually the distance from the head of this large zip file to the head of central directory.

## Part2 - Designing

Files: compression.cpp, compression.h, lz77.cpp, lz77.h.

Compression function: can compression a single file successfully.

NOTE: All my implementations are following the above analysis. So the introduction in "Designing" section is not much.

### compression file 

- Use 3 structure to define the local file header, the central directory header, and the end of central directory record.
- Use crc32tab[] to store the information about crc32. (from https://blog.csdn.net/joeblackzqq/article/details/37910839)
- Use method set_time() to obtain current time and assign to the local file header and central directory header.
- Use method write_little_end() to write all the information into the output file by little end.
- Use write_local_file_header(), write_cd_header(), write_end_of_cd_record() to write information of the 3 structures to the output file.
- Use method crc32() to calculate crc32. (from https://blog.csdn.net/joeblackzqq/article/details/37910839). 

### lz77 file

- MAX_DISTANCE 32768, MAX_LEN 258, BLOCK 327680.
- typedef std::array<uint8_t, 4> array4
- I define 2 structure arr_hash and arr_equal to let the type array4 be able to do hash in unordered_map
- reverse_table[] is to reverse a byte. (from https://blog.csdn.net/wanruiou/article/details/104049881) The calculation of crc32 and reverse_table are the only parts I extract online.
- Other methods in lz77 are all demonstrated in comments. More details can be seen in the file.

## Part3 - Implementation Process

- I actually changed the designing a lot of times, and in order to maintain the function I have achieved successfully. I learned to give the project an iteration version. I have ZipCompression1.0, ZipCompression2.0, ZipCompression3.0......

- ZipCompression1.0 is to achieve the compression function preliminarily. And in this version, I use a brute force method to find the matching pairs in LZ77 process. Although this version can compress a few files, it still has lots of bugs. It is unstable.

- ZipCompression2.0 is to improve the matching method in LZ77 process. I redesigned the matching rules. I use hash map and linked list to store the dictionary, in order to accelerate the matching process. And I found the bug that I ignored the "end of block" code after each block. And also, I found that I did not initialize the output array. After fixing these bugs, the stability of the compression improved. 

- ZipCompression3.0 is to fix bugs for more conditions. In order to test more conditions, I deliberately wrote a .txt file with a very long sentence which is repeated twice. Then the decompression software told me a CRC mistake. But I checked the CRC code, it is correct. Then the problem may exist in the LZ77 process. After comparing the compression code with a successful compression, I found that the extra bits of the length code and distance code should be written in opposite order to literal code. Then I can successfully compress some files that are smaller than 4 kb. 

  I tested a file larger than 4kb, but the decompression software told me a crc mistake. I checked the hex numbers in the zip, and found a section full of 0. It is ridiculous. Then I found a bug in write_code method which is designed to write bit stream. After fixing this, I can compress the files up to 30kb. It is a big step forward.

  Then, I can compress a 32 kb file, but not a 34 kb file. The 32 kb and 34 kb are so close to 32768 which is the max window size we should have. It seem that the bug has something to do with the window size. So I checked my code about window size and found that I ineffectually limited the window size. After fixing it, I was glad to find that I can compress a file up to 60 kb. Another big step!
  
  Then, I can compress a 64 kb file, but not a 65 kb file. The 64 kb happens to be the block size I defined. So I checked the code step by step when it reached the boundary, and I found that there is a cur_bit_pos I did not set to 0 while beginning a new block. After fixing it, I test a 1 mb file, and it works!
  
- ZipCompression4.0. In this version, I tried to test some files in different formats. I tried .docx, but it failed. I was really confused. My compressed size of .docx is strangely small. There may be something wrong with the input. So I checked the file IO and let it input in binary format. Then I recompressed the .docx, it worked. And then I tested .pdf, .md, .xlsx, .csv, .pptx, .jpeg, .jpg and .png...... They all worked.

- ZipCompression5.0. In this version, I added some special judge to improve the compression speed especially when the file has a high repetition rate and fixed some small bugs. Up to this point, I thought the compression function of my project is approximately completed. (Although it may still has small bugs, I have not found yet.)


## Part4 - Brief Highlight and Feeling

- I completed this project version by version, which I had not tried before.
- Actually, I sincerely think that finishing this project is a highlight for me. I thought I cannot finish it on my own at first. But after updating my zip compression project version by version. I think I make it.
- It is important to design a proper framework of a project. A proper framework can make the implementation easier and clearer.

## References

https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

https://www.w3.org/Graphics/PNG/RFC-1951#codes

https://blog.csdn.net/jison_r_wang/category_6335784.html

https://blog.csdn.net/a200710716/article/details/51644421

https://www.hanshq.net/zip.html

https://www.cnblogs.com/esingchan/p/3958962.html

https://blog.csdn.net/wanruiou/article/details/104049881

https://blog.csdn.net/joeblackzqq/article/details/37910839
