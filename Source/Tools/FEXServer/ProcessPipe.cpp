// SPDX-License-Identifier: MIT
#include "CoreDumpService.h"
#include "FEXHeaderUtils/Syscalls.h"
#include "Logger.h"
#include "SocketConnectionHandler.h"
#include "SquashFS.h"

#include "Common/FEXServerClient.h"
#include "Common/SocketUtil.h"

#include <atomic>
#include <fcntl.h>
#include <filesystem>
#include <poll.h>
#include <string>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

namespace ProcessPipe {
  constexpr int USER_PERMS = S_IRWXU | S_IRWXG | S_IRWXO;
  int ServerLockFD {-1};
  int ServerSocketFD{-1};
  std::atomic<bool> ShouldShutdown {false};
  time_t RequestTimeout {10};
  bool Foreground {false};
  std::vector<struct pollfd> PollFDs{};

  // FD count watching
  constexpr size_t static MAX_FD_DISTANCE = 32;
  rlimit MaxFDs{};
  std::atomic<size_t> NumFilesOpened{};

  size_t GetNumFilesOpen() {
    // Walk /proc/self/fd/ to see how many open files we currently have
    const std::filesystem::path self{"/proc/self/fd/"};

    return std::distance(std::filesystem::directory_iterator{self}, std::filesystem::directory_iterator{});
  }

  void GetMaxFDs() {
    // Get our kernel limit for the number of open files
    if (getrlimit(RLIMIT_NOFILE, &MaxFDs) != 0) {
      fprintf(stderr, "[FEXMountDaemon] getrlimit(RLIMIT_NOFILE) returned error %d %s\n", errno, strerror(errno));
    }

    // Walk /proc/self/fd/ to see how many open files we currently have
    NumFilesOpened = GetNumFilesOpen();
  }

  void CheckRaiseFDLimit() {
    if (NumFilesOpened < (MaxFDs.rlim_cur - MAX_FD_DISTANCE)) {
      // No need to raise the limit.
      return;
    }

    if (MaxFDs.rlim_cur == MaxFDs.rlim_max) {
      fprintf(stderr, "[FEXMountDaemon] Our open FD limit is already set to max and we are wanting to increase it\n");
      fprintf(stderr, "[FEXMountDaemon] FEXMountDaemon will now no longer be able to track new instances of FEX\n");
      fprintf(stderr, "[FEXMountDaemon] Current limit is %zd(hard %zd) FDs and we are at %zd\n", MaxFDs.rlim_cur, MaxFDs.rlim_max, GetNumFilesOpen());
      fprintf(stderr, "[FEXMountDaemon] Ask your administrator to raise your kernel's hard limit on open FDs\n");
      return;
    }

    rlimit NewLimit = MaxFDs;

    // Just multiply by two
    NewLimit.rlim_cur <<= 1;

    // Now limit to the hard max
    NewLimit.rlim_cur = std::min(NewLimit.rlim_cur, NewLimit.rlim_max);

    if (setrlimit(RLIMIT_NOFILE, &NewLimit) != 0) {
      fprintf(stderr, "[FEXMountDaemon] Couldn't raise FD limit to %zd even though our hard limit is %zd\n", NewLimit.rlim_cur, NewLimit.rlim_max);
    }
    else {
      // Set the new limit
      MaxFDs = NewLimit;
    }
  }

