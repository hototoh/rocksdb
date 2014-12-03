#ifndef REPLICATION_SLAVE_H
#define REPLICATION_SLAVE_H

#include <thread>
#include <memory>

#include <thrift/transport/TTransport.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "rocksdb/thrift/gen-cpp/Replication.h"

namespace rocksdb {

class ReplicationSlave {
private:
    bool _running = true;
    int _port;
    int _sync_interval = 1;
    boost::shared_ptr<apache::thrift::transport::TTransport> _transport;
    std::string _master_host;
    ReplicationClient* _client;
    DB *_db;
    
public:
    ReplicationSlave(DB *db, std::string master_host, int port);
    ~ReplicationSlave();

    void Update() throw ();
    void Sync();
    void StartReplication();
    Status StopReplication();

}; // class ReplicationSlave

}  // namespace rocksdb

#endif
