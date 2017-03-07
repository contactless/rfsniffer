/*!
 * Just wrapper for script test_core.sh
 * for launching it as a part of automake tests
 * along with wb-homa=rfsniffer-test
 */

#include <cstdlib>

int main()
{
    int ret = system("tests/test_core.sh");
    return WEXITSTATUS(ret);
}