  bool InitializeServerPipe() {
    auto ServerFolder = FEXServerClient::GetServerLockFolder();

    std::error_code ec{};
    if (!std::filesystem::exists(ServerFolder, ec)) {
      // Doesn't exist, create the the folder as a user convenience
      if (!std::filesystem::create_directories(ServerFolder, ec)) {
        LogMan::Msg::EFmt("Couldn't create server pipe folder at: {}", ServerFolder);
        return false;
      }
    }

    auto ServerLockPath = FEXServerClient::GetServerLockFile();

    // Now this is some tricky locking logic to ensure that we only ever have one server running
    // The logic is as follows:
    // - Try to make the lock file
    // - If Exists then check to see if it is a stale handle
    //   - Stale checking means opening the file that we know exists
    //   - Then we try getting a write lock
    //   - If we fail to get the write lock, then leave
    //   - Otherwise continue down the codepath and degrade to read lock
    // - Else try to acquire a write lock to ensure only one FEXServer exists
    //
    // - Once a write lock is acquired, downgrade it to a read lock
    //   - This ensures that future FEXServers won't race to create multiple read locks
    int Ret = open(ServerLockPath.c_str(), O_RDWR | O_CREAT | O_CLOEXEC | O_EXCL, USER_PERMS);
    ServerLockFD = Ret;

    if (Ret == -1 && errno == EEXIST) {
      // If the lock exists then it might be a stale connection.
      // Check the lock status to see if another process is still alive.
      ServerLockFD = open(ServerLockPath.c_str(), O_RDWR | O_CLOEXEC, USER_PERMS);
      if (ServerLockFD != -1) {
        // Now that we have opened the file, try to get a write lock.
        flock lk {
          .l_type = F_WRLCK,
          .l_whence = SEEK_SET,
          .l_start = 0,
          .l_len = 0,
        };
        Ret = fcntl(ServerLockFD, F_SETLK, &lk);

        if (Ret != -1) {
          // Write lock was gained, we can now continue onward.
        }
        else {
          // We couldn't get a write lock, this means that another process already owns a lock on the lock
          close(ServerLockFD);
          ServerLockFD = -1;
          return false;
        }
      }
      else {
        // File couldn't get opened even though it existed?
        // Must have raced something here.
        return false;
      }
    }
    else if (Ret == -1) {
      // Unhandled error.
      LogMan::Msg::EFmt("Unable to create FEXServer named lock file at: {} {} {}", ServerLockPath, errno, strerror(errno));
      return false;
    }
    else {
      // FIFO file was created. Try to get a write lock
      flock lk {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
      };
      Ret = fcntl(ServerLockFD, F_SETLK, &lk);

      if (Ret == -1) {
        // Couldn't get a write lock, something else must have got it
        close(ServerLockFD);
        ServerLockFD = -1;
        return false;
      }
    }

    // Now that a write lock is held, downgrade it to a read lock
    flock lk {
      .l_type = F_RDLCK,
      .l_whence = SEEK_SET,
      .l_start = 0,
      .l_len = 0,
    };
    Ret = fcntl(ServerLockFD, F_SETLK, &lk);

    if (Ret == -1) {
      // This shouldn't occur
      LogMan::Msg::EFmt("Unable to downgrade a write lock to a read lock {} {} {}", ServerLockPath, errno, strerror(errno));
      close(ServerLockFD);
      ServerLockFD = -1;
      return false;
    }

    return true;
  }

