// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.
#include <iostream>
#include <thread>

#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TTransportUtils.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "rocksdb/thrift/gen-cpp/Replication.h"

#include "replication_slave.h"

namespace rocksdb {

ReplicationSlave::ReplicationSlave(DB *db, std::string master_host, int port) { 
    _db         = db;
    _port        = port;
    _master_host = master_host;
}

void ReplicationSlave::Update() throw () { 
    try {
        while(_running) {
            std::string data;
            SequenceNumber last_seq = _db->GetLatestSequenceNumber();

            _client->Update(data, last_seq);

            WriteBatch wb = WriteBatch(data);
            Status s = _db->Write(WriteOptions(), &wb);
            std::cout << data << std::endl;
            if(!s.ok()) {
                std::cout << std::endl;
            }
        }
    } catch (ReplicationError& e) {
        if(e.what == 0) {
            _running = false;
            throw e;
        }
    } catch (apache::thrift::TException& tx) {
        std::cout << "ERROR: " << tx.what() << std::endl;
        StopReplication();
    }
}

void ReplicationSlave::Sync() {
    while(_running) {
        Update();
        sleep(_sync_interval);
    }
}

void ReplicationSlave::StartReplication() {
    boost::shared_ptr<apache::thrift::transport::TTransport> socket(new apache::thrift::transport::TSocket(_master_host, _port));
    boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(socket));
    boost::shared_ptr<apache::thrift::protocol::TProtocol>  protocol(new apache::thrift::protocol::TBinaryProtocol(transport));
    _client = new ReplicationClient(protocol);
    _transport = transport;
    _running   = true;

    try {
        transport->open();
        std::thread replication_thread = std::thread(&ReplicationSlave::Sync, std::ref(*this));
        replication_thread.detach();
    } catch (apache::thrift::TException& tx) {
        std::cout << "ERROR: " << tx.what() << std::endl;
    }
}

Status ReplicationSlave::StopReplication() {
    _running = false;
    _transport->close();
    return Status::OK();
}
    
ReplicationSlave::~ReplicationSlave() {
    StopReplication();
}

} // namespace rocksdb