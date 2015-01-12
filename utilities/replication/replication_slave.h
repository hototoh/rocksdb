#ifndef REPLICATION_SLAVE_H
#define REPLICATION_SLAVE_H

#include <thread>
#include <atomic>
#include <memory>
#include <future>

#include <thrift/transport/TTransport.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "./gen-cpp/Replication.h"

namespace rocksdb {

class ReplicationSlave {
 private:
  DB* db;
  Env* env;
  int port;
  std::string master_host;
  std::atomic_bool running;
  std::future<Status> sync_result;
  boost::shared_ptr<apache::thrift::transport::TTransport> transport;
  ReplicationClient* client;
  const int64_t transfer_data_size; // 2 ** 20

  inline Status DisableFileDeletions(SessionID& session_id);
  inline Status EnableFileDeletions(SessionID& session_id, 
                                   bool force);
  inline Status GetLiveFiles(std::vector<ReplicationFileInfo>& _return,
                             SessionID& session_id, 
                             bool flush_before_replication);

  inline Status GetLiveWALFiles(std::vector<ReplicationFileInfo>& _return,
                                SessionID& session_id);

  inline Status GetFileData(ReplicationFileData & _return,
                            SessionID& session_id,
                            ReplicationFileInfo& file_info,
                            int64_t transfer_data_size);
   
 public:
  static Status Open(ReplicationSlave** slave, DB* _db,
                     const std::string& _master_host,
                     const int _port);

  ReplicationSlave();
  ReplicationSlave(DB* _db, const std::string& _master_host, const int _port);
  ~ReplicationSlave();
  Status PeriodicalSync(int sync_interval);
  Status StopReplication();
  // sync_interval -- millisecond
  Status StartReplication(int sync_interval = 1000,
                          bool flush_before_replication = true); 

 protected:
  Status GetFile(SessionID& session_id, ReplicationFileInfo& file_info,
                 int64_t transfer_data_size);

  Status CreateNewReplication(bool flush_before_replication = true);
  Status PeriodicalUpdate();

}; // class ReplicationSlave

}  // namespace rocksdb



#endif
