#include <FEXCore/Utils/CompilerDefs.h>
#include <FEXCore/Utils/AllocatorHooks.h>
#include <FEXCore/Core/SignalDelegator.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/IR/IRDefines.inc>
#include <FEXCore/IR/IR.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/CoreState.h>
#include <FEXCore/fextl/memory_resource.h>
#include <FEXCore/Utils/CPUInfo.h>
#include <FEXCore/Utils/FileLoading.h>
#include <FEXCore/Utils/Telemetry.h>


#ifdef FEXCORE_FORWARDDECL_DEFINES
// Impls from External/FEXCore/include/FEXCore/Utils/CompilerDefs.h
namespace FCL::FEXCore::Assert {
	using ForcedAssert_Type = void (*)();
	const char ForcedAssert_sym_name[] = "_ZN7FEXCore6Assert12ForcedAssertEv";
	extern ForcedAssert_Type ForcedAssert;

}
// Impls from External/FEXCore/include/FEXCore/Utils/AllocatorHooks.h
namespace FCL {
	using je_malloc_Type = void * (*)(size_t);
	const char je_malloc_sym_name[] = "je_malloc";
	extern je_malloc_Type je_malloc;

	using je_calloc_Type = void * (*)(size_t, size_t);
	const char je_calloc_sym_name[] = "je_calloc";
	extern je_calloc_Type je_calloc;

	using je_memalign_Type = void * (*)(size_t, size_t);
	const char je_memalign_sym_name[] = "je_memalign";
	extern je_memalign_Type je_memalign;

	using je_valloc_Type = void * (*)(size_t);
	const char je_valloc_sym_name[] = "je_valloc";
	extern je_valloc_Type je_valloc;

	using je_posix_memalign_Type = int (*)(void **, size_t, size_t);
	const char je_posix_memalign_sym_name[] = "je_posix_memalign";
	extern je_posix_memalign_Type je_posix_memalign;

	using je_realloc_Type = void * (*)(void *, size_t);
	const char je_realloc_sym_name[] = "je_realloc";
	extern je_realloc_Type je_realloc;

	using je_free_Type = void (*)(void *);
	const char je_free_sym_name[] = "je_free";
	extern je_free_Type je_free;

	using je_malloc_usable_size_Type = size_t (*)(void *);
	const char je_malloc_usable_size_sym_name[] = "je_malloc_usable_size";
	extern je_malloc_usable_size_Type je_malloc_usable_size;

	using je_aligned_alloc_Type = void * (*)(size_t, size_t);
	const char je_aligned_alloc_sym_name[] = "je_aligned_alloc";
	extern je_aligned_alloc_Type je_aligned_alloc;

}
namespace FCL::FEXCore::Allocator {
	const char mmap_sym_name[] = "_ZN7FEXCore9Allocator4mmapE";
	extern ::FEXCore::Allocator::MMAP_Hook mmap;

	const char munmap_sym_name[] = "_ZN7FEXCore9Allocator6munmapE";
	extern ::FEXCore::Allocator::MUNMAP_Hook munmap;

}
// Impls from External/FEXCore/include/FEXCore/Core/SignalDelegator.h
namespace FCL::FEXCore {
	using RegisterTLSState_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char RegisterTLSState_sym_name[] = "_ZN7FEXCore15SignalDelegator16RegisterTLSStateEPNS_4Core19InternalThreadStateE";
	extern RegisterTLSState_Type RegisterTLSState;

	using UninstallTLSState_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char UninstallTLSState_sym_name[] = "_ZN7FEXCore15SignalDelegator17UninstallTLSStateEPNS_4Core19InternalThreadStateE";
	extern UninstallTLSState_Type UninstallTLSState;

	using RegisterHostSignalHandler_Type = void (*)(int, ::FEXCore::HostSignalDelegatorFunction, bool);
	const char RegisterHostSignalHandler_sym_name[] = "_ZN7FEXCore15SignalDelegator25RegisterHostSignalHandlerEiSt8functionIFbPNS_4Core19InternalThreadStateEiPvS5_EEb";
	extern RegisterHostSignalHandler_Type RegisterHostSignalHandler;

	using RegisterFrontendHostSignalHandler_Type = void (*)(int, ::FEXCore::HostSignalDelegatorFunction, bool);
	const char RegisterFrontendHostSignalHandler_sym_name[] = "_ZN7FEXCore15SignalDelegator33RegisterFrontendHostSignalHandlerEiSt8functionIFbPNS_4Core19InternalThreadStateEiPvS5_EEb";
	extern RegisterFrontendHostSignalHandler_Type RegisterFrontendHostSignalHandler;

	using RegisterHostSignalHandlerForGuest_Type = void (*)(int, ::FEXCore::HostSignalDelegatorFunctionForGuest);
	const char RegisterHostSignalHandlerForGuest_sym_name[] = "_ZN7FEXCore15SignalDelegator33RegisterHostSignalHandlerForGuestEiSt8functionIFbPNS_4Core19InternalThreadStateEiPvS5_PNS_14GuestSigActionEP7stack_tEE";
	extern RegisterHostSignalHandlerForGuest_Type RegisterHostSignalHandlerForGuest;

	using HandleSignal_Type = void (*)(int, void *, void *);
	const char HandleSignal_sym_name[] = "_ZN7FEXCore15SignalDelegator12HandleSignalEiPvS1_";
	extern HandleSignal_Type HandleSignal;

	using CheckXIDHandler_Type = void (*)();
	const char CheckXIDHandler_sym_name[] = "_ZN7FEXCore15SignalDelegator15CheckXIDHandlerEv";
	extern CheckXIDHandler_Type CheckXIDHandler;

}
// Impls from External/FEXCore/include/FEXCore/Utils/Allocator.h
namespace FCL::FEXCore::Allocator {
	using SetupHooks_Type = void (*)();
	const char SetupHooks_sym_name[] = "_ZN7FEXCore9Allocator10SetupHooksEv";
	extern SetupHooks_Type SetupHooks;

	using ClearHooks_Type = void (*)();
	const char ClearHooks_sym_name[] = "_ZN7FEXCore9Allocator10ClearHooksEv";
	extern ClearHooks_Type ClearHooks;

	using DetermineVASize_Type = size_t (*)();
	const char DetermineVASize_sym_name[] = "_ZN7FEXCore9Allocator15DetermineVASizeEv";
	extern DetermineVASize_Type DetermineVASize;

	using DisableSBRKAllocations_Type = void * (*)();
	const char DisableSBRKAllocations_sym_name[] = "_ZN7FEXCore9Allocator22DisableSBRKAllocationsEv";
	extern DisableSBRKAllocations_Type DisableSBRKAllocations;

	using ReenableSBRKAllocations_Type = void (*)(void *);
	const char ReenableSBRKAllocations_sym_name[] = "_ZN7FEXCore9Allocator23ReenableSBRKAllocationsEPv";
	extern ReenableSBRKAllocations_Type ReenableSBRKAllocations;

	using StealMemoryRegion_Type = ::fextl::vector< ::FEXCore::Allocator::MemoryRegion> (*)(uintptr_t, uintptr_t);
	const char StealMemoryRegion_sym_name[] = "_ZN7FEXCore9Allocator17StealMemoryRegionEmm";
	extern StealMemoryRegion_Type StealMemoryRegion;

	using ReclaimMemoryRegion_Type = void (*)(const fextl::vector< ::FEXCore::Allocator::MemoryRegion> &);
	const char ReclaimMemoryRegion_sym_name[] = "_ZN7FEXCore9Allocator19ReclaimMemoryRegionERKSt6vectorINS0_12MemoryRegionEN5fextl8FEXAllocIS2_EEE";
	extern ReclaimMemoryRegion_Type ReclaimMemoryRegion;

