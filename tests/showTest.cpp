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

int main()
{
    int N = 10000;
    unsigned data[N];
    N = fread(data, 4, N, stdin);
    for (int i = 0; i < N; i++)
        std::cout << ((data[i] >> 24) ? '+' : '-') << (data[i] & ((1 << 24) - 1)) << " ";
    std::cout << std::endl;
    return 0;
}
