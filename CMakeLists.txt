# Copyright 2021 the gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ afs example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building afs.

cmake_minimum_required(VERSION 3.5.1)

project(afs C CXX)

include(../wi-AFS/common.cmake)

# Proto file
get_filename_component(afs_proto "protos/afs.proto" ABSOLUTE)
get_filename_component(afs_proto_path "${afs_proto}" PATH)

# Generated sources
set(afs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/afs.pb.cc")
set(afs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/afs.pb.h")
set(afs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/afs.grpc.pb.cc")
set(afs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/afs.grpc.pb.h")
add_custom_command(
      OUTPUT "${afs_proto_srcs}" "${afs_proto_hdrs}" "${afs_grpc_srcs}" "${afs_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${afs_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${afs_proto}"
      DEPENDS "${afs_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("/users/monshiva/fuse-3.10.5/include")
# afs_grpc_proto
add_library(afs_grpc_proto
  ${afs_grpc_srcs}
  ${afs_grpc_hdrs}
  ${afs_proto_srcs}
  ${afs_proto_hdrs})
target_link_libraries(afs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# client
add_executable(client "client.cc")
target_link_libraries(client
  "/users/monshiva/fuse-3.10.5/build/lib/libfuse3.so"	
  afs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# server
add_executable(server "server.cc")
target_link_libraries(server
  afs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
