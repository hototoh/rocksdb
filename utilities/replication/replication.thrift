namespace cpp rocksdb

typedef string SessionID

enum ReplicationErrorType {
  // PeriodicalUpdate
  NO_UPDATE = 0,
  INVALID_INDEX = 1, 
  INVALID_ = 2,
  SERVER_ERROR = 3
}

exception _ReplicationError {
  1: ReplicationErrorType what,
  2: string why
}


struct ReplicationFileInfo {
  1: string src_dir,
  2: string src_fname,
  3: string checksum,
  4: i64    size,
  5: i64    position = 0
}

struct ReplicationFileData {
  1: string src_dir,
  2: string src_fname,
  3: binary data,
  4: i64 size
  5: i64 position,
}

service Replication {
  SessionID DisableFileDeletions() throws (1:_ReplicationError ouch),
  bool EnableFileDeletions(1: SessionID sessionId, 2: bool force) throws (1: _ReplicationError ouch),
  list<ReplicationFileInfo> GetLiveFiles(1:SessionID sessionId, 2:bool flush_before_replication) throws (1:_ReplicationError ouch),
  list<ReplicationFileInfo> GetLiveWALFiles(1:SessionID sessionId) throws (1:_ReplicationError ouch),
  ReplicationFileData GetFileData(1:SessionID sessionId, 2:ReplicationFileInfo info, 3:i64 transfer_data_size) throws (1:_ReplicationError ouch),
  ReplicationFileData PeriodicalUpdate(1: i64 seq) throws (1:_ReplicationError ouch)
}
