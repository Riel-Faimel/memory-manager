#define STORAGE_SIZE 0x4000000
#define BLK_SIZE 0x40
#define LIST_NUM STORAGE_SIZE / BLK_SIZE
#define PROTECT_SECTION 0x10000

#ifdef _WIN32
    #define BASE_PTR 0x0000000100000000
#elif __linux__
    #define BASE_PTR 0x0000000000200000
#elif __APPLE__
    #define BASE_PTR 0x0000000100000000
#else
    #define BASE_PTR 0x0000000000010000
#endif

#ifdef DB
#include <iostream>
using namespace std;

class Sine {
public:
    Sine() {};
    
    template<typename T>
    Sine &operator<<(const T &value) {}
};
#define SINE Sine()
#else
class Null {
public:
    Null(){};
    
    template<typename T>
    Null &operator<<(T &value) {
        return *this;
    };
};
#define SINE Null()
#endif

//~~~
class MemMgr_Blk;
class Mem_mgr;
class Msml;
//~~~

class Mem_mgr{
private:
    char Storage[STORAGE_SIZE]; //.bss 64MB
    int Storage_write; //.data 8
public:
    Mem_mgr(){};
    
    void registor(void* *data, int size);
    void fresh();
};

class Msml {
#ifdef DB
public:
#endif
#ifndef DB
private:
#endif
    unsigned int Data_Struct[LIST_NUM];
#ifndef DB
    friend class MemMgr_Blk;
#endif
public:
    Msml();

    int addr(int sec_id, unsigned int blk_offset = 0);
    int search(int blks);
    bool extend(int sign, int size);
    bool compress(int sign, int size);
    bool truncate(int sign, unsigned int offset, unsigned int size);
    void del(int sign);
};

class MemMgr_Blk {
#ifdef DB
public:
#endif
#ifndef DB
private:
#endif
    char BlkStor[STORAGE_SIZE];
    Msml map;
    const long long basic_addr = (long long)BlkStor;

public:
    MemMgr_Blk() {};

    bool need(void* *data, int size = 1024);
    bool free(void* *data, int size = 1024);
};

//~~~
extern Mem_mgr BDS; //basic data space
extern MemMgr_Blk mm;