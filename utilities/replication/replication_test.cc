// 

#include <time.h>
#include <string>
#include <algorithm>
#include <iostream>

#include "util/random.h"
#include "util/testutil.h"
#include "util/testharness.h"
#include "util/auto_roll_logger.h"
#include "rocksdb/db.h"
#include "rocksdb/status.h"
#include "rocksdb/transaction_log.h"

#include "rocksdb/utilities/backupable_db.h"
#include "./gen-cpp/Replication.h"
#include "replication_common.h"
#include "replication_master.h"
#include "replication_slave.h"

namespace rocksdb {

namespace { 

using std::unique_ptr;

class TestEnv : public EnvWrapper {
 public:
  explicit TestEnv(Env* t) : EnvWrapper(t) {}
  
}; // class TestEnv

// utility functions 
// from backupable_db_test.cc
static size_t FillDB(DB *db, int from, int to) {
  size_t bytes_written = 0;
  for (int i = from; i < to; i++) {
    std::string key = "testkey"+std::to_string(i);
    std::string value = "testkey"+std::to_string(i);
    bytes_written += key.size() + value.size();

    ASSERT_OK(db->Put(WriteOptions(), Slice(key), Slice(value)));
  }
  return bytes_written;
}

static void AssertExists(DB* db, int from, int to) {
  for (int i = from; i < to; ++i) {
    std::string key = "testkey" + std::to_string(i);
    std::string value;
    Status s = db->Get(ReadOptions(), Slice(key), &value);
    ASSERT_EQ(value, "testvalue" + std::to_string(i));
  }
}

static void AssertEmpty(DB* db, int from, int to) {
  for (int i = from; i < to; ++i) {
    std::string key = "testkey" + std::to_string(i);
    std::string value = "testvalue" + std::to_string(i);

    Status s = db->Get(ReadOptions(), Slice(key), &value);
    ASSERT_TRUE(s.IsNotFound());
  }
}

class ReplicationDBTest {
 public:
  ReplicationDBTest()
      : 
      master(),
      slave(),
      master_port(8889),
      master_host("localhost") {
      // setup files
    master_db_name = test::TmpDir() + "/replication_master";
    slave_db_name  = test::TmpDir() + "/replication_slave";
    std::cout << "master_db_name:" << master_db_name << std::endl;
    std::cout << "slave_db_name:"  << slave_db_name  << std::endl;
  
    // setup envs
    env = Env::Default();
    //test_master_env.reset(new TestEnv(env));
    //test_slave_env.reset(new TestEnv(env));
    //file_manager.reset(new FileManager(env));

    // setup db options
    master_options.create_if_missing = true;
    master_options.paranoid_checks = true;
    master_options.write_buffer_size = 1 << 17;
    master_options.env = env;
    master_options.wal_dir = master_db_name;
    slave_options.create_if_missing = true;
    slave_options.paranoid_checks = true;
    slave_options.write_buffer_size = 1 << 17;
    slave_options.env = env;
    slave_options.wal_dir = slave_db_name;

    // setup replication db options
    ;;
    
    // setup Log
    CreateLoggerFromOptions(master_db_name, slave_db_name, env,
                            DBOptions(), &logger);

    // delete old files in db
    DestroyDB(master_db_name, Options());
    DestroyDB(slave_db_name, Options());
  }
  
  DB* OpenDB(const Options options,const std::string& dbname) {
    DB* db;
    ASSERT_OK(DB::Open(options, dbname, &db));
    return db;
  }

  void OpenMasterDB() {
    DB* db = OpenDB(master_options, master_db_name);
    std::cout << "**OpenDB()" << std::endl;
    master_db.reset(db);
    std::cout << "**set" << std::endl;
    ASSERT_OK(ReplicationMaster::Open(&master, db, 
                                      ReplicationDBTest::master_port));
    std::cout << "**ASSERT_OKD" << std::endl;
  }

  void CloseMasterDB() {
    master->StopReplication();
    master_db.reset(nullptr);
    master = nullptr;
  }

