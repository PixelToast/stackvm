#pragma once

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/Legacy.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include "llvm/IR/Mangler.h"

#include "ir.h"
#include "backend_llvm.h"

namespace JIT {
  void init();

  typedef void (*EntryFn)(void*);

  struct Linker {
    llvm::TargetMachine &machine;
    const llvm::DataLayout dataLayout;
    llvm::LLVMContext &context;
    llvm::orc::ExecutionSession session;
    std::shared_ptr<llvm::orc::SymbolResolver> resolver;
    llvm::orc::LegacyRTDyldObjectLinkingLayer objectLayer;
    llvm::orc::LegacyIRCompileLayer<decltype(objectLayer), llvm::orc::SimpleCompiler> compileLayer;

    Linker(
      llvm::TargetMachine &machine,
      llvm::LLVMContext &context
    );

    std::string mangle(const std::string &name);

    llvm::orc::VModuleKey addModule(std::unique_ptr<llvm::Module> module);
    void removeModule(llvm::orc::VModuleKey key);
    EntryFn findEntry(const std::string& name);
  };

  struct Pipeline;
  struct Handle {
    llvm::orc::VModuleKey key;
    Pipeline &pipeline;
    EntryFn entry;

    Handle(
      llvm::orc::VModuleKey key,
      Pipeline &pipeline,
      EntryFn entry
    );

    ~Handle();
  };

  struct Pipeline {
    std::unique_ptr<llvm::TargetMachine> machine;
    llvm::LLVMContext context;
    Linker backend;

    Pipeline();

    std::unique_ptr<JIT::Handle> compile(IR::Graph *graph);
  };
}