	using Steal48BitVA_Type = ::fextl::vector< ::FEXCore::Allocator::MemoryRegion> (*)();
	const char Steal48BitVA_sym_name[] = "_ZN7FEXCore9Allocator12Steal48BitVAEv";
	extern Steal48BitVA_Type Steal48BitVA;

}
// Impls from External/FEXCore/include/FEXCore/Utils/LogManager.h
namespace FCL::LogMan::Throw {
	using InstallHandler_Type = void (*)(::LogMan::Throw::ThrowHandler);
	const char InstallHandler_sym_name[] = "_ZN6LogMan5Throw14InstallHandlerEPFvPKcE";
	extern InstallHandler_Type InstallHandler;

	using UnInstallHandlers_Type = void (*)();
	const char UnInstallHandlers_sym_name[] = "_ZN6LogMan5Throw17UnInstallHandlersEv";
	extern UnInstallHandlers_Type UnInstallHandlers;

}
namespace FCL::LogMan::Msg {
	using InstallHandler_Type = void (*)(::LogMan::Msg::MsgHandler);
	const char InstallHandler_sym_name[] = "_ZN6LogMan3Msg14InstallHandlerEPFvNS_11DebugLevelsEPKcE";
	extern InstallHandler_Type InstallHandler;

	using UnInstallHandlers_Type = void (*)();
	const char UnInstallHandlers_sym_name[] = "_ZN6LogMan3Msg17UnInstallHandlersEv";
	extern UnInstallHandlers_Type UnInstallHandlers;

	using D_Type = void (*)(const char *);
	const char D_sym_name[] = "_ZN6LogMan3Msg1DEPKcz";
	extern D_Type D;

	using MFmtImpl_Type = void (*)(::LogMan::DebugLevels, const char *, const fmt::format_args &);
	const char MFmtImpl_sym_name[] = "_ZN6LogMan3Msg8MFmtImplENS_11DebugLevelsEPKcRKN3fmt2v917basic_format_argsINS5_20basic_format_contextINS5_8appenderEcEEEE";
	extern MFmtImpl_Type MFmtImpl;

}
// Impls from Build/include/FEXCore/IR/IRDefines.inc
namespace FCL::FEXCore::IR {
	using GetName_Type = const std::string_view & (*)(::FEXCore::IR::IROps);
	const char GetName_sym_name[] = "_ZN7FEXCore2IR7GetNameENS0_5IROpsE";
	extern GetName_Type GetName;

	using GetArgs_Type = uint8_t (*)(::FEXCore::IR::IROps);
	const char GetArgs_sym_name[] = "_ZN7FEXCore2IR7GetArgsENS0_5IROpsE";
	extern GetArgs_Type GetArgs;

	using GetRAArgs_Type = uint8_t (*)(::FEXCore::IR::IROps);
	const char GetRAArgs_sym_name[] = "_ZN7FEXCore2IR9GetRAArgsENS0_5IROpsE";
	extern GetRAArgs_Type GetRAArgs;

	using GetRegClass_Type = ::FEXCore::IR::RegisterClassType (*)(::FEXCore::IR::IROps);
	const char GetRegClass_sym_name[] = "_ZN7FEXCore2IR11GetRegClassENS0_5IROpsE";
	extern GetRegClass_Type GetRegClass;

	using HasSideEffects_Type = bool (*)(::FEXCore::IR::IROps);
	const char HasSideEffects_sym_name[] = "_ZN7FEXCore2IR14HasSideEffectsENS0_5IROpsE";
	extern HasSideEffects_Type HasSideEffects;

	using GetHasDest_Type = bool (*)(::FEXCore::IR::IROps);
	const char GetHasDest_sym_name[] = "_ZN7FEXCore2IR10GetHasDestENS0_5IROpsE";
	extern GetHasDest_Type GetHasDest;

}
// Impls from External/FEXCore/include/FEXCore/IR/IR.h
namespace FCL::FEXCore::IR {
	using Dump_Type = void (*)(::fextl::stringstream *, const ::FEXCore::IR::IRListView *, ::FEXCore::IR::RegisterAllocationData *);
	const char Dump_sym_name[] = "_ZN7FEXCore2IR4DumpEPNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEEPKNS0_10IRListViewEPNS0_22RegisterAllocationDataE";
	extern Dump_Type Dump;

	using Parse_Type = ::fextl::unique_ptr<::FEXCore::IR::IREmitter> (*)(::FEXCore::Utils::IntrusivePooledAllocator &, ::fextl::stringstream &);
	const char Parse_sym_name[] = "_ZN7FEXCore2IR5ParseERNS_5Utils24IntrusivePooledAllocatorERNSt7__cxx1118basic_stringstreamIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEE";
	extern Parse_Type Parse;

}
// Impls from External/FEXCore/include/FEXCore/Core/Context.h
namespace FCL::FEXCore::Context {
	using CreateNewContext_Type = ::fextl::unique_ptr<::FEXCore::Context::Context> (*)();
	const char CreateNewContext_sym_name[] = "_ZN7FEXCore7Context7Context16CreateNewContextEv";
	extern CreateNewContext_Type CreateNewContext;

	using InitializeContext_Type = bool (*)();
	const char InitializeContext_sym_name[] = "_ZN7FEXCore7Context7Context17InitializeContextEv";
	extern InitializeContext_Type InitializeContext;

	using InitCore_Type = ::FEXCore::Core::InternalThreadState * (*)(uint64_t, uint64_t);
	const char InitCore_sym_name[] = "_ZN7FEXCore7Context7Context8InitCoreEmm";
	extern InitCore_Type InitCore;

	using SetExitHandler_Type = void (*)(::FEXCore::Context::ExitHandler);
	const char SetExitHandler_sym_name[] = "_ZN7FEXCore7Context7Context14SetExitHandlerESt8functionIFvmNS0_10ExitReasonEEE";
	extern SetExitHandler_Type SetExitHandler;

	using GetExitHandler_Type = ::FEXCore::Context::ExitHandler (*)();
	const char GetExitHandler_sym_name[] = "_ZNK7FEXCore7Context7Context14GetExitHandlerEv";
	extern GetExitHandler_Type GetExitHandler;

	using Pause_Type = void (*)();
	const char Pause_sym_name[] = "_ZN7FEXCore7Context7Context5PauseEv";
	extern Pause_Type Pause;

	using Run_Type = void (*)();
	const char Run_sym_name[] = "_ZN7FEXCore7Context7Context3RunEv";
	extern Run_Type Run;

	using Stop_Type = void (*)();
	const char Stop_sym_name[] = "_ZN7FEXCore7Context7Context4StopEv";
	extern Stop_Type Stop;

	using Step_Type = void (*)();
	const char Step_sym_name[] = "_ZN7FEXCore7Context7Context4StepEv";
	extern Step_Type Step;

	using RunUntilExit_Type = ::FEXCore::Context::ExitReason (*)();
	const char RunUntilExit_sym_name[] = "_ZN7FEXCore7Context7Context12RunUntilExitEv";
	extern RunUntilExit_Type RunUntilExit;

	using CompileRIP_Type = void (*)(::FEXCore::Core::InternalThreadState *, uint64_t);
	const char CompileRIP_sym_name[] = "_ZN7FEXCore7Context7Context10CompileRIPEPNS_4Core19InternalThreadStateEm";
	extern CompileRIP_Type CompileRIP;

	using GetProgramStatus_Type = int (*)();
	const char GetProgramStatus_sym_name[] = "_ZNK7FEXCore7Context7Context16GetProgramStatusEv";
	extern GetProgramStatus_Type GetProgramStatus;

	using GetExitReason_Type = ::FEXCore::Context::ExitReason (*)();
	const char GetExitReason_sym_name[] = "_ZN7FEXCore7Context7Context13GetExitReasonEv";
	extern GetExitReason_Type GetExitReason;

	using IsDone_Type = bool (*)();
	const char IsDone_sym_name[] = "_ZNK7FEXCore7Context7Context6IsDoneEv";
	extern IsDone_Type IsDone;

