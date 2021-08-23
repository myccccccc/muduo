// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_PROTORPC_RPCSERVER_H
#define MUDUO_NET_PROTORPC_RPCSERVER_H

#include "muduo/net/TcpServer.h"
#include "muduo/net/protorpc/RpcCodec.h"

namespace google {
namespace protobuf {

class Service;

}  // namespace protobuf
}  // namespace google

namespace muduo
{
namespace net
{

class RpcServer
{
 public:
  RpcServer(EventLoop* loop,
            const InetAddress& listenAddr);

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void registerService(::google::protobuf::Service*);
  void start();

 private:
    struct doneCallbackArgs
    {
        doneCallbackArgs(::google::protobuf::Message* r,
                         int64_t i, const TcpConnectionPtr c)
        : response(r), id(i), conn(c) {}

        ::google::protobuf::Message* response;
        int64_t id;
        const TcpConnectionPtr conn;
    };

  static void onConnection(const TcpConnectionPtr& conn);

  void onRpcMessage(const TcpConnectionPtr& conn,
                    const RpcMessagePtr& messagePtr,
                    Timestamp receiveTime);

  void doneCallback(const doneCallbackArgs args);

  TcpServer server_;
  std::map<std::string, ::google::protobuf::Service*> services_;
  RpcCodec codec_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_PROTORPC_RPCSERVER_H
