namespace cpp rocksdb

exception ReplicationError {
  1: i32 what,
  2: string why
}

service Replication {
     binary Update(1: i64 seq) throws (1:ReplicationError ouch)
}

