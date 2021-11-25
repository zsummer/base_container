

#include "fn_log.h"
#include <string>
#include <zhash_map.h>
#include "zcontain_test.h"
#include "zmalloc.h"
#include "zprof.h"
#include "zarray.h"
#define Now() std::chrono::duration<double>(std::chrono::system_clock().now().time_since_epoch()).count()


using namespace zsummer;


s32 zmalloc_stress()
{
    std::unique_ptr<zmalloc> zstate(new zmalloc());
    memset(zstate.get(), 0, sizeof(zmalloc));
    zstate->max_reserve_block_count_ = 100;
    zstate->set_global(zstate.get());

    static const u32 rand_size = 1000 * 10000;
    static_assert(rand_size > zmalloc::DEFAULT_BLOCK_SIZE, "");
    u32* rand_array = new u32[rand_size];
    static const u32 cover_size = zmalloc::BIG_MAX_REQUEST;
    using Addr = void*;
    zarray <Addr, cover_size>* buffers = new zarray <Addr, cover_size>();
    zarray <Addr, cover_size>* buffers2 = new zarray <Addr, cover_size>();

    for (u32 i = 0; i < zmalloc::DEFAULT_BLOCK_SIZE; i++)
    {
        rand_array[i] = i;
    }
    for (u32 i = zmalloc::DEFAULT_BLOCK_SIZE; i < rand_size; i++)
    {
        rand_array[i] = rand();
    }



    PROF_DEFINE_COUNTER(cost);
    PROF_START_COUNTER(cost);
    for (u64 i = 0; i < rand_size; i++)
    {
        global_zfree(global_zmalloc(1));
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(global_zmalloc(1))", rand_size, cost.stop_and_save().cycles());

    PROF_START_COUNTER(cost);
    for (u64 i = 0; i < rand_size; i++)
    {
        u32 test_size = rand_array[i] % (1024);
        void* p = global_zmalloc(test_size);
        global_zfree(p);
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(global_zmalloc(0~1024))", rand_size, cost.stop_and_save().cycles());

    PROF_START_COUNTER(cost);
    for (u64 i = 0; i < rand_size; i++)
    {
        u32 test_size = (rand_array[i] % (zmalloc::BIG_MAX_REQUEST - zmalloc::SMALL_MAX_REQUEST)) + zmalloc::SMALL_MAX_REQUEST;
        void* p = global_zmalloc(test_size);
        global_zfree(p);
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(global_zmalloc(1024~512k))", rand_size, cost.stop_and_save().cycles());

    PROF_START_COUNTER(cost);
    for (u64 i = rand_size/2; i < rand_size; i++)
    {
        u32 test_size = rand_array[i] % (zmalloc::BIG_MAX_REQUEST * 2);
        void* p = global_zmalloc(test_size);
        global_zfree(p);
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(global_zmalloc(0~1M))", rand_size, cost.stop_and_save().cycles());

    PROF_START_COUNTER(cost);
    for (u64 i = rand_size / 2; i < rand_size; i++)
    {
        u32 test_size = rand_array[i] % (zmalloc::BIG_MAX_REQUEST * 2);
        void* p = global_zmalloc(test_size);
        buffers->push_back(p);
        global_zfree(p);
        if (buffers->full())
        {
            buffers->clear();
        }
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(global_zmalloc(0~1M))", rand_size, cost.stop_and_save().cycles());
    buffers->clear();

    for (size_t loop= 0; loop < 40; loop++)
    {
        PROF_START_COUNTER(cost);
        for (u64 i = cover_size/ 40 * loop; i < cover_size / 40 * (loop+1); i++)
        {
            u32 test_size = rand_array[i] % (zmalloc::BIG_MAX_REQUEST);
            void* p = global_zmalloc(test_size);
            *(u32*)p = (u32)i;
            buffers->push_back(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("global_zmalloc(0~512k)", buffers->size(), cost.stop_and_save().cycles());
        if (loop < 2 || loop >37)
        {
            LogDebug() << zmalloc::instance().debug_string();
        }
        PROF_START_COUNTER(cost);
        for (auto p : *buffers)
        {
            global_zfree(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("global_zfree(0~512k)", buffers->size(), cost.stop_and_save().cycles());
        buffers->clear();
    }

    for (size_t loop = 0; loop < 40; loop++) 
    {
        if (loop%2 == 0 && loop != 0)
        {
            continue;//ϵͳ����̫����  ����Ҫ�����Բ���  ɾ��һ������. 
        }
        PROF_START_COUNTER(cost);
        for (u64 i = cover_size / 40 * loop; i < cover_size / 40 * (loop + 1); i++)
        {
            u32 test_size = rand_array[i] % (zmalloc::BIG_MAX_REQUEST);
            test_size = test_size <8  ? 8 : test_size;
            void* p = malloc(test_size);
            *(u32*)p = (u32)i;
            buffers->push_back(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("sys malloc(0~512k)", buffers->size(), cost.stop_and_save().cycles());
        zstate->check_health();
        PROF_START_COUNTER(cost);
        for (auto p : *buffers)
        {
            free(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("sys free(0~512k)", buffers->size(), cost.stop_and_save().cycles());
        buffers->clear();
        zstate->check_health();
    }
    if (true)
    {
        buffers->clear();
        buffers2->clear();
        PROF_START_COUNTER(cost);
        u64 alloc_count = 0;
        u64 free_count = 0;
        for (u64 i = 0; i < cover_size; i++)
        {
            u32 push_size1 = rand_array[i] % (2048);
            u32 push_size2 = rand_array[cover_size - i] % (2048);
            if ((push_size1 + push_size2) % 3 == 0 || buffers->full())
            {
                if (!buffers->empty())
                {
                    global_zfree(buffers->back());
                    buffers->pop_back();
                    free_count++;
                }
            }
            if ((push_size1 + push_size2) % 7 == 0 || buffers2->full())
            {
                if (!buffers2->empty())
                {
                    global_zfree(buffers2->back());
                    buffers2->pop_back();
                    free_count++;
                }
            }

            buffers->push_back(global_zmalloc(push_size1));
            buffers2->push_back(global_zmalloc(push_size2));
            alloc_count += 2;
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("rand zmalloc/zfree(0~2k)", alloc_count + free_count, cost.stop_and_save().cycles());
        zstate->check_health();
        LogDebug() << zmalloc::instance().debug_string();
        for (auto p : *buffers)
        {
            global_zfree(p);
        }
        for (auto p : *buffers2)
        {
            global_zfree(p);
        }
        buffers->clear();
        buffers2->clear();
    }
    
    if (true)
    {
        buffers->clear();
        buffers2->clear();
        PROF_START_COUNTER(cost);
        u64 alloc_count = 0;
        u64 free_count = 0;
        for (u64 i = 0; i < cover_size; i++)
        {
            u32 push_size1 = rand_array[i] % (2048) + 1;
            u32 push_size2 = rand_array[cover_size - i] % (2048) + 1;
            if ((push_size1 + push_size2) % 3 == 0 || buffers->full())
            {
                if (!buffers->empty())
                {
                    free(buffers->back());
                    buffers->pop_back();
                    free_count++;
                }
            }
            if ((push_size1 + push_size2) % 7 == 0 || buffers2->full())
            {
                if (!buffers2->empty())
                {
                    free(buffers2->back());
                    buffers2->pop_back();
                    free_count++;
                }
            }

            buffers->push_back(malloc(push_size1));
            buffers2->push_back(malloc(push_size2));
            alloc_count += 2;
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("rand sys malloc/free(0~2k)", alloc_count + free_count, cost.stop_and_save().cycles());
        for (auto p : *buffers)
        {
            free(p);
        }
        for (auto p : *buffers2)
        {
            free(p);
        }

        buffers->clear();
        buffers2->clear();
    }


    //�̶�С�ֽ�����  
    static int fixed_size = 100; 
    for (size_t loop = 0; loop < 40; loop++) //ϵͳ����̫���� ֻ
    {
        if (loop % 2 == 0 && loop != 0)
        {
            continue;//ϵͳ����̫����  ����Ҫ�����Բ���  ɾ��һ������. 
        }
        PROF_START_COUNTER(cost);
        for (u64 i = cover_size / 40 * loop; i < cover_size / 40 * (loop + 1); i++)
        {
            u32 test_size = fixed_size;
            void* p = malloc(test_size);
            buffers->push_back(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("sys malloc(100) bat", buffers->size(), cost.stop_and_save().cycles());
        zstate->check_health();
        PROF_START_COUNTER(cost);
        for (auto p : *buffers)
        {
            free(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("sys free(100) bat", buffers->size(), cost.stop_and_save().cycles());
        buffers->clear();
        zstate->check_health();
    }
    for (size_t loop = 0; loop < 40; loop++) //ϵͳ����̫���� ֻ
    {
        if (loop % 2 == 0 && loop != 0)
        {
            continue;//ϵͳ����̫����  ����Ҫ�����Բ���  ɾ��һ������. 
        }
        PROF_START_COUNTER(cost);
        for (u64 i = cover_size / 40 * loop; i < cover_size / 40 * (loop + 1); i++)
        {
            u32 test_size = fixed_size;
            void* p = global_zmalloc(test_size);
            buffers->push_back(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("zmalloc(100) bat", buffers->size(), cost.stop_and_save().cycles());
        zstate->check_health();
        PROF_START_COUNTER(cost);
        for (auto p : *buffers)
        {
            global_zfree(p);
        }
        PROF_OUTPUT_MULTI_COUNT_CPU("zfree(100) bat", buffers->size(), cost.stop_and_save().cycles());
        buffers->clear();
        zstate->check_health();
    }



    LogDebug() << "check health";
    buffers->clear();

    for (size_t loop = 0; loop < 40; loop++)
    {
        for (u64 i = cover_size / 20 * loop; i < cover_size / 20 * (loop + 1); i++)
        {
            u32 test_size = rand_array[i] % (zmalloc::BIG_MAX_REQUEST);
            void* p = global_zmalloc(test_size);
            if (test_size < 64)
            {
                memset(p, 0, test_size);
            }
            else
            {
                *(u64*)p = 0;
                *(((u64*)p) + test_size / 8 - 1) = 0;
            }
            buffers->push_back(p);
        }
        zstate->check_health();
        for (auto p : *buffers)
        {
            global_zfree(p);
        }
        zstate->check_health();
        buffers->clear();
    }
    void* pz = global_zmalloc(0);
    zstate->check_health();
    global_zfree(pz);


    zstate->clear_cache();
    LogDebug() << "check health finish";
    AssertTest(zstate->used_block_count_ + zstate->reserve_block_count_, 0U, "");
    delete[]rand_array;
    return 0;
}



s32 zmalloc_base_test()
{
    PROF_DEFINE_COUNTER(cost);
    PROF_START_COUNTER(cost);
    for (u32 i = 1024; i < 1000*10000; i++)
    {
        u32 align_value = zmalloc_align_third_bit_value(i);
        u32 bit_order = zmalloc_align_third_bit_order(align_value);
        u32 align_id = zmalloc_third_sequence(bit_order, align_value);
        u32 compress_id = zmalloc_third_sequence_compress(align_id);
        u32 resolve = zmalloc_resolve_order_size(compress_id);
        AssertBoolTest(i <= resolve, "");
    }
    PROF_OUTPUT_MULTI_COUNT_CPU("zmalloc third used", 1000 * 10000 - 1024, cost.stop_and_save().cycles());



    return 0;
}




s32 zmalloc_test()
{
    AssertTest(zmalloc_base_test(), 0, " zmalloc_base_test()");
    AssertTest(zmalloc_stress(), 0, " zmalloc_stress()");
    return 0;
}

