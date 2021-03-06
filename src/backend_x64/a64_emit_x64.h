/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include "backend_x64/a64_jitstate.h"
#include "backend_x64/block_range_information.h"
#include "backend_x64/emit_x64.h"
#include "dynarmic/A64/config.h"
#include "frontend/A64/location_descriptor.h"
#include "frontend/ir/terminal.h"

namespace Dynarmic {
namespace BackendX64 {

class RegAlloc;

struct A64EmitContext final : public EmitContext {
    A64EmitContext(RegAlloc& reg_alloc, IR::Block& block);
    A64::LocationDescriptor Location() const;
    bool FPSCR_RoundTowardsZero() const override;
    bool FPSCR_FTZ() const override;
    bool FPSCR_DN() const override;
};

class A64EmitX64 final : public EmitX64 {
public:
    A64EmitX64(BlockOfCode* code, A64::UserConfig conf);
    ~A64EmitX64();

    /**
     * Emit host machine code for a basic block with intermediate representation `ir`.
     * @note ir is modified.
     */
    BlockDescriptor Emit(IR::Block& ir);

    void ClearCache() override;

    void InvalidateCacheRanges(const boost::icl::interval_set<u64>& ranges);

protected:
    const A64::UserConfig conf;
    BlockRangeInformation<u64> block_ranges;

    // Microinstruction emitters
#define OPCODE(...)
#define A32OPC(...)
#define A64OPC(name, type, ...) void EmitA64##name(A64EmitContext& ctx, IR::Inst* inst);
#include "frontend/ir/opcodes.inc"
#undef OPCODE
#undef A32OPC
#undef A64OPC

    // Terminal instruction emitters
    void EmitTerminalImpl(IR::Term::Interpret terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::ReturnToDispatch terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::LinkBlock terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::LinkBlockFast terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::PopRSBHint terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::If terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::CheckBit terminal, IR::LocationDescriptor initial_location) override;
    void EmitTerminalImpl(IR::Term::CheckHalt terminal, IR::LocationDescriptor initial_location) override;

    // Patching
    void EmitPatchJg(const IR::LocationDescriptor& target_desc, CodePtr target_code_ptr = nullptr) override;
    void EmitPatchJmp(const IR::LocationDescriptor& target_desc, CodePtr target_code_ptr = nullptr) override;
    void EmitPatchMovRcx(CodePtr target_code_ptr = nullptr) override;
};

} // namespace BackendX64
} // namespace Dynarmic