	using GetCPUState_Type = void (*)(::FEXCore::Core::CPUState *);
	const char GetCPUState_sym_name[] = "_ZNK7FEXCore7Context7Context11GetCPUStateEPNS_4Core8CPUStateE";
	extern GetCPUState_Type GetCPUState;

	using SetCPUState_Type = void (*)(const ::FEXCore::Core::CPUState *);
	const char SetCPUState_sym_name[] = "_ZN7FEXCore7Context7Context11SetCPUStateEPKNS_4Core8CPUStateE";
	extern SetCPUState_Type SetCPUState;

	using SetCustomCPUBackendFactory_Type = void (*)(::FEXCore::Context::CustomCPUFactoryType);
	const char SetCustomCPUBackendFactory_sym_name[] = "_ZN7FEXCore7Context7Context26SetCustomCPUBackendFactoryESt8functionIFSt10unique_ptrINS_3CPU10CPUBackendEN5fextl14default_deleteIS5_EEEPS1_PNS_4Core19InternalThreadStateEEE";
	extern SetCustomCPUBackendFactory_Type SetCustomCPUBackendFactory;

	using AddVirtualMemoryMapping_Type = bool (*)(uint64_t, uint64_t, uint64_t);
	const char AddVirtualMemoryMapping_sym_name[] = "_ZN7FEXCore7Context7Context23AddVirtualMemoryMappingEmmm";
	extern AddVirtualMemoryMapping_Type AddVirtualMemoryMapping;

	using GetHostFeatures_Type = ::FEXCore::HostFeatures (*)();
	const char GetHostFeatures_sym_name[] = "_ZNK7FEXCore7Context7Context15GetHostFeaturesEv";
	extern GetHostFeatures_Type GetHostFeatures;

	using HandleCallback_Type = void (*)(::FEXCore::Core::InternalThreadState *, uint64_t);
	const char HandleCallback_sym_name[] = "_ZN7FEXCore7Context7Context14HandleCallbackEPNS_4Core19InternalThreadStateEm";
	extern HandleCallback_Type HandleCallback;

	using RegisterHostSignalHandler_Type = void (*)(int, ::FEXCore::HostSignalDelegatorFunction, bool);
	const char RegisterHostSignalHandler_sym_name[] = "_ZN7FEXCore7Context7Context25RegisterHostSignalHandlerEiSt8functionIFbPNS_4Core19InternalThreadStateEiPvS6_EEb";
	extern RegisterHostSignalHandler_Type RegisterHostSignalHandler;

	using HandleSignalHandlerReturn_Type = void (*)(bool);
	const char HandleSignalHandlerReturn_sym_name[] = "_ZN7FEXCore7Context7Context25HandleSignalHandlerReturnEb";
	extern HandleSignalHandlerReturn_Type HandleSignalHandlerReturn;

	using RegisterFrontendHostSignalHandler_Type = void (*)(int, ::FEXCore::HostSignalDelegatorFunction, bool);
	const char RegisterFrontendHostSignalHandler_sym_name[] = "_ZN7FEXCore7Context7Context33RegisterFrontendHostSignalHandlerEiSt8functionIFbPNS_4Core19InternalThreadStateEiPvS6_EEb";
	extern RegisterFrontendHostSignalHandler_Type RegisterFrontendHostSignalHandler;

	using CreateThread_Type = ::FEXCore::Core::InternalThreadState * (*)(::FEXCore::Core::CPUState *, uint64_t);
	const char CreateThread_sym_name[] = "_ZN7FEXCore7Context7Context12CreateThreadEPNS_4Core8CPUStateEm";
	extern CreateThread_Type CreateThread;

	using ExecutionThread_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char ExecutionThread_sym_name[] = "_ZN7FEXCore7Context7Context15ExecutionThreadEPNS_4Core19InternalThreadStateE";
	extern ExecutionThread_Type ExecutionThread;

	using InitializeThread_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char InitializeThread_sym_name[] = "_ZN7FEXCore7Context7Context16InitializeThreadEPNS_4Core19InternalThreadStateE";
	extern InitializeThread_Type InitializeThread;

	using RunThread_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char RunThread_sym_name[] = "_ZN7FEXCore7Context7Context9RunThreadEPNS_4Core19InternalThreadStateE";
	extern RunThread_Type RunThread;

	using StopThread_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char StopThread_sym_name[] = "_ZN7FEXCore7Context7Context10StopThreadEPNS_4Core19InternalThreadStateE";
	extern StopThread_Type StopThread;

	using DestroyThread_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char DestroyThread_sym_name[] = "_ZN7FEXCore7Context7Context13DestroyThreadEPNS_4Core19InternalThreadStateE";
	extern DestroyThread_Type DestroyThread;

	using CleanupAfterFork_Type = void (*)(::FEXCore::Core::InternalThreadState *);
	const char CleanupAfterFork_sym_name[] = "_ZN7FEXCore7Context7Context16CleanupAfterForkEPNS_4Core19InternalThreadStateE";
	extern CleanupAfterFork_Type CleanupAfterFork;

	using SetSignalDelegator_Type = void (*)(::FEXCore::SignalDelegator *);
	const char SetSignalDelegator_sym_name[] = "_ZN7FEXCore7Context7Context18SetSignalDelegatorEPNS_15SignalDelegatorE";
	extern SetSignalDelegator_Type SetSignalDelegator;

	using SetSyscallHandler_Type = void (*)(::FEXCore::HLE::SyscallHandler *);
	const char SetSyscallHandler_sym_name[] = "_ZN7FEXCore7Context7Context17SetSyscallHandlerEPNS_3HLE14SyscallHandlerE";
	extern SetSyscallHandler_Type SetSyscallHandler;

	using RunCPUIDFunction_Type = ::FEXCore::CPUID::FunctionResults (*)(uint32_t, uint32_t);
	const char RunCPUIDFunction_sym_name[] = "_ZN7FEXCore7Context7Context16RunCPUIDFunctionEjj";
	extern RunCPUIDFunction_Type RunCPUIDFunction;

	using RunCPUIDFunctionName_Type = ::FEXCore::CPUID::FunctionResults (*)(uint32_t, uint32_t, uint32_t);
	const char RunCPUIDFunctionName_sym_name[] = "_ZN7FEXCore7Context7Context20RunCPUIDFunctionNameEjjj";
	extern RunCPUIDFunctionName_Type RunCPUIDFunctionName;

	using LoadAOTIRCacheEntry_Type = ::FEXCore::IR::AOTIRCacheEntry * (*)(const fextl::string &);
	const char LoadAOTIRCacheEntry_sym_name[] = "_ZN7FEXCore7Context7Context19LoadAOTIRCacheEntryERKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEE";
	extern LoadAOTIRCacheEntry_Type LoadAOTIRCacheEntry;

	using UnloadAOTIRCacheEntry_Type = void (*)(::FEXCore::IR::AOTIRCacheEntry *);
	const char UnloadAOTIRCacheEntry_sym_name[] = "_ZN7FEXCore7Context7Context21UnloadAOTIRCacheEntryEPNS_2IR15AOTIRCacheEntryE";
	extern UnloadAOTIRCacheEntry_Type UnloadAOTIRCacheEntry;

	using SetAOTIRLoader_Type = void (*)(std::function<int (const fextl::string &)>);
	const char SetAOTIRLoader_sym_name[] = "_ZN7FEXCore7Context7Context14SetAOTIRLoaderESt8functionIFiRKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEEEE";
	extern SetAOTIRLoader_Type SetAOTIRLoader;

