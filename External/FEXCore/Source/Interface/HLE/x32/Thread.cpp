#include "Interface/Context/Context.h"
#include "Interface/Core/InternalThreadState.h"
#include "Interface/HLE/Syscalls.h"
#include "Interface/HLE/x32/Thread.h"


#include <asm/ldt.h>
#include <stdint.h>

namespace FEXCore::HLE::x32 {
  uint32_t Set_thread_area(FEXCore::Core::InternalThreadState *Thread, struct user_desc *u_info) {
    LogMan::Msg::D("Set_thread_area");
    LogMan::Msg::D("\tentry_number:    %d", u_info->entry_number);
    LogMan::Msg::D("\tbase_addr:       0x%x", u_info->base_addr);
    LogMan::Msg::D("\tlimit:           0x%x", u_info->limit);
    LogMan::Msg::D("\tseg_32bit:       %d", u_info->seg_32bit);
    LogMan::Msg::D("\tcontents:        %d", u_info->contents);
    LogMan::Msg::D("\tread_exec_only:  %d", u_info->read_exec_only);
    LogMan::Msg::D("\tlimit_in_pages:  %d", u_info->limit_in_pages);
    LogMan::Msg::D("\tseg_not_present: %d", u_info->seg_not_present);
    LogMan::Msg::D("\tuseable:         %d", u_info->useable);

    static bool Initialized = false;
    if (Initialized == true) {
      LogMan::Msg::A("Trying to load a new GDT");
    }
    if (u_info->entry_number == -1) {
      u_info->entry_number = 12; // Sure?
      Initialized = true;
    }
    // Now we need to update the thread's GDT to handle this change
    auto GDT = &Thread->State.State.gdt[u_info->entry_number];
    GDT->base = u_info->base_addr;
    return 0;
  }
}
