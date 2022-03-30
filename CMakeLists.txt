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
# cmake build file for C++ wifs example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building wifs.

cmake_minimum_required(VERSION 3.5.1)

project(wifs C CXX)

include(../Strongly-Consistent-FS/common.cmake)

# Proto file
get_filename_component(wifs_proto "protos/wifs.proto" ABSOLUTE)
get_filename_component(wifs_proto_path "${wifs_proto}" PATH)
get_filename_component(primarybackup_proto "protos/primarybackup.proto" ABSOLUTE)
get_filename_component(primarybackup_proto_path "${primarybackup_proto}" PATH)

# Generated sources
set(wifs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/wifs.pb.cc")
set(wifs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/wifs.pb.h")
set(wifs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/wifs.grpc.pb.cc")
set(wifs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/wifs.grpc.pb.h")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_custom_command(
      OUTPUT "${wifs_proto_srcs}" "${wifs_proto_hdrs}" "${wifs_grpc_srcs}" "${wifs_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${wifs_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${wifs_proto}"
      DEPENDS "${wifs_proto}")

set(primarybackup_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/primarybackup.pb.cc")
set(primarybackup_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/primarybackup.pb.h")
set(primarybackup_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/primarybackup.grpc.pb.cc")
set(primarybackup_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/primarybackup.grpc.pb.h")
add_custom_command(
      OUTPUT "${primarybackup_proto_srcs}" "${primarybackup_proto_hdrs}" "${primarybackup_grpc_srcs}" "${primarybackup_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${primarybackup_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${primarybackup_proto}"
      DEPENDS "${primarybackup_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
# wifs_grpc_proto
add_library(wifs_grpc_proto
  ${wifs_grpc_srcs}
  ${wifs_grpc_hdrs}
  ${wifs_proto_srcs}
  ${wifs_proto_hdrs})
target_link_libraries(wifs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

add_library(primarybackup_grpc_proto
  ${primarybackup_grpc_srcs}
  ${primarybackup_grpc_hdrs}
  ${primarybackup_proto_srcs}
  ${primarybackup_proto_hdrs})
target_link_libraries(primarybackup_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

find_library(PTHREAD_LIB  NAMES pthread)

# client
add_library(client SHARED "client.cc")
target_link_libraries(client
  ${PTHREAD_LIB}
  wifs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
set_property(TARGET client PROPERTY POSITION_INDEPENDENT_CODE ON)

# server
add_executable(server "server.cc")
target_link_libraries(server
  ${PTHREAD_LIB}
  wifs_grpc_proto
  primarybackup_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# master
add_executable(master "master.cc")
target_link_libraries(master
  ${PTHREAD_LIB}
  wifs_grpc_proto
  primarybackup_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# client manual, easy for manual testing. spin up server and then client
add_executable(client_manual "client_manual.cc")
target_link_libraries(client_manual
  ${PTHREAD_LIB}
  wifs_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})
