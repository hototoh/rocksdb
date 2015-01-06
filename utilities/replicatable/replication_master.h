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
  ReplicationHandler(DB *_db);
  ~ReplicationHandler();
  void Update(std::string& _return, const int64_t _seq);
}; // class ReplicationHandler

class ReplicationMaster {
 private:
  DB *db;
  int port;
  apache::thrift::server::TThreadPoolServer *server;
  
 public:
  static Status Open(ReplicationMaster *master, DB* _db, int _port);
  ReplicationMaster(DB* _db, int _port);
  ~ReplicationMaster();
  void StopReplication();
  
 protected:
  Status StartReplication();
  void PeriodicalSync();
}; // class ReplicationMaster

} // namespace rocksdb

#endif
