#include <iostream>
#include <sstream>
#include <random>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "replication_master.h"

using namespace rocksdb;

    
void dump(DB *master) {
    Iterator *iter = master->NewIterator(ReadOptions());
    std::cout << "dump:" << std::endl;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next())
        std::cout << iter->key().ToString() << ":" << iter->value().ToString() << std::endl;
}

int main(int argc, char **argv) {
    int port = 9090;
    DB *master;
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;
    options.WAL_ttl_seconds = 100000;
    std::string kDBPath = "/tmp/rocksdb_custom_master";
    Status st = DB::Open(options, kDBPath, &master);
    assert(st.ok());


    std::cout << "last seq:" <<   master->GetLatestSequenceNumber() << std::endl; 
    std::random_device seed_gen;
    std::minstd_rand engine(seed_gen());
    SequenceNumber seq = master->GetLatestSequenceNumber();
    std::ostringstream oss, oss_;
    oss << "key" << seq++;
    oss_ << "value" << engine();
    st = master->Put(WriteOptions(), oss.str(), oss_.str());
    assert(st.ok());
    oss.str("");
    oss_.str("");
    oss << "key" << seq++;
    oss_ << "value" << engine();
    st = master->Put(WriteOptions(), oss.str(), oss_.str());
    assert(st.ok());
    oss.str("");
    oss_.str("");
    oss << "key" << seq++;
    oss_ << "value" << engine();
    st = master->Put(WriteOptions(), oss.str(), oss_.str());
    assert(st.ok());
    oss.str("");
    oss_.str("");

    dump(master);

    ReplicationMaster rep(master, port);
    rep.StartReplication();
    while(1) {
        oss.str("");
        oss_.str("");
        oss << "key" << seq++;
        oss_ << "value" << engine();
        st = master->Put(WriteOptions(), oss.str(), oss_.str());

        assert(st.ok());
        sleep(60);
    }
    rep.StopReplication();
    
    return 0;
}

