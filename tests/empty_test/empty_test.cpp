
/*
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
* All rights reserved
* This file is part of the zbase, used MIT License.
*/



#include <unordered_map>
#include <unordered_set>
#include "fn_log.h"
#include "zprof.h"
#include "zarray.h"
#include "test_common.h"


struct MyStruct
{
public:
    const static u32 MODULE_TYPE = 5;
};

#define D(m)   z.push_back(decltype(m)::MODULE_TYPE);




int main(int argc, char *argv[])
{
    ztest_init();

    PROF_DEFINE_AUTO_ANON_RECORD(delta, "self use mem in main func begin and exit");
    PROF_OUTPUT_SELF_MEM("self use mem in main func begin and exit");


    u64 salt = std::atoll("");


    LogInfo() << "all test finish .salt:" << salt;
    return 0;
}




