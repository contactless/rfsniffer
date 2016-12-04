
#include <algorithm>

#include "stdafx.h"
#include <sys/types.h>
#include <dirent.h>
#include "../libs/libutils/logging.h"
#include "../libs/libutils/Exception.h"
#include "../libs/librf/RFParser.h"
#include "../libs/librf/RFProtocolLivolo.h"
#include "../libs/librf/RFProtocolX10.h"
#include "../libs/librf/RFProtocolRST.h"
#include "../libs/librf/RFProtocolRaex.h"
#include "../libs/librf/RFProtocolNooLite.h"
#include "../libs/librf/RFProtocolOregon.h"


#define MAX_SIZE 10000

string DecodeFile(CRFParser *parser, CLog *log, const char *fileName)
{
    FILE *f = fopen(fileName, "rb");
    if (!f)
        return "File not found";

    base_type *data = new base_type[MAX_SIZE];
    size_t dataSize = fread(data, sizeof(base_type), MAX_SIZE, f);
    fclose(f);

    string result = parser->Parse(data, dataSize);
    delete []data;
    return result;
}

typedef const char *pstr;
typedef const pstr pstr_pair[2];
typedef pstr_pair *ppstr_pair;

typedef std::pair<string, string> string_pair;

static const std::vector<string_pair> Tests = {
    /*{ "capture-1311-190209.rcf", "Rubitek:0110111010111110000010110" },
    { "capture-1311-190019.rcf", "Rubitek:0110111010111110000011100" },
    { "capture-1311-190013.rcf", "Rubitek:0110111010111110000001110" },
    { "capture-1311-190018.rcf", "Rubitek:0110111010111110000011100" },
    { "capture-1311-190202.rcf", "" },
    { "capture-1311-190206.rcf", "" },
    { "capture-1311-190014.rcf", "Rubitek:0110111010111110000001110" },
    { "capture-1311-190019.rcf", "Rubitek:0110111010111110000011100" },
    { "capture-1311-190204.rcf", "Rubitek:0110111010111110000010110" },
    { "capture-1311-190209.rcf", "Rubitek:0110111010111110000010110" },*/

    { "capture-0307-215444.rcf", "nooLite:flip=1 cmd=4 addr=9a13 fmt=00 crc=76" },
    { "capture-0307-215449.rcf", "nooLite:flip=0 cmd=4 addr=9a13 fmt=00 crc=6a" },
    { "capture-2706-190620.rcf", "nooLite:flip=0 cmd=21 type=2 t=29.6 h=39 s3=ff bat=0 addr=1492 fmt=07 crc=ec"},
    { "capture-2706-143835.rcf", "nooLite:flip=1 cmd=21 type=2 t=30.3 h=43 s3=ff bat=0 addr=1492 fmt=07 crc=d3"},
    { "capture-2706-093124.rcf", "nooLite:flip=1 cmd=5 addr=9a12 fmt=00 crc=e5" },
    { "capture-2706-093129.rcf", "nooLite:flip=0 cmd=10 addr=9a12 fmt=00 crc=88" },
    { "capture-2706-093217.rcf", "nooLite:flip=0 cmd=15 addr=9a11 fmt=00 crc=b4" },
    { "capture-2706-093128.rcf", "nooLite:flip=1 cmd=5 addr=9a12 fmt=00 crc=e5" },
    { "capture-2706-093221.rcf", "nooLite:flip=1 cmd=15 addr=9a11 fmt=00 crc=a8" },
    { "capture-2706-093205.rcf", "nooLite:flip=0 cmd=15 addr=9a13 fmt=00 crc=fb" },
    //{ "","" },
    { "capture-2506-120004.rcf", "nooLite:flip=0 cmd=5 addr=9a13 fmt=00 crc=52"},
    { "capture-2506-115944.rcf", "nooLite:flip=1 cmd=5 addr=9a13 fmt=00 crc=4e" },

    //	{ "capture-0906-212933.rcf", "" },* /
#define ROLLING_CODE "51" // or "15" as in original code
    // for some reason in original code Rolling Code was read in ambigous way (either ascending order of nibbles or descending one)
    // I prefer to read it in fixed way and just change tests
    { "capture-0906-214352.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=22.6 h=41" },
    { "capture-0906-214234.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=22.6 h=41" },
    { "capture-0906-212618.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=22.7 h=41" },
    { "capture-0906-183444.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=23.3 h=39" },
    { "capture-0906-165011.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=24.2 h=40" },
    { "capture-0906-164649.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=23.9 h=40" },
    { "capture-0706-211823.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=24.7 h=46" },
    { "capture-0906-210412.rcf", "Oregon:type=1D20 id="ROLLING_CODE" ch=1 t=23.0 h=40" },
#undef ROLLING_CODE
    // Tests ported from ism-radio
    { "ism-test-3b374d96c9.rcf", "Oregon:type=1D20 id=12 ch=4 battery=low t=26.3 h=35" },
    { "ism-test-ca09da62af.rcf", "Oregon:type=1D20 id=85 ch=2 battery=low t=25.6 h=43"},
    { "ism-test-d66b21c765.rcf", "Oregon:type=F824 id=42 ch=1 battery=normal t=26.9 h=39"},
    { "ism-test-41d890077f.rcf", "Oregon:type=F824 id=42 ch=1 battery=normal t=26.7 h=39"},
    { "ism-test-df05ec91e6.rcf", "Oregon:type=F824 id=42 ch=1 battery=normal t=26.6 h=39"},
    { "ism-test-7031d3158b.rcf", "Oregon:type=F824 id=42 ch=1 battery=normal t=26.4 h=39"},
    { "ism-test-7b8d00e86c.rcf", "Oregon:type=F824 id=42 ch=1 battery=normal t=26.3 h=40"},

    { "capture-2708-132748.rcf", "X10:D2ON" },
    { "capture-2708-132756.rcf", "X10:D2ON" },
    { "capture-2708-132933.rcf", "X10:D2ON" },

    //*
    { "capture-1303-212826.rcf", "" }, // Inverted X10:D2ON" },
    { "capture-1303-204025.rcf", "" }, // Inverted X10:D2ON" },
    { "capture-2902-213735.rcf", "" }, // Inverted X10:D2ON" },
    //{ "capture-2902-214441.rcf", "Livolo:00110011110100010001000" }, FAILED INVERTED
    //*
    { "capture-1304-214720.rcf", "Livolo:00110011110100010001000" },
    { "capture-1304-214730.rcf", "Livolo:00110011110100010010000" },
    { "capture-1304-214741.rcf", "Livolo:00110011110100010010000" },
    { "capture-1304-214753.rcf", "Livolo:00110011110100010111000" },
    { "capture-1304-214758.rcf", "Livolo:00110011110100010101010" },
    { "capture-1304-222352.rcf", "Livolo:00000110101001000110000" },
    { "capture-1304-222343.rcf", "Livolo:00000110101001001010000" },


    { "capture-0604-212458.rcf", "Livolo:00110011110100010001000" },
    { "capture-0604-212505.rcf", "Livolo:00110011110100010001000" },
    { "capture-0604-212552.rcf", "Livolo:00110011110100010001000" },
    //* /

    { "capture-1604-081847.rcf", "Raex:raw=F07BF0407FFFFF ch=F0 btn=1" },
    { "capture-1604-080728.rcf", "Raex:raw=F07BF0407FFFFF ch=F0 btn=1" },
    { "capture-1404-083759.rcf", "Raex:raw=F07BF0407FFFFF ch=F0 btn=1" },
    { "capture-1404-083803.rcf", "Raex:raw=F07BF0407FFFFF ch=F0 btn=1" },
    { "capture-1404-083811.rcf", "Raex:raw=087BF0807FFFFF ch=08 btn=2" },

    //File: capture-1408-204138.rcf, decoded: RST:id=1b00 h=46 t=14.1
    { "capture-1408-204138.rcf", "RST:id=1b00 h=46 t=14.1" },

    //File: capture-1004-105819.rcf, decoded: RST:id=1b10 h=82 t=29.1
    { "capture-1004-105819.rcf", "RST:id=1b10 h=82 t=29.1" },

    // Inverted
    //	{ "capture-0804-094607.rcf", "?" },
    { "capture-0904-091533.rcf", ""}, //RST:id=1b10 h=20 t=26.7" },
    { "capture-0904-091545.rcf", ""}, //RST:id=1b10 h=20 t=26.9" },
    { "capture-1004-121901.rcf", ""}, //RST:id=1b10 h=34 t=27.2"},

    {"capture-1004-122939.rcf", ""},  //FAILED
    {"capture-1004-125748.rcf", "" }, // Inverted X10:D2ON" },X10:D2OFF" },
};

