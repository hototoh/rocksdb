#ifndef REPLICATION_MASTER_H
#define REPLICATION_MASTER_H

#include <thread>
#include <thrift/server/TThreadPoolServer.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "./gen-cpp/Replication.h"

namespace rocksdb {
  
class ReplicationHandler : virtual public ReplicationIf {
 private:
  DB *db;

 public:
  static Status ReplicationHandler(DB *_db);
  ~ReplicationHandler();
  Status Update(std::string& _return, const int64_t _seq);
}; // class ReplicationHandler

class ReplicationMaster {
 private:
  DB *db;
  int port;
  apache::thrift::server::TThreadPoolServer *server;
  
 public:
  static Status ReplicationMaster(DB* _db, int _port);
  ~ReplicationMaster();
  Status PeriodicalSync();
  void StopReplication();
  
 protected:
  Status StartReplication();
}; // class ReplicationMaster

} // namespace rocksdb

#endif