	using SetAOTIRWriter_Type = void (*)(std::function<fextl::unique_ptr< ::FEXCore::Context::AOTIRWriter> (const fextl::string &)>);
	const char SetAOTIRWriter_sym_name[] = "_ZN7FEXCore7Context7Context14SetAOTIRWriterESt8functionIFSt10unique_ptrINS0_11AOTIRWriterEN5fextl14default_deleteIS4_EEERKNSt7__cxx1112basic_stringIcSt11char_traitsIcENS5_8FEXAllocIcEEEEEE";
	extern SetAOTIRWriter_Type SetAOTIRWriter;

	using SetAOTIRRenamer_Type = void (*)(std::function<void (const fextl::string &)>);
	const char SetAOTIRRenamer_sym_name[] = "_ZN7FEXCore7Context7Context15SetAOTIRRenamerESt8functionIFvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEEEE";
	extern SetAOTIRRenamer_Type SetAOTIRRenamer;

	using FinalizeAOTIRCache_Type = void (*)();
	const char FinalizeAOTIRCache_sym_name[] = "_ZN7FEXCore7Context7Context18FinalizeAOTIRCacheEv";
	extern FinalizeAOTIRCache_Type FinalizeAOTIRCache;

	using WriteFilesWithCode_Type = void (*)(std::function<void (const fextl::string &, const fextl::string &)>);
	const char WriteFilesWithCode_sym_name[] = "_ZN7FEXCore7Context7Context18WriteFilesWithCodeESt8functionIFvRKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEESC_EE";
	extern WriteFilesWithCode_Type WriteFilesWithCode;

	using InvalidateGuestCodeRange_Type = void (*)(uint64_t, uint64_t);
	const char InvalidateGuestCodeRange_sym_name[] = "_ZN7FEXCore7Context7Context24InvalidateGuestCodeRangeEmm";
	extern InvalidateGuestCodeRange_Type InvalidateGuestCodeRange;

	using MarkMemoryShared_Type = void (*)();
	const char MarkMemoryShared_sym_name[] = "_ZN7FEXCore7Context7Context16MarkMemorySharedEv";
	extern MarkMemoryShared_Type MarkMemoryShared;

	using ConfigureAOTGen_Type = void (*)(::FEXCore::Core::InternalThreadState *, ::fextl::set<uint64_t> *, uint64_t);
	const char ConfigureAOTGen_sym_name[] = "_ZN7FEXCore7Context7Context15ConfigureAOTGenEPNS_4Core19InternalThreadStateEPSt3setImSt4lessImEN5fextl8FEXAllocImEEEm";
	extern ConfigureAOTGen_Type ConfigureAOTGen;

	using AddCustomIREntrypoint_Type = ::FEXCore::Context::CustomIRResult (*)(uintptr_t, std::function<void (uintptr_t, ::FEXCore::IR::IREmitter *)>, void *, void *);
	const char AddCustomIREntrypoint_sym_name[] = "_ZN7FEXCore7Context7Context21AddCustomIREntrypointEmSt8functionIFvmPNS_2IR9IREmitterEEEPvS8_";
	extern AddCustomIREntrypoint_Type AddCustomIREntrypoint;

	using AppendThunkDefinitions_Type = void (*)(const fextl::vector< ::FEXCore::IR::ThunkDefinition> &);
	const char AppendThunkDefinitions_sym_name[] = "_ZN7FEXCore7Context7Context22AppendThunkDefinitionsERKSt6vectorINS_2IR15ThunkDefinitionEN5fextl8FEXAllocIS4_EEE";
	extern AppendThunkDefinitions_Type AppendThunkDefinitions;

	using SetVDSOSigReturn_Type = void (*)(const ::FEXCore::Context::VDSOSigReturn &);
	const char SetVDSOSigReturn_sym_name[] = "_ZN7FEXCore7Context7Context16SetVDSOSigReturnERKNS0_13VDSOSigReturnE";
	extern SetVDSOSigReturn_Type SetVDSOSigReturn;

	using InitializeStaticTables_Type = void (*)(::FEXCore::Context::OperatingMode);
	const char InitializeStaticTables_sym_name[] = "_ZN7FEXCore7Context22InitializeStaticTablesENS0_13OperatingModeE";
	extern InitializeStaticTables_Type InitializeStaticTables;

}
// Impls from External/FEXCore/include/FEXCore/Config/Config.h
namespace FCL::FEXCore::Config {
	using SetDataDirectory_Type = void (*)(std::string_view);
	const char SetDataDirectory_sym_name[] = "_ZN7FEXCore6Config16SetDataDirectoryESt17basic_string_viewIcSt11char_traitsIcEE";
	extern SetDataDirectory_Type SetDataDirectory;

	using SetConfigDirectory_Type = void (*)(const std::string_view, bool);
	const char SetConfigDirectory_sym_name[] = "_ZN7FEXCore6Config18SetConfigDirectoryESt17basic_string_viewIcSt11char_traitsIcEEb";
	extern SetConfigDirectory_Type SetConfigDirectory;

	using SetConfigFileLocation_Type = void (*)(std::string_view, bool);
	const char SetConfigFileLocation_sym_name[] = "_ZN7FEXCore6Config21SetConfigFileLocationESt17basic_string_viewIcSt11char_traitsIcEEb";
	extern SetConfigFileLocation_Type SetConfigFileLocation;

	using GetDataDirectory_Type = const fextl::string & (*)();
	const char GetDataDirectory_sym_name[] = "_ZN7FEXCore6Config16GetDataDirectoryB5cxx11Ev";
	extern GetDataDirectory_Type GetDataDirectory;

	using GetConfigDirectory_Type = const fextl::string & (*)(bool);
	const char GetConfigDirectory_sym_name[] = "_ZN7FEXCore6Config18GetConfigDirectoryB5cxx11Eb";
	extern GetConfigDirectory_Type GetConfigDirectory;

	using GetConfigFileLocation_Type = const fextl::string & (*)(bool);
	const char GetConfigFileLocation_sym_name[] = "_ZN7FEXCore6Config21GetConfigFileLocationB5cxx11Eb";
	extern GetConfigFileLocation_Type GetConfigFileLocation;

	using GetApplicationConfig_Type = ::fextl::string (*)(const std::string_view, bool);
	const char GetApplicationConfig_sym_name[] = "_ZN7FEXCore6Config20GetApplicationConfigB5cxx11ESt17basic_string_viewIcSt11char_traitsIcEEb";
	extern GetApplicationConfig_Type GetApplicationConfig;

	using Initialize_Type = void (*)();
	const char Initialize_sym_name[] = "_ZN7FEXCore6Config10InitializeEv";
	extern Initialize_Type Initialize;

	using Shutdown_Type = void (*)();
	const char Shutdown_sym_name[] = "_ZN7FEXCore6Config8ShutdownEv";
	extern Shutdown_Type Shutdown;

	using Load_Type = void (*)();
	const char Load_sym_name[] = "_ZN7FEXCore6Config4LoadEv";
	extern Load_Type Load;

	using ReloadMetaLayer_Type = void (*)();
	const char ReloadMetaLayer_sym_name[] = "_ZN7FEXCore6Config15ReloadMetaLayerEv";
	extern ReloadMetaLayer_Type ReloadMetaLayer;

	using FindContainer_Type = ::fextl::string (*)();
	const char FindContainer_sym_name[] = "_ZN7FEXCore6Config13FindContainerB5cxx11Ev";
	extern FindContainer_Type FindContainer;

	using FindContainerPrefix_Type = ::fextl::string (*)();
	const char FindContainerPrefix_sym_name[] = "_ZN7FEXCore6Config19FindContainerPrefixB5cxx11Ev";
	extern FindContainerPrefix_Type FindContainerPrefix;

	using AddLayer_Type = void (*)(::fextl::unique_ptr< ::FEXCore::Config::Layer>);
	const char AddLayer_sym_name[] = "_ZN7FEXCore6Config8AddLayerESt10unique_ptrINS0_5LayerEN5fextl14default_deleteIS2_EEE";
	extern AddLayer_Type AddLayer;