void getAllTestFiles( string path, string_vector &result )
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


bool OneTest(const string_pair &test, CLog *log, CRFParser &parser)
{
    string file_name = test.first, exp_result = test.second;

    // expected values
    string exp_type, exp_value;
    string_map exp_values;

    // parsed values
    string res_type, res_value;
    string_map res_values;

    // printf("Test #%d: %s\n", (int)(test - Tests), (*test)[1]);

    try {
        SplitPair(exp_result, ':', exp_type, exp_value);
        if (exp_value.find(' ') != exp_value.npos)
            SplitValues(exp_value, exp_values);
    } catch (CHaException ex) {
    }

    string res = DecodeFile(&parser, log, (string("tests/testfiles/") + file_name).c_str());


    // Simple check for cases with not "a=1 b=2..." format
    if (res == exp_result)
        return true;
    try {
        SplitPair(res, ':', res_type, res_value);
        if (res_value.find(' ') != res_value.npos)
            SplitValues(res_value, res_values);
    } catch (CHaException ex) {
        printf("Failed! Format is incorrect! File:%s, result:%s, Expected: %s\n", file_name.c_str(),
               res.c_str(), exp_result.c_str());
        return false;
    }

    if (res_type != exp_type) {
        printf("Failed! Types mismatch! File:%s, result:%s, Expected: %s\n", file_name.c_str(), res.c_str(),
               exp_result.c_str());
        return false;
    }
    // Compare values, extra values in parsed result are ignored
    for (auto value_pair : exp_values)
        if (value_pair.second != res_values[value_pair.first]) {
            printf("Failed! Field\"%s\" mismatch! \n\tFile:%s, result:%s, Expected: %s\n",
                   value_pair.second.c_str(), file_name.c_str(), res.c_str(), exp_result.c_str());
            return false;
        }
    return true;
}

void RfParserTest(string path)
{
    bool allPassed = true;
    CLog *log = CLog::GetLog("Main");
    CRFParser parser(log);
    //parser.EnableAnalyzer();

    //parser.AddProtocol(new CRFProtocolRST());
    //parser.AddProtocol(new CRFProtocolLivolo());
    //parser.AddProtocol(new CRFProtocolX10());
    //parser.AddProtocol(new CRFProtocolRaex());
    //parser.AddProtocol("Oregon");
    parser.AddProtocol("All");

    if (path.length()) {
        string_vector files;
        getAllTestFiles("./" + path, files);
        std::sort(files.begin(), files.end());
        printf("Directory %s, %d files\n", path.c_str(), files.size());
        for (const string &f : files)
            printf("\t\t%s\n", f.c_str());
        for (const string &f : files) {
            string res = DecodeFile(&parser, log, ("./" + path + "/" + f).c_str());
            printf("File: %s, decoded: %s\n", f.c_str(), res.c_str());
        }
        return;
    }

    for (const string_pair test : Tests)
        allPassed &= OneTest(test, log, parser);

    if (!allPassed)
        exit(1);
}
