// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/protorpc/RpcServer.h"

#include "muduo/base/Logging.h"
#include "muduo/net/protorpc/rpc.pb.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

using namespace muduo;
using namespace muduo::net;

RpcServer::RpcServer(EventLoop* loop,
                     const InetAddress& listenAddr)
  : server_(loop, listenAddr, "RpcServer"),
    codec_(std::bind(&RpcServer::onRpcMessage, this, _1, _2, _3))
{
  server_.setConnectionCallback(
      std::bind(&RpcServer::onConnection, _1));
  server_.setMessageCallback(
          std::bind(&RpcCodec::onMessage, &codec_, _1, _2, _3));
}

void RpcServer::registerService(google::protobuf::Service* service)
{
  const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
  services_[desc->full_name()] = service;
}

void RpcServer::start()
{
  server_.start();
}

void RpcServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "RpcServer - " << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
}

void RpcServer::onRpcMessage(const TcpConnectionPtr &conn, const RpcMessagePtr &messagePtr, Timestamp receiveTime) {
    RpcMessage& message = *messagePtr;
    if (message.type() == RESPONSE)
    {
    }
    else if (message.type() == REQUEST)
    {
        // FIXME: extract to a function
        ErrorCode error = WRONG_PROTO;
        if (!services_.empty())
        {
            std::map<std::string, google::protobuf::Service*>::const_iterator it = services_.find(message.service());
            if (it != services_.end())
            {
                google::protobuf::Service* service = it->second;
                assert(service != NULL);
                const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
                const google::protobuf::MethodDescriptor* method
                = desc->FindMethodByName(message.method());
                if (method)
                {
                    std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
                    if (request->ParseFromString(message.request()))
                    {
                        google::protobuf::Message* response = service->GetResponsePrototype(method).New();
                        // response is deleted in doneCallback
                        int64_t id = message.id();
                        service->CallMethod(method, NULL, get_pointer(request), response,
                                            google::protobuf::NewCallback(this, &RpcServer::doneCallback, doneCallbackArgs(response, id, conn)));
                        error = NO_ERROR;
                    }
                    else
                    {
                        error = INVALID_REQUEST;
                    }
                }
                else
                {
                    error = NO_METHOD;
                }
            }
            else
            {
                error = NO_SERVICE;
            }
        }
        else
        {
            error = NO_SERVICE;
        }
        if (error != NO_ERROR)
        {
            RpcMessage response;
            response.set_type(RESPONSE);
            response.set_id(message.id());
            response.set_error(error);
            codec_.send(conn, response);
        }
    }
    else if (message.type() == ERROR)
    {
    }
}

void RpcServer::doneCallback(const doneCallbackArgs args)
{
    std::unique_ptr<google::protobuf::Message> d(args.response);
    RpcMessage message;
    message.set_type(RESPONSE);
    message.set_id(args.id);
    message.set_response(args.response->SerializeAsString()); // FIXME: error check
    codec_.send(args.conn, message);
}

