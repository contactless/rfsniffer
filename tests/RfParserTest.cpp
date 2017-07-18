/* An automatical test of parsers of different protocols
 * It pushes dumps (may be artificial) of messages to parser
 * and compares result with given
 * */
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <dirent.h>
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "../libs/libutils/DebugPrintf.h"
#include "../libs/librf/RFParser.h"
#include "../libs/librf/RFProtocolLivolo.h"
#include "../libs/librf/RFProtocolX10.h"
#include "../libs/librf/RFProtocolRST.h"
#include "../libs/librf/RFProtocolRaex.h"
#include "../libs/librf/RFProtocolNooLite.h"
#include "../libs/librf/RFProtocolOregon.h"

#define MAX_SIZE 100000

typedef std::string string;
using namespace strutils;

string DecodeFile(CRFParser *parser, const char *fileName)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Start\n");

    base_type *data = new base_type[MAX_SIZE];
    size_t dataSize = 0;
    // reading data
    {
        FILE *f = fopen(fileName, "rb");
        if (!f)
            return "File not found";
        dataSize = fread(data, sizeof(base_type), MAX_SIZE, f);
        fclose(f);
    }
    dprintf("$P 1 dataSize = %\n", dataSize);

    string result = parser->Parse(data, dataSize);
    dprintf("$P 2 result = %\n", result);

    delete []data;
    dprintf("$P Before finish(return)\n");
    return result;
}

typedef const char *pstr;
typedef const pstr pstr_pair[2];
typedef pstr_pair *ppstr_pair;

typedef std::pair<string, string> string_pair;


void getAllTestFiles( string path, String::Vector &result )
{
    DIR *dirFile = opendir( path.c_str() );
    if ( dirFile ) {
        struct dirent *hFile;
        errno = 0;
        while (( hFile = readdir( dirFile )) != NULL ) {
            if ( !strcmp( hFile->d_name, "."  )) continue;
            if ( !strcmp( hFile->d_name, ".." )) continue;

            // in linux hidden files all start with '.'
            bool gIgnoreHidden = true;
            if ( gIgnoreHidden && ( hFile->d_name[0] == '.' )) continue;

            // dirFile.name is the name of the file. Do whatever string comparison
            // you want here. Something like:
            if ( strstr( hFile->d_name, ".rcf" ))
                result.push_back(hFile->d_name);
        }
        closedir( dirFile );
    }
}


bool OneTest(const string &path, const string_pair &test, CRFParser &parser)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Start test=%\n", test);

    String file_name = test.first, exp_result = test.second;

    if (file_name.empty()) {
        dprintf("$P Finish because empty name\n");
        return true;
    }

    // expected values
    String exp_type, exp_value;
    String::Map exp_values;

    // parsed values
    String res_type, res_value;
    String::Map res_values;


    dprintf("$P Before parsing test\n");
    try {
        exp_result.SplitByExactlyOneDelimiter(":", exp_type, exp_value);
        if (exp_value.find('=') != exp_value.npos)
            exp_values = exp_value.SplitToPairs();
    } catch (CHaException ex) {
        printf("Failed! Format of TEST is incorrect! Test: %s\n", exp_result.c_str());
        return false;
    }

    dprintf("$P Before decoding\n");
    dprintf("$P exp_type = %, size(exp_values) = %\n", exp_type, exp_values.size());
    String res = DecodeFile(&parser, (path + file_name).c_str());


    dprintf("$P After decoding before parsing decoded (got: %)\n", res);

    // Simple check for cases with not "a=1 b=2..." format
    if (res == exp_result)
        return true;
    try {
        res.SplitByExactlyOneDelimiter(":", res_type, res_value);
        if (res_value.find('=') != res_value.npos)
            res_values = res_value.SplitToPairs();
    } catch (CHaException ex) {
        printf("Failed! Format is incorrect! File:%s, result:%s, Expected: %s\n", file_name.c_str(),
               res.c_str(), exp_result.c_str());
        return false;
    }


    dprintf("$P Before checking\n");

    if (res_type != exp_type) {
        printf("Failed! Types mismatch! File:%s, result:%s, Expected: %s\n", file_name.c_str(), res.c_str(),
               exp_result.c_str());
        return false;
    }
    // Compare values, extra values in parsed result are ignored
    for (auto value_pair : exp_values) {
        dprintf("$P Compare with % = %\n", value_pair.first, value_pair.second);
        if (value_pair.second != res_values[value_pair.first]) {
            // there may be a regular expression for check by unix grep
            if (value_pair.second.length() > 0 && value_pair.second[0] == '[')
                continue;
            printf("Failed! Field\"%s\" mismatch! \n\tFile:%s, "\
                   " result:%s, Expected: %s\n",
                   value_pair.first.c_str(), file_name.c_str(),
                   res_values[value_pair.first].c_str(), value_pair.second.c_str());
            return false;
        }
    }


    dprintf("$P Finish\n");
    return true;
}

bool GroupTest(const string &group, const std::vector<string> &protocols) {
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Start\n");

    CRFParser parser;
    for (const auto &protocol : protocols)
        parser.AddProtocol(protocol);

    bool allPassed = true;

    // read tests from descriptive file
    {
        std::ifstream test_file;
        test_file.open("tests/testfiles/" + group + ".desc");
        if (test_file.is_open()) {
            String test;
            while (std::getline(test_file, test)) {
                String testWitoutComment, comment;
                test.SplitByFirstOccurenceDelimiter('#', testWitoutComment, comment);
                String file, answer;
                testWitoutComment.SplitByFirstOccurenceDelimiter(' ', file, answer);
                //std::cout << "Test: " << file << " & " << answer << std::endl;;
                allPassed &= OneTest("tests/testfiles/" + group + "/" , std::pair<string, string>(file, answer), parser);
                dprintf("$P OK!\n");
            }
            test_file.close();
        } else {
            printf("Can't open testfiles descriptive file.");
            exit(1);
        }
    }

    return allPassed;
}

void RfParserTest(string path)
{
    DPRINTF_DECLARE(dprintf, false);
    dprintf("$P Start\n");

    bool allPassed = true;

    if (path.length()) {
        CRFParser parser;
        parser.AddProtocol("All");

        String::Vector files;
        getAllTestFiles("./" + path, files);
        std::sort(files.begin(), files.end());
        printf("Directory %s, %d files\n", path.c_str(), (int)files.size());
        for (const string &f : files)
            printf("\t\t%s\n", f.c_str());
        for (const string &f : files) {
            string res = DecodeFile(&parser, ("./" + path + "/" + f).c_str());
            printf("File: %s, decoded: %s\n", f.c_str(), res.c_str());
        }
        return;
    }

    allPassed &= GroupTest("mix", {"All"});
    allPassed &= GroupTest("vhome", {"VHome"});

    dprintf("$P Before finish (%)\n", (allPassed ? "tests PASSED" : "tests FAILED"));

    if (!allPassed)
        exit(1);
}