	using Exists_Type = bool (*)(::FEXCore::Config::ConfigOption);
	const char Exists_sym_name[] = "_ZN7FEXCore6Config6ExistsENS0_12ConfigOptionE";
	extern Exists_Type Exists;

	using All_Type = std::optional< ::FEXCore::Config::LayerValue *> (*)(::FEXCore::Config::ConfigOption);
	const char All_sym_name[] = "_ZN7FEXCore6Config3AllB5cxx11ENS0_12ConfigOptionE";
	extern All_Type All;

	using Get_Type = std::optional< ::fextl::string *> (*)(::FEXCore::Config::ConfigOption);
	const char Get_sym_name[] = "_ZN7FEXCore6Config3GetB5cxx11ENS0_12ConfigOptionE";
	extern Get_Type Get;

	using Set_Type = void (*)(::FEXCore::Config::ConfigOption, std::string_view);
	const char Set_sym_name[] = "_ZN7FEXCore6Config3SetENS0_12ConfigOptionESt17basic_string_viewIcSt11char_traitsIcEE";
	extern Set_Type Set;

	using Erase_Type = void (*)(::FEXCore::Config::ConfigOption);
	const char Erase_sym_name[] = "_ZN7FEXCore6Config5EraseENS0_12ConfigOptionE";
	extern Erase_Type Erase;

	using EraseSet_Type = void (*)(::FEXCore::Config::ConfigOption, std::string_view);
	const char EraseSet_sym_name[] = "_ZN7FEXCore6Config8EraseSetENS0_12ConfigOptionESt17basic_string_viewIcSt11char_traitsIcEE";
	extern EraseSet_Type EraseSet;

	using CreateGlobalMainLayer_Type = ::fextl::unique_ptr<::FEXCore::Config::Layer> (*)();
	const char CreateGlobalMainLayer_sym_name[] = "_ZN7FEXCore6Config21CreateGlobalMainLayerEv";
	extern CreateGlobalMainLayer_Type CreateGlobalMainLayer;

	using CreateMainLayer_Type = ::fextl::unique_ptr<::FEXCore::Config::Layer> (*)(const fextl::string *);
	const char CreateMainLayer_sym_name[] = "_ZN7FEXCore6Config15CreateMainLayerEPKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEE";
	extern CreateMainLayer_Type CreateMainLayer;

	using CreateAppLayer_Type = ::fextl::unique_ptr<::FEXCore::Config::Layer> (*)(const fextl::string &, ::FEXCore::Config::LayerType);
	const char CreateAppLayer_sym_name[] = "_ZN7FEXCore6Config14CreateAppLayerERKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEENS0_9LayerTypeE";
	extern CreateAppLayer_Type CreateAppLayer;

	using CreateEnvironmentLayer_Type = ::fextl::unique_ptr<::FEXCore::Config::Layer> (*)(char *const[]);
	const char CreateEnvironmentLayer_sym_name[] = "_ZN7FEXCore6Config22CreateEnvironmentLayerEPKPc";
	extern CreateEnvironmentLayer_Type CreateEnvironmentLayer;

}
// Impls from External/FEXCore/include/FEXCore/Core/CoreState.h
namespace FCL::FEXCore::Core {
	using GetFlagName_Type = const std::string_view & (*)(unsigned int);
	const char GetFlagName_sym_name[] = "_ZN7FEXCore4Core11GetFlagNameEj";
	extern GetFlagName_Type GetFlagName;

	using GetGRegName_Type = const std::string_view & (*)(unsigned int);
	const char GetGRegName_sym_name[] = "_ZN7FEXCore4Core11GetGRegNameEj";
	extern GetGRegName_Type GetGRegName;

}
// Impls from External/FEXCore/include/FEXCore/fextl/memory_resource.h
namespace FCL::fextl::pmr {
	using get_default_resource_Type = std::pmr::memory_resource * (*)();
	const char get_default_resource_sym_name[] = "_ZN5fextl3pmr20get_default_resourceEv";
	extern get_default_resource_Type get_default_resource;

}
// Impls from External/FEXCore/include/FEXCore/Utils/CPUInfo.h
namespace FCL::FEXCore::CPUInfo {
	using CalculateNumberOfCPUs_Type = uint32_t (*)();
	const char CalculateNumberOfCPUs_sym_name[] = "_ZN7FEXCore7CPUInfo21CalculateNumberOfCPUsEv";
	extern CalculateNumberOfCPUs_Type CalculateNumberOfCPUs;

}
// Impls from External/FEXCore/include/FEXCore/Utils/FileLoading.h
namespace FCL::FEXCore::FileLoading {
	using LoadFile_Type = bool (*)(::fextl::vector<char> &, const ::fextl::string &, size_t);
	const char LoadFile_sym_name[] = "_ZN7FEXCore11FileLoading8LoadFileERSt6vectorIcN5fextl8FEXAllocIcEEERKNSt7__cxx1112basic_stringIcSt11char_traitsIcES4_EEm";
	extern LoadFile_Type LoadFile;

	using LoadFileToBuffer_Type = ssize_t (*)(const ::fextl::string &, std::span<char>);
	const char LoadFileToBuffer_sym_name[] = "_ZN7FEXCore11FileLoading16LoadFileToBufferERKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEESt4spanIcLm18446744073709551615EE";
	extern LoadFileToBuffer_Type LoadFileToBuffer;

}
// Impls from External/FEXCore/include/FEXCore/Utils/Telemetry.h
namespace FCL::FEXCore::Telemetry {
	using Initialize_Type = void (*)();
	const char Initialize_sym_name[] = "_ZN7FEXCore9Telemetry10InitializeEv";
	extern Initialize_Type Initialize;

	using Shutdown_Type = void (*)(const ::fextl::string &);
	const char Shutdown_sym_name[] = "_ZN7FEXCore9Telemetry8ShutdownERKNSt7__cxx1112basic_stringIcSt11char_traitsIcEN5fextl8FEXAllocIcEEEE";
	extern Shutdown_Type Shutdown;

}
namespace FCL {
using LoaderFunction = void* (*)(void* UserPtr, const char* SymbolName);
bool LoadFEXCoreSymbols(LoaderFunction Load);
}
#undef FEXCORE_FORWARDDECL_DEFINES
#endif
#ifdef FEXCORE_IMPL_DEFINES
// Impls from External/FEXCore/include/FEXCore/Utils/CompilerDefs.h
namespace FCL::FEXCore::Assert {
	ForcedAssert_Type ForcedAssert;

}
// Impls from External/FEXCore/include/FEXCore/Utils/AllocatorHooks.h
namespace FCL {
	je_malloc_Type je_malloc;

	je_calloc_Type je_calloc;

	je_memalign_Type je_memalign;

	je_valloc_Type je_valloc;

	je_posix_memalign_Type je_posix_memalign;

	je_realloc_Type je_realloc;

	je_free_Type je_free;

	je_malloc_usable_size_Type je_malloc_usable_size;

	je_aligned_alloc_Type je_aligned_alloc;

}
namespace FCL::FEXCore::Allocator {
	::FEXCore::Allocator::MMAP_Hook mmap;

	::FEXCore::Allocator::MUNMAP_Hook munmap;

}
// Impls from External/FEXCore/include/FEXCore/Core/SignalDelegator.h
namespace FCL::FEXCore {
	RegisterTLSState_Type RegisterTLSState;

	UninstallTLSState_Type UninstallTLSState;

	RegisterHostSignalHandler_Type RegisterHostSignalHandler;

	RegisterFrontendHostSignalHandler_Type RegisterFrontendHostSignalHandler;

	RegisterHostSignalHandlerForGuest_Type RegisterHostSignalHandlerForGuest;

	HandleSignal_Type HandleSignal;

	CheckXIDHandler_Type CheckXIDHandler;

}
// Impls from External/FEXCore/include/FEXCore/Utils/Allocator.h
namespace FCL::FEXCore::Allocator {
	SetupHooks_Type SetupHooks;

