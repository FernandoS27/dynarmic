/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include "frontend/ir/ir_emitter.h"

namespace Dynarmic {
namespace Arm {

enum class ConditionalState {
    /// We haven't met any conditional instructions yet.
    None,
    /// Current instruction is a conditional. This marks the end of this basic block.
    Break,
    /// This basic block is made up solely of conditional instructions.
    Translating,
};

struct ArmTranslatorVisitor final {
    explicit ArmTranslatorVisitor(LocationDescriptor descriptor) : ir(descriptor) {
        ASSERT_MSG(!descriptor.TFlag(), "The processor must be in Arm mode");
    }

    IREmitter ir;
    ConditionalState cond_state = ConditionalState::None;

    bool ConditionPassed(Cond cond);
    bool InterpretThisInstruction();
    bool UnpredictableInstruction();
    bool LinkToNextInstruction();

    static u32 rotr(u32 x, int shift) {
        shift &= 31;
        if (!shift) return x;
        return (x >> shift) | (x << (32 - shift));
    }

    static u32 ArmExpandImm(int rotate, Imm8 imm8) {
        return rotr(static_cast<u32>(imm8), rotate*2);
    }

    struct ImmAndCarry {
        u32 imm32;
        IR::Value carry;
    };

    ImmAndCarry ArmExpandImm_C(int rotate, u32 imm8, IR::Value carry_in) {
        u32 imm32 = imm8;
        auto carry_out = carry_in;
        if (rotate) {
            imm32 = rotr(imm8, rotate * 2);
            carry_out = ir.Imm1(imm32 >> 31 == 1);
        }
        return {imm32, carry_out};
    }

    IREmitter::ResultAndCarry EmitImmShift(IR::Value value, ShiftType type, Imm5 imm5, IR::Value carry_in);
    IREmitter::ResultAndCarry EmitRegShift(IR::Value value, ShiftType type, IR::Value amount, IR::Value carry_in);
    IR::Value SignZeroExtendRor(Reg m, SignExtendRotation rotate);

    // Branch instructions
    bool arm_B(Cond cond, Imm24 imm24);
    bool arm_BL(Cond cond, Imm24 imm24);
    bool arm_BLX_imm(bool H, Imm24 imm24);
    bool arm_BLX_reg(Cond cond, Reg m);
    bool arm_BX(Cond cond, Reg m);
    bool arm_BXJ(Cond cond, Reg m);

    // Coprocessor instructions
    bool arm_CDP() { return InterpretThisInstruction(); }
    bool arm_LDC() { return InterpretThisInstruction(); }
    bool arm_MCR() { return InterpretThisInstruction(); }
    bool arm_MCRR() { return InterpretThisInstruction(); }
    bool arm_MRC() { return InterpretThisInstruction(); }
    bool arm_MRRC() { return InterpretThisInstruction(); }
    bool arm_STC() { return InterpretThisInstruction(); }