  void OpenSlaveDB() {
    DB* db = OpenDB(slave_options, slave_db_name);
    slave_db.reset(db);
    ASSERT_OK(ReplicationSlave::Open(&slave, db, ReplicationDBTest::master_host,
                                     ReplicationDBTest::master_port));
  }

  void CloseSlaveDB() {
    ASSERT_OK(slave->StopReplication());
    slave_db.reset(nullptr);
    slave = nullptr;
  }

  // check replication slave db and asserts the existence of
  // [start_exist, end_exist> and not-existence of
  // [end_exist, end>
  // if end == 0, don't check AssertEmpty
  void AssertReplicationConsistency(uint32_t start_exist, uint32_t end_exist,
                                    uint32_t end = 0) {
    OpenSlaveDB();
    AssertExists(slave_db.get(), start_exist, end_exist);
    if (end != 0) {
      AssertEmpty(slave_db.get(), end_exist, end);
    }
    CloseSlaveDB();
  }
  
  void DeleteLogFiles() {
    // I don't know this function well.
  }

  Status SlaveStartReplication(int sync_interval = 1000,
                               bool flush_before_replication = true) {
    return slave->StartReplication(sync_interval, flush_before_replication);
  }

  void MasterStopReplication() {
    master->StopReplication();
  }

  void SlaveStopReplication() {
    slave->StopReplication();
  }

  void DestroyMasterDB() {
    DestroyDB(master_db_name, Options());
  }

  void DestroySlaveDB() {
    DestroyDB(slave_db_name, Options());
  }
  // files
  std::string master_db_name; 
  std::string slave_db_name;
  
  // dbs
  unique_ptr<DB> master_db;
  unique_ptr<DB> slave_db;

  // replication instance
  ReplicationMaster* master;
  ReplicationSlave*  slave;

  // envs
  Env* env;
  unique_ptr<TestEnv> test_master_env;
  unique_ptr<TestEnv> test_slave_env;

  // options
  Options master_options;
  Options slave_options;
  std::shared_ptr<Logger> logger;

  // settings => TODO ReplicationDBOptions
  const int master_port;
  const std::string master_host;
}; // ReplicationDBTest

// open DB, write, replication, write, replication
TEST(ReplicationDBTest, OfflineLocalWriteTest) {
  const int sync_interval = 1000;
  const int key_iteration = 5000;
  const int max_key = key_iteration * 4 + 100;
  const int not_exist_key = key_iteration * 5;
  time_t now = time(NULL);
  Random rnd((uint32_t) now);

  std::cout << "Start Iteration" << std::endl;
  // first iter -- flush before replication
  // second iter -- don't flush before replication
  for (int iter = 0; iter < 2; ++iter) {
    // every iteration --
    // 1. insert new data in master
    // 2. slave starts replication
    // 3. insert new data in master
    // 4. close master and slave
    // 5. compare master and slave db
    for (int i = 0; i < 5; ++i) {
      // in last iteration, put smaller amount of data,
      int fill_up_to = std::min(key_iteration * (i+1), max_key);
      std::cout << "**OpenMasterDB()" << std::endl;
      OpenMasterDB();
      std::cout << "**OpenSlaveDB()" << std::endl;
      OpenSlaveDB();
      // 1.
      std::cout << "**Fill Master DB" << std::endl;
      FillDB(master_db.get(), 0, fill_up_to);
      // 2.
      std::cout << "**SlaveStartReplication" << std::endl;
      ASSERT_OK(SlaveStartReplication(sync_interval, iter == 0));
      // 3.
      std::cout << "**Fill Master DB" << std::endl;
      FillDB(master_db.get(), fill_up_to, max_key);
      // 4. wait a little for sync
      usleep(sync_interval * 10);
      MasterStopReplication();
      SlaveStopReplication();
      CloseSlaveDB();
      CloseMasterDB();
      // 5.
      AssertReplicationConsistency(0, max_key, not_exist_key);
    }
  }
}

} // anon namespace
  
} // namespace rocksdb

int main(int argc, char **argv) {
  return rocksdb::test::RunAllTests();
}