	ClearHooks_Type ClearHooks;

	DetermineVASize_Type DetermineVASize;

	DisableSBRKAllocations_Type DisableSBRKAllocations;

	ReenableSBRKAllocations_Type ReenableSBRKAllocations;

	StealMemoryRegion_Type StealMemoryRegion;

	ReclaimMemoryRegion_Type ReclaimMemoryRegion;

	Steal48BitVA_Type Steal48BitVA;

}
// Impls from External/FEXCore/include/FEXCore/Utils/LogManager.h
namespace FCL::LogMan::Throw {
	InstallHandler_Type InstallHandler;

	UnInstallHandlers_Type UnInstallHandlers;

}
namespace FCL::LogMan::Msg {
	InstallHandler_Type InstallHandler;

	UnInstallHandlers_Type UnInstallHandlers;

	D_Type D;

	MFmtImpl_Type MFmtImpl;

}
// Impls from Build/include/FEXCore/IR/IRDefines.inc
namespace FCL::FEXCore::IR {
	GetName_Type GetName;

	GetArgs_Type GetArgs;

	GetRAArgs_Type GetRAArgs;

	GetRegClass_Type GetRegClass;

	HasSideEffects_Type HasSideEffects;

	GetHasDest_Type GetHasDest;

}
// Impls from External/FEXCore/include/FEXCore/IR/IR.h
namespace FCL::FEXCore::IR {
	Dump_Type Dump;

	Parse_Type Parse;

}
// Impls from External/FEXCore/include/FEXCore/Core/Context.h
namespace FCL::FEXCore::Context {
	CreateNewContext_Type CreateNewContext;

	InitializeContext_Type InitializeContext;

	InitCore_Type InitCore;

	SetExitHandler_Type SetExitHandler;

	GetExitHandler_Type GetExitHandler;

	Pause_Type Pause;

	Run_Type Run;

	Stop_Type Stop;

	Step_Type Step;

	RunUntilExit_Type RunUntilExit;

	CompileRIP_Type CompileRIP;

	GetProgramStatus_Type GetProgramStatus;

	GetExitReason_Type GetExitReason;

	IsDone_Type IsDone;

	GetCPUState_Type GetCPUState;

	SetCPUState_Type SetCPUState;

	SetCustomCPUBackendFactory_Type SetCustomCPUBackendFactory;

	AddVirtualMemoryMapping_Type AddVirtualMemoryMapping;

	GetHostFeatures_Type GetHostFeatures;

	HandleCallback_Type HandleCallback;

	RegisterHostSignalHandler_Type RegisterHostSignalHandler;

	HandleSignalHandlerReturn_Type HandleSignalHandlerReturn;

	RegisterFrontendHostSignalHandler_Type RegisterFrontendHostSignalHandler;

	CreateThread_Type CreateThread;

	ExecutionThread_Type ExecutionThread;

	InitializeThread_Type InitializeThread;

	RunThread_Type RunThread;

	StopThread_Type StopThread;

	DestroyThread_Type DestroyThread;

	CleanupAfterFork_Type CleanupAfterFork;

	SetSignalDelegator_Type SetSignalDelegator;

	SetSyscallHandler_Type SetSyscallHandler;

	RunCPUIDFunction_Type RunCPUIDFunction;

	RunCPUIDFunctionName_Type RunCPUIDFunctionName;

	LoadAOTIRCacheEntry_Type LoadAOTIRCacheEntry;

	UnloadAOTIRCacheEntry_Type UnloadAOTIRCacheEntry;

	SetAOTIRLoader_Type SetAOTIRLoader;

	SetAOTIRWriter_Type SetAOTIRWriter;

	SetAOTIRRenamer_Type SetAOTIRRenamer;

	FinalizeAOTIRCache_Type FinalizeAOTIRCache;

	WriteFilesWithCode_Type WriteFilesWithCode;

	InvalidateGuestCodeRange_Type InvalidateGuestCodeRange;

	MarkMemoryShared_Type MarkMemoryShared;

	ConfigureAOTGen_Type ConfigureAOTGen;

	AddCustomIREntrypoint_Type AddCustomIREntrypoint;

	AppendThunkDefinitions_Type AppendThunkDefinitions;

	SetVDSOSigReturn_Type SetVDSOSigReturn;

	InitializeStaticTables_Type InitializeStaticTables;

}
// Impls from External/FEXCore/include/FEXCore/Config/Config.h
namespace FCL::FEXCore::Config {
	SetDataDirectory_Type SetDataDirectory;

	SetConfigDirectory_Type SetConfigDirectory;

	SetConfigFileLocation_Type SetConfigFileLocation;

	GetDataDirectory_Type GetDataDirectory;

	GetConfigDirectory_Type GetConfigDirectory;

	GetConfigFileLocation_Type GetConfigFileLocation;

	GetApplicationConfig_Type GetApplicationConfig;

	Initialize_Type Initialize;

	Shutdown_Type Shutdown;

	Load_Type Load;

	ReloadMetaLayer_Type ReloadMetaLayer;

	FindContainer_Type FindContainer;

	FindContainerPrefix_Type FindContainerPrefix;

	AddLayer_Type AddLayer;

	Exists_Type Exists;

	All_Type All;

	Get_Type Get;

	Set_Type Set;

	Erase_Type Erase;

	EraseSet_Type EraseSet;

	CreateGlobalMainLayer_Type CreateGlobalMainLayer;

	CreateMainLayer_Type CreateMainLayer;

	CreateAppLayer_Type CreateAppLayer;

	CreateEnvironmentLayer_Type CreateEnvironmentLayer;

}
// Impls from External/FEXCore/include/FEXCore/Core/CoreState.h
namespace FCL::FEXCore::Core {
	GetFlagName_Type GetFlagName;

	GetGRegName_Type GetGRegName;

}
// Impls from External/FEXCore/include/FEXCore/fextl/memory_resource.h
namespace FCL::fextl::pmr {
	get_default_resource_Type get_default_resource;

}
// Impls from External/FEXCore/include/FEXCore/Utils/CPUInfo.h
namespace FCL::FEXCore::CPUInfo {
	CalculateNumberOfCPUs_Type CalculateNumberOfCPUs;

}
// Impls from External/FEXCore/include/FEXCore/Utils/FileLoading.h
namespace FCL::FEXCore::FileLoading {
	LoadFile_Type LoadFile;

	LoadFileToBuffer_Type LoadFileToBuffer;

}
// Impls from External/FEXCore/include/FEXCore/Utils/Telemetry.h
namespace FCL::FEXCore::Telemetry {
	Initialize_Type Initialize;

