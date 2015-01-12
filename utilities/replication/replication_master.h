#ifndef REPLICATION_MASTER_H
#define REPLICATION_MASTER_H

#include <thread>
#include <atomic>
#include <thrift/server/TThreadPoolServer.h>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "./gen-cpp/replication_types.h"
#include "./gen-cpp/Replication.h"

namespace rocksdb {
  
class ReplicationHandler : virtual public ReplicationIf {
 private:
  // Master database
  DB* db;
  Env* env;

  // During initial copy in replication, we must disable file deletion. 
  // So we manage file delettion lock using this map.
  std::map<SessionID, int64_t> file_deletion_lock_table;

  // Usually unlock file deletion by remote salve node. However, when
  // something wrong happen in slave, file deletion will never be unlocked
  // So, this observe the table and remove expired entry. When the table
  // is empty, it enable file deletion.
  std::thread file_deletion_lock_table_observer;

  std::atomic<bool> stop_replication;
  // XXX
  // 60 * 60 * 5
  const int64_t expired_time = 18000;
  const int64_t copy_file_buffer_size = 1048576; // 2 ** 20

  
  

  bool NewFileDeletionEntry();
  bool UpdateFileDeletionEntry(const SessionID& sessionId);
  bool DeleteFileDeletionEntry(const SessionID& sessionId);

 public:
  ReplicationHandler(DB* _db);
  ~ReplicationHandler();
  void DisableFileDeletions(SessionID& _return);
  bool EnableFileDeletions(const SessionID& sessionId,
                           const bool force);
  void GetLiveFiles(std::vector<ReplicationFileInfo> & _return, 
                    const SessionID& sessionId,
                    const bool flush_before_replication);
  void GetLiveWALFiles(std::vector<ReplicationFileInfo> & _return,
                       const SessionID& sessionId);
  void GetFileData(ReplicationFileData& _return, 
                   const SessionID& sessionId,
                   const ReplicationFileInfo& info,
                   const int64_t transfer_data_size);
  void PeriodicalUpdate(ReplicationFileData& _return,
                        const int64_t seq);

}; // class ReplicationHandler

class ReplicationMaster {
 private:
  DB* db;
  int port;
  apache::thrift::server::TThreadPoolServer* server;  
  
 public:
  static Status Open(ReplicationMaster* master, DB* _db, const int _port);
  ReplicationMaster() {}
  ReplicationMaster(DB* _db, int _port);
  ~ReplicationMaster();
  void StopReplication();
  
 protected:
  Status StartReplication();
  void PeriodicalSync();
}; // class ReplicationMaster

} // namespace rocksdb

#endif
