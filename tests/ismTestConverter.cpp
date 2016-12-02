#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <cassert>
#include <vector>


std::vector <int> toBytes(const std::string &str) {
    if (str.length() % 2 != 0) {
        std::cerr << "Incorrect test, it is impossible to divide string into pairs of chars" << std::endl;
        exit(-1);
    }
    std::vector<int> bytes(str.size() / 2);
    for (int i = 0; i < (int)bytes.size(); i++)
        if (sscanf(str.substr(i * 2, 2).c_str(), "%x", &bytes[i]) != 1) {
            std::cerr << "Incorrect test, some problem with parsing bytes" << std::endl;
            exit(-1);
        }
            
    return bytes;
}

std::vector <int> bytesToBits(const std::vector <int> &bytes) {
    std::vector <int> bits;
    for (int i = 0; i < (int)bytes.size(); i++)
        for (int j = 7; j >= 0; j--)
            bits.push_back((bytes[i] >> j) & 1);
    return bits;
}

std::vector <int> bitsToIntervals(std::vector <int> bits, int discr_len, int mask_one) {
    bits.push_back(bits.back() ^ 1); // add a contrast bit to the end to make life easier
    
    std::vector <int> intervals;
    
    int length = 1;
    for (int i = 1; i < (int)bits.size(); i++) {
        if (bits[i] == bits[i - 1])
            length++;
        else {
            intervals.push_back(length * discr_len | (bits[i - 1] ? mask_one : 0));
            length = 1;
        }
    }
    return intervals;
    
}

int main(int argc, char* argv[]) {
    std::string str;
    std::cin >> str;
    std::vector <int> bytes = toBytes(str);
    std::vector <int> bits = bytesToBits(bytes);
    std::cerr << "BITS: ";
    for (int i = 0; i < (int)bits.size(); i++)
        std::cerr << bits[i];
    std::cerr << std::endl;
    std::cerr << "INTERVALS: " << std::endl;*/
    std::vector <int> intervals = bitsToIntervals(bits, 500, (1 << 24));
    for (int i = 0; i < (int)intervals.size(); i++)
        std::cerr << intervals[i] << " ";
    std::cerr << std::endl;
    fwrite(intervals.data(), sizeof(int), intervals.size(), stdout);
}