	Shutdown_Type Shutdown;

}
#undef FEXCORE_IMPL_DEFINES
#endif
#ifdef FEXCORE_LOADER
namespace FCL {
bool LoadFEXCoreSymbols(LoaderFunction Load, void* UserPtr) {
	// Impls from External/FEXCore/include/FEXCore/Utils/CompilerDefs.h
	{
		auto SymbolName = FCL::FEXCore::Assert::ForcedAssert_sym_name;
		FCL::FEXCore::Assert::ForcedAssert_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Assert::ForcedAssert = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/AllocatorHooks.h
	{
		auto SymbolName = FCL::je_malloc_sym_name;
		FCL::je_malloc_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_malloc = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_calloc_sym_name;
		FCL::je_calloc_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_calloc = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_memalign_sym_name;
		FCL::je_memalign_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_memalign = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_valloc_sym_name;
		FCL::je_valloc_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_valloc = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_posix_memalign_sym_name;
		FCL::je_posix_memalign_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_posix_memalign = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_realloc_sym_name;
		FCL::je_realloc_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_realloc = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_free_sym_name;
		FCL::je_free_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_free = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_malloc_usable_size_sym_name;
		FCL::je_malloc_usable_size_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_malloc_usable_size = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::je_aligned_alloc_sym_name;
		FCL::je_aligned_alloc_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::je_aligned_alloc = LoadedSymbol;
	}
	{
		auto DataSymbolName = FCL::FEXCore::Allocator::mmap_sym_name;
		::FEXCore::Allocator::MMAP_Hook LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, DataSymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::mmap = LoadedSymbol;
	}
	{
		auto DataSymbolName = FCL::FEXCore::Allocator::munmap_sym_name;
		::FEXCore::Allocator::MUNMAP_Hook LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, DataSymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::munmap = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Core/SignalDelegator.h
	{
		auto SymbolName = FCL::FEXCore::RegisterTLSState_sym_name;
		FCL::FEXCore::RegisterTLSState_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::RegisterTLSState = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::UninstallTLSState_sym_name;
		FCL::FEXCore::UninstallTLSState_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::UninstallTLSState = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::RegisterHostSignalHandler_sym_name;
		FCL::FEXCore::RegisterHostSignalHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::RegisterHostSignalHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::RegisterFrontendHostSignalHandler_sym_name;
		FCL::FEXCore::RegisterFrontendHostSignalHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::RegisterFrontendHostSignalHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::RegisterHostSignalHandlerForGuest_sym_name;
		FCL::FEXCore::RegisterHostSignalHandlerForGuest_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::RegisterHostSignalHandlerForGuest = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::HandleSignal_sym_name;
		FCL::FEXCore::HandleSignal_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::HandleSignal = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::CheckXIDHandler_sym_name;
		FCL::FEXCore::CheckXIDHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::CheckXIDHandler = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/Allocator.h
	{
		auto SymbolName = FCL::FEXCore::Allocator::SetupHooks_sym_name;
		FCL::FEXCore::Allocator::SetupHooks_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::SetupHooks = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::ClearHooks_sym_name;
		FCL::FEXCore::Allocator::ClearHooks_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::ClearHooks = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::DetermineVASize_sym_name;
		FCL::FEXCore::Allocator::DetermineVASize_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::DetermineVASize = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::DisableSBRKAllocations_sym_name;
		FCL::FEXCore::Allocator::DisableSBRKAllocations_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::DisableSBRKAllocations = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::ReenableSBRKAllocations_sym_name;
		FCL::FEXCore::Allocator::ReenableSBRKAllocations_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::ReenableSBRKAllocations = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::StealMemoryRegion_sym_name;
		FCL::FEXCore::Allocator::StealMemoryRegion_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::StealMemoryRegion = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::ReclaimMemoryRegion_sym_name;
		FCL::FEXCore::Allocator::ReclaimMemoryRegion_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::ReclaimMemoryRegion = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Allocator::Steal48BitVA_sym_name;
		FCL::FEXCore::Allocator::Steal48BitVA_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Allocator::Steal48BitVA = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/LogManager.h
	{
		auto SymbolName = FCL::LogMan::Throw::InstallHandler_sym_name;
		FCL::LogMan::Throw::InstallHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Throw::InstallHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::LogMan::Throw::UnInstallHandlers_sym_name;
		FCL::LogMan::Throw::UnInstallHandlers_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Throw::UnInstallHandlers = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::LogMan::Msg::InstallHandler_sym_name;
		FCL::LogMan::Msg::InstallHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Msg::InstallHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::LogMan::Msg::UnInstallHandlers_sym_name;
		FCL::LogMan::Msg::UnInstallHandlers_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Msg::UnInstallHandlers = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::LogMan::Msg::D_sym_name;
		FCL::LogMan::Msg::D_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Msg::D = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::LogMan::Msg::MFmtImpl_sym_name;
		FCL::LogMan::Msg::MFmtImpl_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::LogMan::Msg::MFmtImpl = LoadedSymbol;
	}
	// Impls from Build/include/FEXCore/IR/IRDefines.inc
	{
		auto SymbolName = FCL::FEXCore::IR::GetName_sym_name;
		FCL::FEXCore::IR::GetName_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::GetName = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::GetArgs_sym_name;
		FCL::FEXCore::IR::GetArgs_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::GetArgs = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::GetRAArgs_sym_name;
		FCL::FEXCore::IR::GetRAArgs_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::GetRAArgs = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::GetRegClass_sym_name;
		FCL::FEXCore::IR::GetRegClass_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::GetRegClass = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::HasSideEffects_sym_name;
		FCL::FEXCore::IR::HasSideEffects_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::HasSideEffects = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::GetHasDest_sym_name;
		FCL::FEXCore::IR::GetHasDest_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::GetHasDest = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/IR/IR.h
	{
		auto SymbolName = FCL::FEXCore::IR::Dump_sym_name;
		FCL::FEXCore::IR::Dump_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::Dump = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::IR::Parse_sym_name;
		FCL::FEXCore::IR::Parse_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::IR::Parse = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Core/Context.h
	{
		auto SymbolName = FCL::FEXCore::Context::CreateNewContext_sym_name;
		FCL::FEXCore::Context::CreateNewContext_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::CreateNewContext = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::InitializeContext_sym_name;
		FCL::FEXCore::Context::InitializeContext_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::InitializeContext = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::InitCore_sym_name;
		FCL::FEXCore::Context::InitCore_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::InitCore = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetExitHandler_sym_name;
		FCL::FEXCore::Context::SetExitHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetExitHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::GetExitHandler_sym_name;
		FCL::FEXCore::Context::GetExitHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::GetExitHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::Pause_sym_name;
		FCL::FEXCore::Context::Pause_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::Pause = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::Run_sym_name;
		FCL::FEXCore::Context::Run_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::Run = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::Stop_sym_name;
		FCL::FEXCore::Context::Stop_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::Stop = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::Step_sym_name;
		FCL::FEXCore::Context::Step_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::Step = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RunUntilExit_sym_name;
		FCL::FEXCore::Context::RunUntilExit_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RunUntilExit = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::CompileRIP_sym_name;
		FCL::FEXCore::Context::CompileRIP_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::CompileRIP = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::GetProgramStatus_sym_name;
		FCL::FEXCore::Context::GetProgramStatus_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::GetProgramStatus = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::GetExitReason_sym_name;
		FCL::FEXCore::Context::GetExitReason_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::GetExitReason = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::IsDone_sym_name;
		FCL::FEXCore::Context::IsDone_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::IsDone = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::GetCPUState_sym_name;
		FCL::FEXCore::Context::GetCPUState_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::GetCPUState = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetCPUState_sym_name;
		FCL::FEXCore::Context::SetCPUState_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetCPUState = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetCustomCPUBackendFactory_sym_name;
		FCL::FEXCore::Context::SetCustomCPUBackendFactory_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetCustomCPUBackendFactory = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::AddVirtualMemoryMapping_sym_name;
		FCL::FEXCore::Context::AddVirtualMemoryMapping_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::AddVirtualMemoryMapping = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::GetHostFeatures_sym_name;
		FCL::FEXCore::Context::GetHostFeatures_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::GetHostFeatures = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::HandleCallback_sym_name;
		FCL::FEXCore::Context::HandleCallback_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::HandleCallback = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RegisterHostSignalHandler_sym_name;
		FCL::FEXCore::Context::RegisterHostSignalHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RegisterHostSignalHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::HandleSignalHandlerReturn_sym_name;
		FCL::FEXCore::Context::HandleSignalHandlerReturn_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::HandleSignalHandlerReturn = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RegisterFrontendHostSignalHandler_sym_name;
		FCL::FEXCore::Context::RegisterFrontendHostSignalHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RegisterFrontendHostSignalHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::CreateThread_sym_name;
		FCL::FEXCore::Context::CreateThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::CreateThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::ExecutionThread_sym_name;
		FCL::FEXCore::Context::ExecutionThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::ExecutionThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::InitializeThread_sym_name;
		FCL::FEXCore::Context::InitializeThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::InitializeThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RunThread_sym_name;
		FCL::FEXCore::Context::RunThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RunThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::StopThread_sym_name;
		FCL::FEXCore::Context::StopThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::StopThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::DestroyThread_sym_name;
		FCL::FEXCore::Context::DestroyThread_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::DestroyThread = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::CleanupAfterFork_sym_name;
		FCL::FEXCore::Context::CleanupAfterFork_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::CleanupAfterFork = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetSignalDelegator_sym_name;
		FCL::FEXCore::Context::SetSignalDelegator_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetSignalDelegator = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetSyscallHandler_sym_name;
		FCL::FEXCore::Context::SetSyscallHandler_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetSyscallHandler = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RunCPUIDFunction_sym_name;
		FCL::FEXCore::Context::RunCPUIDFunction_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RunCPUIDFunction = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::RunCPUIDFunctionName_sym_name;
		FCL::FEXCore::Context::RunCPUIDFunctionName_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::RunCPUIDFunctionName = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::LoadAOTIRCacheEntry_sym_name;
		FCL::FEXCore::Context::LoadAOTIRCacheEntry_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::LoadAOTIRCacheEntry = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::UnloadAOTIRCacheEntry_sym_name;
		FCL::FEXCore::Context::UnloadAOTIRCacheEntry_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::UnloadAOTIRCacheEntry = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetAOTIRLoader_sym_name;
		FCL::FEXCore::Context::SetAOTIRLoader_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetAOTIRLoader = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetAOTIRWriter_sym_name;
		FCL::FEXCore::Context::SetAOTIRWriter_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetAOTIRWriter = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetAOTIRRenamer_sym_name;
		FCL::FEXCore::Context::SetAOTIRRenamer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetAOTIRRenamer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::FinalizeAOTIRCache_sym_name;
		FCL::FEXCore::Context::FinalizeAOTIRCache_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::FinalizeAOTIRCache = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::WriteFilesWithCode_sym_name;
		FCL::FEXCore::Context::WriteFilesWithCode_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::WriteFilesWithCode = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::InvalidateGuestCodeRange_sym_name;
		FCL::FEXCore::Context::InvalidateGuestCodeRange_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::InvalidateGuestCodeRange = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::MarkMemoryShared_sym_name;
		FCL::FEXCore::Context::MarkMemoryShared_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::MarkMemoryShared = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::ConfigureAOTGen_sym_name;
		FCL::FEXCore::Context::ConfigureAOTGen_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::ConfigureAOTGen = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::AddCustomIREntrypoint_sym_name;
		FCL::FEXCore::Context::AddCustomIREntrypoint_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::AddCustomIREntrypoint = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::AppendThunkDefinitions_sym_name;
		FCL::FEXCore::Context::AppendThunkDefinitions_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::AppendThunkDefinitions = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::SetVDSOSigReturn_sym_name;
		FCL::FEXCore::Context::SetVDSOSigReturn_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::SetVDSOSigReturn = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Context::InitializeStaticTables_sym_name;
		FCL::FEXCore::Context::InitializeStaticTables_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Context::InitializeStaticTables = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Config/Config.h
	{
		auto SymbolName = FCL::FEXCore::Config::SetDataDirectory_sym_name;
		FCL::FEXCore::Config::SetDataDirectory_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::SetDataDirectory = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::SetConfigDirectory_sym_name;
		FCL::FEXCore::Config::SetConfigDirectory_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::SetConfigDirectory = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::SetConfigFileLocation_sym_name;
		FCL::FEXCore::Config::SetConfigFileLocation_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::SetConfigFileLocation = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::GetDataDirectory_sym_name;
		FCL::FEXCore::Config::GetDataDirectory_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::GetDataDirectory = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::GetConfigDirectory_sym_name;
		FCL::FEXCore::Config::GetConfigDirectory_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::GetConfigDirectory = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::GetConfigFileLocation_sym_name;
		FCL::FEXCore::Config::GetConfigFileLocation_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::GetConfigFileLocation = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::GetApplicationConfig_sym_name;
		FCL::FEXCore::Config::GetApplicationConfig_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::GetApplicationConfig = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Initialize_sym_name;
		FCL::FEXCore::Config::Initialize_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Initialize = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Shutdown_sym_name;
		FCL::FEXCore::Config::Shutdown_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Shutdown = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Load_sym_name;
		FCL::FEXCore::Config::Load_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Load = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::ReloadMetaLayer_sym_name;
		FCL::FEXCore::Config::ReloadMetaLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::ReloadMetaLayer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::FindContainer_sym_name;
		FCL::FEXCore::Config::FindContainer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::FindContainer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::FindContainerPrefix_sym_name;
		FCL::FEXCore::Config::FindContainerPrefix_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::FindContainerPrefix = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::AddLayer_sym_name;
		FCL::FEXCore::Config::AddLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::AddLayer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Exists_sym_name;
		FCL::FEXCore::Config::Exists_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Exists = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::All_sym_name;
		FCL::FEXCore::Config::All_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::All = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Get_sym_name;
		FCL::FEXCore::Config::Get_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Get = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Set_sym_name;
		FCL::FEXCore::Config::Set_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Set = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::Erase_sym_name;
		FCL::FEXCore::Config::Erase_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::Erase = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::EraseSet_sym_name;
		FCL::FEXCore::Config::EraseSet_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::EraseSet = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::CreateGlobalMainLayer_sym_name;
		FCL::FEXCore::Config::CreateGlobalMainLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::CreateGlobalMainLayer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::CreateMainLayer_sym_name;
		FCL::FEXCore::Config::CreateMainLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::CreateMainLayer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::CreateAppLayer_sym_name;
		FCL::FEXCore::Config::CreateAppLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::CreateAppLayer = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Config::CreateEnvironmentLayer_sym_name;
		FCL::FEXCore::Config::CreateEnvironmentLayer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Config::CreateEnvironmentLayer = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Core/CoreState.h
	{
		auto SymbolName = FCL::FEXCore::Core::GetFlagName_sym_name;
		FCL::FEXCore::Core::GetFlagName_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Core::GetFlagName = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Core::GetGRegName_sym_name;
		FCL::FEXCore::Core::GetGRegName_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Core::GetGRegName = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/fextl/memory_resource.h
	{
		auto SymbolName = FCL::fextl::pmr::get_default_resource_sym_name;
		FCL::fextl::pmr::get_default_resource_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::fextl::pmr::get_default_resource = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/CPUInfo.h
	{
		auto SymbolName = FCL::FEXCore::CPUInfo::CalculateNumberOfCPUs_sym_name;
		FCL::FEXCore::CPUInfo::CalculateNumberOfCPUs_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::CPUInfo::CalculateNumberOfCPUs = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/FileLoading.h
	{
		auto SymbolName = FCL::FEXCore::FileLoading::LoadFile_sym_name;
		FCL::FEXCore::FileLoading::LoadFile_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::FileLoading::LoadFile = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::FileLoading::LoadFileToBuffer_sym_name;
		FCL::FEXCore::FileLoading::LoadFileToBuffer_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::FileLoading::LoadFileToBuffer = LoadedSymbol;
	}
	// Impls from External/FEXCore/include/FEXCore/Utils/Telemetry.h
	{
		auto SymbolName = FCL::FEXCore::Telemetry::Initialize_sym_name;
		FCL::FEXCore::Telemetry::Initialize_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Telemetry::Initialize = LoadedSymbol;
	}
	{
		auto SymbolName = FCL::FEXCore::Telemetry::Shutdown_sym_name;
		FCL::FEXCore::Telemetry::Shutdown_Type LoadedSymbol;
		LoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));
		if (LoadedSymbol == nullptr) return false;
		FCL::FEXCore::Telemetry::Shutdown = LoadedSymbol;
	}
	return true;
}
}
#undef FEXCORE_LOADER
#endif
