#include "memory_manager.hpp"

void Mem_mgr::registor(void* *data, int size){
    if(Storage_write + size > STORAGE_SIZE){
        *data = nullptr;
        return;
    };
    *data = (void *)&Storage[Storage_write];
    Storage_write += size;
    //registor
    //list[list_size] = {*data, size};
    //list_size++;
};

void Mem_mgr::fresh() {
    Storage_write = 0;
};

//=========================

Msml::Msml() {
    Data_Struct[0] = STORAGE_SIZE / BLK_SIZE;
    Data_Struct[1] = 0;
};

int Msml::addr(int sec_id, unsigned int blk_offset) {
    int addr = 0;
    for (int i = 0; i < sec_id; i++) {
        addr += Data_Struct[i];
    }
    addr += blk_offset;
    return addr * BLK_SIZE;
}

int Msml::search(int blks) {
    int id = 2;
    while (Data_Struct[id]) {
        if (Data_Struct[id] > blks) {
            return id;
        }
        id += 2;
    }
    if (Data_Struct[0] > blks) return 0;
    return -1;
}

bool Msml::extend(int sign, int size) {
    if (Data_Struct[sign + 1] < size) return false;
    Data_Struct[sign] += size;
    Data_Struct[sign + 1] -= size;
    return true;
}

bool Msml::compress(int sign, int size) {
    if (Data_Struct[sign] < size) return false;
    Data_Struct[sign] -= size;
    Data_Struct[sign + 1] += size;
    return true;
}

bool Msml::truncate(int sign, unsigned int offset, unsigned int size) {
    SINE << "truncate start\n";
    SINE << "sec_id: " << sign << " | blk: " << offset << " | free_blks: " << size << "\n";
    if (Data_Struct[sign] < offset + size) return false;
    SINE << "Could be truncate\n";
    int i = 0;
    for (i; Data_Struct[i]; i++);
    SINE << "i: " << i << "\n";
    for (int id = i - 1; id > sign; id--) {
        Data_Struct[id + 2] = Data_Struct[id];
        SINE << "Data_Struct[id + 2]: " << Data_Struct[id + 2] << "\n";
    }
    
    Data_Struct[sign + 2] = Data_Struct[sign] - offset - size;
    Data_Struct[sign + 1] = size;
    Data_Struct[sign] = offset;
    
    Data_Struct[i + 2] = 0;
    return true;
}

void Msml::del(int sign) {
    Data_Struct[sign - 1] += Data_Struct[sign] + Data_Struct[sign + 1];
    int i = sign;
    for (i; Data_Struct[i]; i++) {
        Data_Struct[i] = Data_Struct[i + 2];
    }
    Data_Struct[i] = 0;
};

bool MemMgr_Blk::need(void* *data, int size) {
    unsigned int needed_blks = (size + BLK_SIZE - 1) / BLK_SIZE;
    int sec_id = map.search(needed_blks);

    SINE << sec_id << "\n";

    if (!sec_id) {
        map.truncate(sec_id, PROTECT_SECTION, needed_blks);
        *data = (void *)(map.addr(sec_id + 1) + basic_addr);
        return true;
    } else if (sec_id == -1) {
        *data = nullptr;
        return false;
    }
    
    *data = (void *)(map.addr(sec_id) + basic_addr);
    map.extend(sec_id - 1, needed_blks);
    return true;
}

bool MemMgr_Blk::free(void* *data, int size) {
    unsigned int free_blks = (size + BLK_SIZE - 1) / BLK_SIZE;
#ifdef DB
    long long int da = (long long )data;
#define DA <<" | data: "<<da
#else
#define DA
#endif
    long long blk= (long long)data - basic_addr;
    SINE<<"\n\n===\n\n"<<"free_blks: "<<free_blks DA<<" | basic_addr: "<<basic_addr<<" | blk: "<<blk<<"\n\n\n===\n\n";
    if (blk % BLK_SIZE || blk < 0) return false;
    blk = blk / BLK_SIZE;
    SINE<<"blk: "<<blk<<"\n";

    int sec_id = 0;
    while (blk - map.Data_Struct[sec_id] >= 0){
        SINE<<"( "<<blk<<","<<map.Data_Struct[sec_id]<<" )\n";
        blk -= map.Data_Struct[sec_id];
        sec_id++;
    }
    SINE<<" ->  "<<sec_id<<"\n";
    SINE<<"\n\n===\n\n"<<"free_blks: "<<free_blks<<" | sec_id: "<<sec_id<<" | blk: "<<blk<<" | map.Data_Struct[sec_id]: "<<map.Data_Struct[sec_id]<<"\n\n\n===\n\n";
    if((unsigned int )blk+free_blks > map.Data_Struct[sec_id])return false;

    if(blk){
        if(!(sec_id & 1))return false;
        goto negative_lable;
    }else{
        goto null_lable;
    }

    return false;
    
null_lable:
    SINE << "free_blks: " << free_blks << " | blk: " << blk << " | sec_id: " << sec_id  <<"\n\n";
    if (map.Data_Struct[sec_id] == free_blks) {
        SINE << ">>>null_lable: del<<<\n";
        map.del(sec_id); //整段删除
        *data = nullptr;
        return true;
    }
    SINE << ">>>null_lable<<<\n";
    map.extend(sec_id - 1, free_blks);
    *data = nullptr;
    return true; //前段扩张

negative_lable:
    SINE << "free_blks: " << free_blks << " | blk: " << blk << " | sec_id: " << sec_id  <<"\n\n";
    if (map.Data_Struct[sec_id] == free_blks + (unsigned int )blk) {
        SINE << ">>>negtive_lable: compress<<<\n";
        map.compress(sec_id, free_blks);
        *data = nullptr;
        return true; //段尾压缩
    }
    SINE << ">>>negtive_lable<<<\n";
    if (!map.truncate(sec_id, blk, free_blks)) return false;
    *data = nullptr;
    return true; //段中释放
}

//~~~
Mem_mgr BDS;
MemMgr_Blk mm;