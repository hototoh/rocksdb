#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#
AUTOMAKE_OPTIONS = subdir-objects
ROCKSDB_DIR = /Users/tkk/dev/rocksdb
ETC_FLAGS   = -std=c++11 
MACRO_FLAGS = -DROCKSDB_PLATFORM_POSIX  -DOS_MACOSX -DROCKSDB_ATOMIC_PRESENT -DSNAPPY -DZLIB -DBZIP2
LINK_FLAGS  = -lsnappy -lz -lbz2 -lthrift
HEAD_FLAGS  = -Igen-cpp -I/usr/local/include -I/opt/local/include -I$(ROCKSDB_DIR)/include -I$(top_builddir)/lib/cpp/src -I$(ROCKSDB_DIR)
LIB_FLAGS   = -L/opt/local/lib -L/usr/local/lib

AM_CPPFLAGS = $(ETC_FLAGS) $(MACRO_FLAGS) $(HEAD_FLAGS) $(LIB_FLAGS) $(BOOST_CPPFLAGS) $(LIBEVENT_CPPFLAGS) 
AM_CXXFLAGS = -Wall -I/Users/tkk/dev/rocksdb/include
AM_LDFLAGS = $(BOOST_LDFLAGS) $(LIBEVENT_LDFLAGS) $(LIB_FLAGS) $(LINK_FLAGS) $(HEAD_FLAGS) $(ROCKSDB_DIR)/librocksdb.a

.NOTPARALLEL:
noinst_LTLIBRARIES = librepgencpp.la
librepgencpp_la_SOURCES = \
	gen-cpp/replication_constants.cpp \
	gen-cpp/replication_types.cpp \
	gen-cpp/Replication.cpp 

librepgencpp_la_LDFLAGS = -lthrift

bin_PROGRAMS = \
	repmaster \
	repslave \
	replicationtest

repmaster_CXXFLAGS = -c
repmaster_SOURCES = replication_master.cc 
repslave_CXXFLAGS = -c
repslave_SOURCES = replication_slave.cc

replicationtest_SOURCES = \
	replication_master.cc \
	replication_slave.cc \
	replication_test.cc \
	$(ROCKSDB_DIR)/librocksdb.a

replicationtest_LDADD = \
	librepgencpp.la 

# Common thrift code generation rules
#
THRIFT = /usr/local/bin/thrift

#gen-cpp/Cluster.cpp gen-cpp/replication_types.cpp gen-cpp/replication_constants.cpp: $(top_srcdir)/tutorial/tutorial.thrift
#	$(THRIFT) --gen cpp -r $<



runm: all
	./masterdb

runs: all
	./slavedb

runrs: all
	./repserver

runrc: all
	./repclient

EXTRA_DIST = \
	ReplicationServer.cc \
	ReplicationClient.cc
