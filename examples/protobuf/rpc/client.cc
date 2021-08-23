#include "examples/protobuf/rpc/sudoku.pb.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/protorpc/RpcChannel.h"

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void solved(sudoku::SudokuResponse* resp)
{
    LOG_INFO << "solved:\n" << resp->DebugString();
}

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    EventLoopThread loopThread;
    InetAddress serverAddr(argv[1], 9981);

    sudoku::SudokuRequest request;
    request.set_checkerboard("001010");
    sudoku::SudokuResponse* response = new sudoku::SudokuResponse;

    RpcChannelPtr channel(new RpcChannel(loopThread.startLoop(), serverAddr));
    sudoku::SudokuService_Stub stub(channel.get());
    stub.Solve(NULL, &request, response, NewCallback(solved, response));

    CurrentThread::sleepUsec(1000*1000);
  }
  else
  {
    printf("Usage: %s host_ip\n", argv[0]);
  }
  google::protobuf::ShutdownProtobufLibrary();
}