  bool InitializeServerSocket() {
    auto ServerSocketName = FEXServerClient::GetServerSocketName();

    // Create the initial unix socket
    ServerSocketFD = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (ServerSocketFD == -1) {
      LogMan::Msg::EFmt("Couldn't create AF_UNIX socket: {} {}\n", errno, strerror(errno));
      return false;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    size_t SizeOfSocketString = std::min(ServerSocketName.size() + 1, sizeof(addr.sun_path) - 1);
    addr.sun_path[0] = 0; // Abstract AF_UNIX sockets start with \0
    strncpy(addr.sun_path + 1, ServerSocketName.data(), SizeOfSocketString);
    // Include final null character.
    size_t SizeOfAddr = sizeof(addr.sun_family) + SizeOfSocketString;

    // Bind the socket to the path
    int Result = bind(ServerSocketFD, reinterpret_cast<struct sockaddr*>(&addr), SizeOfAddr);
    if (Result == -1) {
      LogMan::Msg::EFmt("Couldn't bind AF_UNIX socket '{}': {} {}\n", addr.sun_path, errno, strerror(errno));
      close(ServerSocketFD);
      ServerSocketFD = -1;
      return false;
    }

    listen(ServerSocketFD, 16);
    PollFDs.emplace_back(pollfd {
      .fd = ServerSocketFD,
      .events = POLLIN,
      .revents = 0,
    });

    return true;
  }

  void SendFDSuccessPacket(int Socket, int FD) {
    FEXServerClient::FEXServerResultPacket Res {
      .Header {
        .Type = FEXServerClient::PacketType::TYPE_SUCCESS,
      },
    };
    FEXServerClient::SendFDPacket(Socket, &Res, sizeof(Res), FD);
  }

  void HandleSocketData(int Socket) {
    fextl::vector<uint8_t> Data(1500);

    // Get the current number of FDs of the process before we start handling sockets.
    GetMaxFDs();

    size_t CurrentRead = SocketUtil::ReadDataFromSocket(Socket, Data);

    size_t CurrentOffset{};
    while (CurrentOffset < CurrentRead) {
      FEXServerClient::FEXServerRequestPacket *Req = reinterpret_cast<FEXServerClient::FEXServerRequestPacket *>(&Data[CurrentOffset]);
      switch (Req->Header.Type) {
        case FEXServerClient::PacketType::TYPE_KILL:
          ShouldShutdown = true;
          CurrentOffset += sizeof(FEXServerClient::FEXServerRequestPacket::BasicRequest);
          break;
        case FEXServerClient::PacketType::TYPE_GET_LOG_FD: {
          if (Logger::LogThreadRunning()) {
            int fds[2]{};
            pipe2(fds, 0);
            // 0 = Read
            // 1 = Write
            Logger::AppendLogFD(fds[0]);

            SendFDSuccessPacket(Socket, fds[1]);

            // Close the write side now, doesn't matter to us
            close(fds[1]);

            // Check if we need to increase the FD limit.
            ++NumFilesOpened;
            CheckRaiseFDLimit();
          }
          else {
            // Log thread isn't running. Let FEXInterpreter know it can't have one.
            FEXServerClient::SendEmptyErrorPacket(Socket);
          }

          CurrentOffset += sizeof(FEXServerClient::FEXServerRequestPacket::Header);
          break;
        }
        case FEXServerClient::PacketType::TYPE_GET_ROOTFS_PATH: {
          fextl::string MountFolder = SquashFS::GetMountFolder();

          FEXServerClient::FEXServerResultPacket Res {
            .MountPath {
              .Header {
                .Type = FEXServerClient::PacketType::TYPE_GET_ROOTFS_PATH,
              },
              .Length = MountFolder.size() + 1,
            },
          };

          char Null{};

          iovec iov[3] {
            {
              .iov_base = &Res,
              .iov_len = sizeof(Res),
            },
            {
              .iov_base = MountFolder.data(),
              .iov_len = MountFolder.size(),
            },
            {
              .iov_base = &Null,
              .iov_len = 1,
            },
          };

          struct msghdr msg {
            .msg_name = nullptr,
            .msg_namelen = 0,
            .msg_iov = iov,
            .msg_iovlen = 3,
          };

          sendmsg(Socket, &msg, 0);

          CurrentOffset += sizeof(FEXServerClient::FEXServerRequestPacket::BasicRequest);
          break;
        }
        case FEXServerClient::PacketType::TYPE_GET_PID_FD: {
          int FD = FHU::Syscalls::pidfd_open(::getpid(), 0);

          if (FD < 0) {
            // Couldn't get PIDFD due to too old of kernel.
            // Return a pipe to track the same information.
            //
            int fds[2];
            pipe2(fds, O_CLOEXEC);
            SendFDSuccessPacket(Socket, fds[0]);

            // Close the read side now, doesn't matter to us
            close(fds[0]);

            // Check if we need to increase the FD limit.
            ++NumFilesOpened;
            CheckRaiseFDLimit();

            // Write side will naturally close on process exit, letting the other process know we have exited.
          }
          else {
            SendFDSuccessPacket(Socket, FD);

            // Close the FD now since we've sent it
            close(FD);
          }

          CurrentOffset += sizeof(FEXServerClient::FEXServerRequestPacket::Header);
          break;
        }
        case FEXServerClient::PacketType::TYPE_GET_COREDUMP_FD: {
          // Coredump FD only should be returned if FEXServer is running in foreground mode.
          // Otherwise the logs won't be visible regardless of receiving them.
          if (Foreground) {
            auto Result = CoreDumpService::CreateCoreDumpService();
            SendFDSuccessPacket(Socket, Result);

            // Close the sent socket now that it was passed over.
            CoreDumpService::ShutdownFD(Result);

            // Check if we need to increase the FD limit.
            ++NumFilesOpened;
            CheckRaiseFDLimit();
          }
          else {
            // Log thread isn't running. Let FEXInterpreter know it can't have one.
            FEXServerClient::SendEmptyErrorPacket(Socket);
          }

          CurrentOffset += sizeof(FEXServerClient::FEXServerRequestPacket::Header);

          break;
        }
          // Invalid
        case FEXServerClient::PacketType::TYPE_ERROR:
        default:
          // Something sent us an invalid packet. To ensure we don't spin infinitely, consume all the data.
          LogMan::Msg::EFmt("[FEXServer] InvalidPacket size received 0x{:x} bytes", CurrentRead - CurrentOffset);
          CurrentOffset = CurrentRead;
          break;
      }
    }
  }

  void CloseConnections() {
    // Close the server pipe so new processes will know to spin up a new FEXServer.
    // This one is closing
    close (ServerLockFD);

    // Close the server socket so no more connections can be started
    close(ServerSocketFD);
  }

  class ProcessPipeHandler final : public FEXServer::SocketConnectionHandler {
    public:
      ProcessPipeHandler(int ServerSocketFD) {
        PollFDs.push_back(pollfd {
          .fd = ServerSocketFD,
          .events = POLLIN | POLLHUP | POLLERR | POLLNVAL | POLLREMOVE | POLLRDHUP,
          .revents = 0,
        });
      }
    protected:
      void OnShutdown() override {
        CloseConnections();
      }

      void CheckShouldShutdownHandler(std::chrono::seconds TimeDuration) override;

      FDEventResult HandleFDEvent(struct pollfd &Event) override;
  };

  FEXServer::SocketConnectionHandler::FDEventResult ProcessPipeHandler::HandleFDEvent(struct pollfd &Event) {
    if (Event.fd == ServerSocketFD) {
      if (Event.revents & POLLIN) {
        // If it is the listen socket then we have a new connection
        struct sockaddr_storage Addr{};
        socklen_t AddrSize{};
        int NewFD = accept(ServerSocketFD, reinterpret_cast<struct sockaddr*>(&Addr), &AddrSize);

        // Track the new FD
        AppendFDToTrack(NewFD);
      }
    }
    else {
      if (Event.revents & POLLIN) {
        // Data from the socket
        HandleSocketData(Event.fd);
      }

      if (Event.revents & (POLLHUP | POLLERR | POLLNVAL | POLLREMOVE | POLLRDHUP)) {
        // Error or hangup, close the socket and erase it from our list
        return FEXServer::SocketConnectionHandler::FDEventResult::ERASE;
      }
    }

    return FEXServer::SocketConnectionHandler::FDEventResult::SUCCESS;
  }

  void ProcessPipeHandler::CheckShouldShutdownHandler(std::chrono::seconds TimeDuration) {
    if (TimeDuration >= std::chrono::duration_cast<std::chrono::seconds>(ExecutionTimeout) &&
        !Foreground &&
        PollFDs.size() == 1) {
      // If we aren't running in the foreground and we have no connections after a timeout
      // Then we can just go ahead and leave
      RequestShutdown();
      LogMan::Msg::DFmt("[FEXServer] Shutting Down");
    }
  }

  void WaitForRequests() {
    ProcessPipeHandler Handler(ServerSocketFD);
    Handler.RunUntilShutdown(std::chrono::seconds(RequestTimeout), RequestTimeout);
  }

  void SetConfiguration(bool Foreground, uint32_t PersistentTimeout) {
    ProcessPipe::Foreground = Foreground;
    ProcessPipe::RequestTimeout = PersistentTimeout;
  }

  void Shutdown() {
    ShouldShutdown = true;
  }
}