    // Data processing instructions
    bool arm_ADC_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_ADC_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_ADC_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_ADD_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_ADD_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_ADD_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_AND_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_AND_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_AND_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_BIC_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_BIC_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_BIC_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_CMN_imm(Cond cond, Reg n, int rotate, Imm8 imm8);
    bool arm_CMN_reg(Cond cond, Reg n, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_CMN_rsr(Cond cond, Reg n, Reg s, ShiftType shift, Reg m);
    bool arm_CMP_imm(Cond cond, Reg n, int rotate, Imm8 imm8);
    bool arm_CMP_reg(Cond cond, Reg n, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_CMP_rsr(Cond cond, Reg n, Reg s, ShiftType shift, Reg m);
    bool arm_EOR_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_EOR_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_EOR_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_MOV_imm(Cond cond, bool S, Reg d, int rotate, Imm8 imm8);
    bool arm_MOV_reg(Cond cond, bool S, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_MOV_rsr(Cond cond, bool S, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_MVN_imm(Cond cond, bool S, Reg d, int rotate, Imm8 imm8);
    bool arm_MVN_reg(Cond cond, bool S, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_MVN_rsr(Cond cond, bool S, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_ORR_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_ORR_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_ORR_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_RSB_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_RSB_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_RSB_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_RSC_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_RSC_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_RSC_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_SBC_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_SBC_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_SBC_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_SUB_imm(Cond cond, bool S, Reg n, Reg d, int rotate, Imm8 imm8);
    bool arm_SUB_reg(Cond cond, bool S, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_SUB_rsr(Cond cond, bool S, Reg n, Reg d, Reg s, ShiftType shift, Reg m);
    bool arm_TEQ_imm(Cond cond, Reg n, int rotate, Imm8 imm8);
    bool arm_TEQ_reg(Cond cond, Reg n, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_TEQ_rsr(Cond cond, Reg n, Reg s, ShiftType shift, Reg m);
    bool arm_TST_imm(Cond cond, Reg n, int rotate, Imm8 imm8);
    bool arm_TST_reg(Cond cond, Reg n, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_TST_rsr(Cond cond, Reg n, Reg s, ShiftType shift, Reg m);

    // Exception generating instructions
    bool arm_BKPT(Cond cond, Imm12 imm12, Imm4 imm4);
    bool arm_SVC(Cond cond, Imm24 imm24);
    bool arm_UDF();

    // Extension instructions
    bool arm_SXTAB(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_SXTAB16(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_SXTAH(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_SXTB(Cond cond, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_SXTB16(Cond cond, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_SXTH(Cond cond, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTAB(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTAB16(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTAH(Cond cond, Reg n, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTB(Cond cond, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTB16(Cond cond, Reg d, SignExtendRotation rotate, Reg m);
    bool arm_UXTH(Cond cond, Reg d, SignExtendRotation rotate, Reg m);

    // Hint instructions
    bool arm_PLD() { return true; }
    bool arm_SEV() { return true; }
    bool arm_WFE() { return true; }
    bool arm_WFI() { return true; }
    bool arm_YIELD() { return true; }

    // Load/Store instructions
    bool arm_LDR_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm12 imm12);
    bool arm_LDR_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_LDRB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm12 imm12);
    bool arm_LDRB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_LDRBT();
    bool arm_LDRD_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_LDRD_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_LDRH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_LDRH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_LDRHT();
    bool arm_LDRSB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_LDRSB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_LDRSBT();
    bool arm_LDRSH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_LDRSH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_LDRSHT();
    bool arm_LDRT();
    bool arm_STR_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm12 imm12);
    bool arm_STR_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_STRB_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm12 imm12);
    bool arm_STRB_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm5 imm5, ShiftType shift, Reg m);
    bool arm_STRBT();
    bool arm_STRD_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_STRD_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_STRH_imm(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Imm4 imm8a, Imm4 imm8b);
    bool arm_STRH_reg(Cond cond, bool P, bool U, bool W, Reg n, Reg d, Reg m);
    bool arm_STRHT();
    bool arm_STRT();

    // Load/Store multiple instructions
    bool arm_LDM(Cond cond, bool W, Reg n, RegList list);
    bool arm_LDMDA(Cond cond, bool W, Reg n, RegList list);
    bool arm_LDMDB(Cond cond, bool W, Reg n, RegList list);
    bool arm_LDMIB(Cond cond, bool W, Reg n, RegList list);
    bool arm_LDM_usr();
    bool arm_LDM_eret();
    bool arm_STM(Cond cond, bool W, Reg n, RegList list);
    bool arm_STMDA(Cond cond, bool W, Reg n, RegList list);
    bool arm_STMDB(Cond cond, bool W, Reg n, RegList list);
    bool arm_STMIB(Cond cond, bool W, Reg n, RegList list);
    bool arm_STM_usr();

    // Miscellaneous instructions
    bool arm_CLZ(Cond cond, Reg d, Reg m) { return InterpretThisInstruction(); }
    bool arm_NOP() { return true; }
    bool arm_SEL(Cond cond, Reg n, Reg d, Reg m) { return InterpretThisInstruction(); }

    // Unsigned sum of absolute difference functions
    bool arm_USAD8(Cond cond, Reg d, Reg m, Reg n) { return InterpretThisInstruction(); }
    bool arm_USADA8(Cond cond, Reg d, Reg a, Reg m, Reg n) { return InterpretThisInstruction(); }

    // Packing instructions
    bool arm_PKHBT(Cond cond, Reg n, Reg d, Imm5 imm5, Reg m) { return InterpretThisInstruction(); }
    bool arm_PKHTB(Cond cond, Reg n, Reg d, Imm5 imm5, Reg m) { return InterpretThisInstruction(); }

    // Reversal instructions
    bool arm_REV(Cond cond, Reg d, Reg m);
    bool arm_REV16(Cond cond, Reg d, Reg m);
    bool arm_REVSH(Cond cond, Reg d, Reg m);

    // Saturation instructions
    bool arm_SSAT(Cond cond, Imm5 sat_imm, Reg d, Imm5 imm5, bool sh, Reg n) { return InterpretThisInstruction(); }
    bool arm_SSAT16(Cond cond, Imm4 sat_imm, Reg d, Reg n) { return InterpretThisInstruction(); }
    bool arm_USAT(Cond cond, Imm5 sat_imm, Reg d, Imm5 imm5, bool sh, Reg n) { return InterpretThisInstruction(); }
    bool arm_USAT16(Cond cond, Imm4 sat_imm, Reg d, Reg n) { return InterpretThisInstruction(); }

    // Multiply (Normal) instructions
    bool arm_MLA(Cond cond, bool S, Reg d, Reg a, Reg m, Reg n);
    bool arm_MUL(Cond cond, bool S, Reg d, Reg m, Reg n);

    // Multiply (Long) instructions
    bool arm_SMLAL(Cond cond, bool S, Reg dHi, Reg dLo, Reg m, Reg n);
    bool arm_SMULL(Cond cond, bool S, Reg dHi, Reg dLo, Reg m, Reg n);
    bool arm_UMAAL(Cond cond, Reg dHi, Reg dLo, Reg m, Reg n);
    bool arm_UMLAL(Cond cond, bool S, Reg dHi, Reg dLo, Reg m, Reg n);
    bool arm_UMULL(Cond cond, bool S, Reg dHi, Reg dLo, Reg m, Reg n);

    // Multiply (Halfword) instructions
    bool arm_SMLALxy(Cond cond, Reg dHi, Reg dLo, Reg m, bool M, bool N, Reg n);
    bool arm_SMLAxy(Cond cond, Reg d, Reg a, Reg m, bool M, bool N, Reg n);
    bool arm_SMULxy(Cond cond, Reg d, Reg m, bool M, bool N, Reg n);

    // Multiply (word by halfword) instructions
    bool arm_SMLAWy(Cond cond, Reg d, Reg a, Reg m, bool M, Reg n);
    bool arm_SMULWy(Cond cond, Reg d, Reg m, bool M, Reg n);

    // Multiply (Most significant word) instructions
    bool arm_SMMLA(Cond cond, Reg d, Reg a, Reg m, bool R, Reg n);
    bool arm_SMMLS(Cond cond, Reg d, Reg a, Reg m, bool R, Reg n);
    bool arm_SMMUL(Cond cond, Reg d, Reg m, bool R, Reg n);

    // Multiply (Dual) instructions
    bool arm_SMLAD(Cond cond, Reg d, Reg a, Reg m, bool M, Reg n);
    bool arm_SMLALD(Cond cond, Reg dHi, Reg dLo, Reg m, bool M, Reg n);
    bool arm_SMLSD(Cond cond, Reg d, Reg a, Reg m, bool M, Reg n);
    bool arm_SMLSLD(Cond cond, Reg dHi, Reg dLo, Reg m, bool M, Reg n);
    bool arm_SMUAD(Cond cond, Reg d, Reg m, bool M, Reg n);
    bool arm_SMUSD(Cond cond, Reg d, Reg m, bool M, Reg n);

    // Parallel Add/Subtract (Modulo arithmetic) instructions
    bool arm_SADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SSAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SSUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SSUB16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_USAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_USUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_USUB16(Cond cond, Reg n, Reg d, Reg m);

    // Parallel Add/Subtract (Saturating) instructions
    bool arm_QADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_QADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_QASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_QSAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_QSUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_QSUB16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQSAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQSUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UQSUB16(Cond cond, Reg n, Reg d, Reg m);

    // Parallel Add/Subtract (Halving) instructions
    bool arm_SHADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SHADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SHASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SHSAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SHSUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SHSUB16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHADD8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHADD16(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHASX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHSAX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHSUB8(Cond cond, Reg n, Reg d, Reg m);
    bool arm_UHSUB16(Cond cond, Reg n, Reg d, Reg m);

    // Saturated Add/Subtract instructions
    bool arm_QADD(Cond cond, Reg n, Reg d, Reg m) { return InterpretThisInstruction(); }
    bool arm_QSUB(Cond cond, Reg n, Reg d, Reg m) { return InterpretThisInstruction(); }
    bool arm_QDADD(Cond cond, Reg n, Reg d, Reg m) { return InterpretThisInstruction(); }
    bool arm_QDSUB(Cond cond, Reg n, Reg d, Reg m) { return InterpretThisInstruction(); }

    // Synchronization Primitive instructions
    bool arm_CLREX();
    bool arm_LDREX(Cond cond, Reg n, Reg d);
    bool arm_LDREXB(Cond cond, Reg n, Reg d);
    bool arm_LDREXD(Cond cond, Reg n, Reg d);
    bool arm_LDREXH(Cond cond, Reg n, Reg d);
    bool arm_STREX(Cond cond, Reg n, Reg d, Reg m);
    bool arm_STREXB(Cond cond, Reg n, Reg d, Reg m);
    bool arm_STREXD(Cond cond, Reg n, Reg d, Reg m);
    bool arm_STREXH(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SWP(Cond cond, Reg n, Reg d, Reg m);
    bool arm_SWPB(Cond cond, Reg n, Reg d, Reg m);

    // Status register access instructions
    bool arm_CPS();
    bool arm_MRS(Cond cond, Reg d);
    bool arm_MSR_imm(Cond cond, int mask, int rotate, Imm8 imm8);
    bool arm_MSR_reg(Cond cond, int mask, Reg n);
    bool arm_RFE();
    bool arm_SETEND(bool E);
    bool arm_SRS();

    // Floating-point three-register data processing instructions
    bool vfp2_VADD(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VSUB(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VMUL(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VMLA(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VMLS(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VNMUL(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VNMLA(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VNMLS(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);
    bool vfp2_VDIV(Cond cond, bool D, size_t Vn, size_t Vd, bool sz, bool N, bool M, size_t Vm);

    // Floating-point move instructions
    bool vfp2_VMOV_u32_f64(Cond cond, size_t Vd, Reg t, bool D);
    bool vfp2_VMOV_f64_u32(Cond cond, size_t Vn, Reg t, bool N);
    bool vfp2_VMOV_u32_f32(Cond cond, size_t Vn, Reg t, bool N);
    bool vfp2_VMOV_f32_u32(Cond cond, size_t Vn, Reg t, bool N);
    bool vfp2_VMOV_2u32_2f32(Cond cond, Reg t2, Reg t, bool M, size_t Vm);
    bool vfp2_VMOV_2f32_2u32(Cond cond, Reg t2, Reg t, bool M, size_t Vm);
    bool vfp2_VMOV_2u32_f64(Cond cond, Reg t2, Reg t, bool M, size_t Vm);
    bool vfp2_VMOV_f64_2u32(Cond cond, Reg t2, Reg t, bool M, size_t Vm);
    bool vfp2_VMOV_reg(Cond cond, bool D, size_t Vd, bool sz, bool M, size_t Vm);

    // Floating-point misc instructions
    bool vfp2_VABS(Cond cond, bool D, size_t Vd, bool sz, bool M, size_t Vm);
    bool vfp2_VNEG(Cond cond, bool D, size_t Vd, bool sz, bool M, size_t Vm);
    bool vfp2_VSQRT(Cond cond, bool D, size_t Vd, bool sz, bool M, size_t Vm);
    bool vfp2_VPOP(Cond cond, bool D, size_t Vd, bool sz, Imm8 imm8);
    bool vfp2_VPUSH(Cond cond, bool D, size_t Vd, bool sz, Imm8 imm8);

    // Floating-point load-store instructions
    bool vfp2_VLDR(Cond cond, bool U, bool D, Reg n, size_t Vd, bool sz, Imm8 imm8);
    bool vfp2_VSTR(Cond cond, bool U, bool D, Reg n, size_t Vd, bool sz, Imm8 imm8);
};

} // namespace Arm
} // namespace Dynarmic
