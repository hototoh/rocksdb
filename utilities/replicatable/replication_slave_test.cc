#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "replication_slave.h"

using namespace rocksdb;

int main(int argc, char **argv) {
    DB *_slave;
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;
    std::string kDBPath = "/tmp/rocksdb_custom_slave1";
    Status st = DB::Open(options, kDBPath, &_slave);

    ReplicationSlave slave(_slave, "localhost", 9090);
    std::cout << "start replication" << std::endl;
    slave.StartReplication();
    sleep(10);
    
    return 0;
}

