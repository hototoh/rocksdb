#ifndef REPLICATION_MASTER_H
#define REPLICATION_MASTER_H

#include <thread>
#include <thrift/server/TThreadPoolServer.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "rocksdb/thrift/gen-cpp/Replication.h"

namespace rocksdb {

class ReplicationHandler : virtual public ReplicationIf {
private:
    DB *_db;

public:
    ReplicationHandler(DB *db);
    ~ReplicationHandler();
    
    void Update(std::string& _return, const int64_t seq);

};

class ReplicationMaster {
private:
    DB *_db;
    int _port;
    apache::thrift::server::TThreadPoolServer *_server;

public:
    ReplicationMaster(DB* db, int port);
    ~ReplicationMaster();

    void Sync();
    void StartReplication();
    Status StopReplication();
};


} // namespace rocksdb

#endif