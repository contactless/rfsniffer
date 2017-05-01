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
#include <stdint.h>

struct TLircElement {
    typedef int32_t TInt;
    union {
        TInt Raw : 32;
        struct {
            TInt Length : 24;
            bool IsPulse : 1, : 7;
        };
    };
};

int main(int argc, char **argv)
{
    int splitLength = 10000; // defines where to make new line
    if (argc > 1)
        sscanf(argv[1], "%d", &splitLength);

    int N = 1000000;
    TLircElement data[N];
    //std::cout << sizeof(int) << std::endl;
    //std::cout << sizeof(TLircElement) << std::endl;
    N = fread(data, sizeof(TLircElement), N, stdin);
    for (int i = 0; i < N; i++) {
        //std::cout << "(" << data[i].Raw << "; " << (data[i].IsPulse ? "+" : "-") << data[i].Length << ")" << std::endl;
        std::cout << (data[i].IsPulse ? "+" : "-") << data[i].Length << (data[i].Length < splitLength ?
                  " " : "\n");
        //char pulseOrPauseChar = ((data[i] >> 24) ? '+' : '-');
        //int length = (data[i] & ((1 << 24) - 1));
        //std::cout << pulseOrPauseChar << length << (length < splitLength ? " " : "\n");
    }
    std::cout << std::endl;
    return 0;
}
