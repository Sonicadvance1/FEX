/*
$info$
tags: LinuxSyscalls|syscalls-x86-64
$end_info$
*/

#pragma once

#include <FEXCore/Core/SignalDelegator.h>

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <sys/ipc.h>
#include <sys/sem.h>

namespace FEX::HLE::x64 {

struct
__attribute__((annotate("alias-x86_64-ipc_perm")))
//__attribute__((annotate("fex-match")))
ipc_perm_64 {
  uint32_t key;
  uint32_t uid;
  uint32_t gid;
  uint32_t cuid;
  uint32_t cgid;
  uint16_t mode;
  uint16_t _pad1;
  uint16_t seq;
  uint16_t _pad2;
  uint64_t _pad[2];

  ipc_perm_64() = delete;

  operator struct ipc_perm() const {
    struct ipc_perm perm;
    perm.__key = key;
    perm.uid   = uid;
    perm.gid   = gid;
    perm.cuid  = cuid;
    perm.cgid  = cgid;
    perm.mode  = mode;
    perm.__seq = seq;
    return perm;
  }

  ipc_perm_64(struct ipc_perm perm) {
    key  = perm.__key;
    uid  = perm.uid;
    gid  = perm.gid;
    cuid = perm.cuid;
    cgid = perm.cgid;
    mode = perm.mode;
    seq  = perm.__seq;
  }
};

static_assert(std::is_trivial<ipc_perm_64>::value, "Needs to be trivial");
static_assert(sizeof(ipc_perm_64) == 48, "Incorrect size");

struct
__attribute__((annotate("alias-x86_64-semid_ds")))
__attribute__((annotate("fex-match")))
semid_ds_64 {
  struct ipc_perm_64 sem_perm;
  uint64_t sem_otime;
  uint64_t sem_ctime;
  uint64_t sem_nsems;
  uint64_t _pad[2];

  semid_ds_64() = delete;

  operator struct semid_ds() const {
    struct semid_ds buf{};
    buf.sem_perm = sem_perm;

    buf.sem_otime = sem_otime;
    buf.sem_ctime = sem_ctime;
    buf.sem_nsems = sem_nsems;

    // sem_base, sem_pending, sem_pending_last, undo doesn't exist in the definition
    // Kernel doesn't return anything in them
    return buf;
  }

  semid_ds_64(struct semid_ds buf)
    : sem_perm {buf.sem_perm} {
    sem_otime = buf.sem_otime;
    sem_ctime = buf.sem_ctime;
    sem_nsems = buf.sem_nsems;
  }
};

static_assert(std::is_trivial<semid_ds_64>::value, "Needs to be trivial");
static_assert(sizeof(semid_ds_64) == 88, "Incorrect size");

union
__attribute__((annotate("alias-x86_64-semun")))
__attribute__((annotate("fex-match")))
semun {
  int val;			/* value for SETVAL */
  struct semid_ds *buf;	/* buffer for IPC_STAT & IPC_SET */
  unsigned short *array;	/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
  void *__pad;
};

}
