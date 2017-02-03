/* This is separate one-source-file program
 * It just outputs test (given in lirc format) in human-readable format
 * */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <cassert>
#include <vector>

int main(int argc, char **argv)
{
    int splitLength = 10000; // defines where to make new line
    if (argc > 1)
        sscanf(argv[1], "%d", &splitLength);

    int N = 1000000;
    unsigned long int data[N];
    N = fread(data, 4, N, stdin);
    for (int i = 0; i < N; i++) {
        char pulseOrPauseChar = ((data[i] >> 24) ? '+' : '-');
        int length = (data[i] & ((1 << 24) - 1));
        std::cout << pulseOrPauseChar << length << (length < splitLength ? " " : "\n");
    }
    std::cout << std::endl;
    return 0;
}
