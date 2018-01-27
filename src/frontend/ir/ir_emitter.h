/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include "common/common_types.h"
#include "frontend/ir/basic_block.h"
#include "frontend/ir/location_descriptor.h"
#include "frontend/ir/terminal.h"
#include "frontend/ir/value.h"

// ARM JIT Microinstruction Intermediate Representation
//
// This intermediate representation is an SSA IR. It is designed primarily for analysis,
// though it can be lowered into a reduced form for interpretation. Each IR node (Value)
// is a microinstruction of an idealised ARM CPU. The choice of microinstructions is made
// not based on any existing microarchitecture but on ease of implementation.

namespace Dynarmic::IR {

enum class Opcode;

template <typename T>
struct ResultAndCarry {
    T result;
    U1 carry;
};

template <typename T>
struct ResultAndOverflow {
    T result;
    U1 overflow;
};

template <typename T>
struct ResultAndCarryAndOverflow {
    T result;
    U1 carry;
    U1 overflow;
};

template <typename T>
struct ResultAndGE {
    T result;
    U32 ge;
};

/**
 * Convenience class to construct a basic block of the intermediate representation.
 * `block` is the resulting block.
 * The user of this class updates `current_location` as appropriate.
 */
class IREmitter {
public:
    explicit IREmitter(Block& block) : block(block), insertion_point(block.end()) {}

    Block& block;

    U1 Imm1(bool value);
    U8 Imm8(u8 value);
    U32 Imm32(u32 value);
    U64 Imm64(u64 value);

    void PushRSB(const LocationDescriptor& return_location);

    U64 Pack2x32To1x64(const U32& lo, const U32& hi);
    U32 LeastSignificantWord(const U64& value);
    ResultAndCarry<U32> MostSignificantWord(const U64& value);
    U16 LeastSignificantHalf(U32U64 value);
    U8 LeastSignificantByte(U32U64 value);
    U1 MostSignificantBit(const U32& value);
    U1 IsZero(const U32& value);
    U1 IsZero(const U64& value);
    U1 IsZero(const U32U64& value);
    U1 TestBit(const U32U64& value, const U8& bit);
    U32 ConditionalSelect(Cond cond, const U32& a, const U32& b);
    U64 ConditionalSelect(Cond cond, const U64& a, const U64& b);
    U32U64 ConditionalSelect(Cond cond, const U32U64& a, const U32U64& b);

    // This pseudo-instruction may only be added to instructions that support it.
    NZCV NZCVFrom(const Value& value);

    ResultAndCarry<U32> LogicalShiftLeft(const U32& value_in, const U8& shift_amount, const U1& carry_in);
    ResultAndCarry<U32> LogicalShiftRight(const U32& value_in, const U8& shift_amount, const U1& carry_in);
    ResultAndCarry<U32> ArithmeticShiftRight(const U32& value_in, const U8& shift_amount, const U1& carry_in);
    ResultAndCarry<U32> RotateRight(const U32& value_in, const U8& shift_amount, const U1& carry_in);
    U32 LogicalShiftLeft(const U32& value_in, const U8& shift_amount);
    U64 LogicalShiftLeft(const U64& value_in, const U8& shift_amount);
    U32U64 LogicalShiftLeft(const U32U64& value_in, const U8& shift_amount);
    U32 LogicalShiftRight(const U32& value_in, const U8& shift_amount);
    U64 LogicalShiftRight(const U64& value_in, const U8& shift_amount);
    U32U64 LogicalShiftRight(const U32U64& value_in, const U8& shift_amount);
    U32U64 ArithmeticShiftRight(const U32U64& value_in, const U8& shift_amount);
    U32U64 RotateRight(const U32U64& value_in, const U8& shift_amount);
    ResultAndCarry<U32> RotateRightExtended(const U32& value_in, const U1& carry_in);
    ResultAndCarryAndOverflow<U32> AddWithCarry(const U32& a, const U32& b, const U1& carry_in);
    ResultAndCarryAndOverflow<U32> SubWithCarry(const U32& a, const U32& b, const U1& carry_in);
    U32U64 AddWithCarry(const U32U64& a, const U32U64& b, const U1& carry_in);
    U32U64 SubWithCarry(const U32U64& a, const U32U64& b, const U1& carry_in);
    U32 Add(const U32& a, const U32& b);
    U64 Add(const U64& a, const U64& b);
    U32U64 Add(const U32U64& a, const U32U64& b);
    U32 Sub(const U32& a, const U32& b);
    U64 Sub(const U64& a, const U64& b);
    U32U64 Sub(const U32U64& a, const U32U64& b);
    U32 Mul(const U32& a, const U32& b);
    U64 Mul(const U64& a, const U64& b);
    U32U64 Mul(const U32U64& a, const U32U64& b);
    U64 UnsignedMultiplyHigh(const U64& a, const U64& b);
    U64 SignedMultiplyHigh(const U64& a, const U64& b);
    U32 UnsignedDiv(const U32& a, const U32& b);
    U64 UnsignedDiv(const U64& a, const U64& b);
    U32U64 UnsignedDiv(const U32U64& a, const U32U64& b);
    U32 SignedDiv(const U32& a, const U32& b);
    U64 SignedDiv(const U64& a, const U64& b);
    U32U64 SignedDiv(const U32U64& a, const U32U64& b);
    U32 And(const U32& a, const U32& b);
    U32U64 And(const U32U64& a, const U32U64& b);
    U32 Eor(const U32& a, const U32& b);
    U32U64 Eor(const U32U64& a, const U32U64& b);
    U32 Or(const U32& a, const U32& b);
    U32U64 Or(const U32U64& a, const U32U64& b);
    U32 Not(const U32& a);
    U32U64 Not(const U32U64& a);
    U32 SignExtendToWord(const UAny& a);
    U64 SignExtendToLong(const UAny& a);
    U32 SignExtendByteToWord(const U8& a);
    U32 SignExtendHalfToWord(const U16& a);
    U64 SignExtendWordToLong(const U32& a);
    U32 ZeroExtendToWord(const UAny& a);
    U64 ZeroExtendToLong(const UAny& a);
    U128 ZeroExtendToQuad(const UAny& a);
    U32 ZeroExtendByteToWord(const U8& a);
    U32 ZeroExtendHalfToWord(const U16& a);
    U64 ZeroExtendWordToLong(const U32& a);
    U32 IndeterminateExtendToWord(const UAny& a);
    U64 IndeterminateExtendToLong(const UAny& a);
    U32 ByteReverseWord(const U32& a);
    U16 ByteReverseHalf(const U16& a);
    U64 ByteReverseDual(const U64& a);
    U32 CountLeadingZeros(const U32& a);
    U64 CountLeadingZeros(const U64& a);
    U32U64 CountLeadingZeros(const U32U64& a);
    U32 ExtractRegister(const U32& a, const U32& b, const U8& lsb);
    U64 ExtractRegister(const U64& a, const U64& b, const U8& lsb);
    U32U64 ExtractRegister(const U32U64& a, const U32U64& b, const U8& lsb);

    ResultAndOverflow<U32> SignedSaturatedAdd(const U32& a, const U32& b);
    ResultAndOverflow<U32> SignedSaturatedSub(const U32& a, const U32& b);
    ResultAndOverflow<U32> UnsignedSaturation(const U32& a, size_t bit_size_to_saturate_to);
    ResultAndOverflow<U32> SignedSaturation(const U32& a, size_t bit_size_to_saturate_to);

    ResultAndGE<U32> PackedAddU8(const U32& a, const U32& b);
    ResultAndGE<U32> PackedAddS8(const U32& a, const U32& b);
    ResultAndGE<U32> PackedAddU16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedAddS16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubU8(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubS8(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubU16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubS16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedAddSubU16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedAddSubS16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubAddU16(const U32& a, const U32& b);
    ResultAndGE<U32> PackedSubAddS16(const U32& a, const U32& b);
    U32 PackedHalvingAddU8(const U32& a, const U32& b);
    U32 PackedHalvingAddS8(const U32& a, const U32& b);
    U32 PackedHalvingSubU8(const U32& a, const U32& b);
    U32 PackedHalvingSubS8(const U32& a, const U32& b);
    U32 PackedHalvingAddU16(const U32& a, const U32& b);
    U32 PackedHalvingAddS16(const U32& a, const U32& b);
    U32 PackedHalvingSubU16(const U32& a, const U32& b);
    U32 PackedHalvingSubS16(const U32& a, const U32& b);
    U32 PackedHalvingAddSubU16(const U32& a, const U32& b);
    U32 PackedHalvingAddSubS16(const U32& a, const U32& b);
    U32 PackedHalvingSubAddU16(const U32& a, const U32& b);
    U32 PackedHalvingSubAddS16(const U32& a, const U32& b);
    U32 PackedSaturatedAddU8(const U32& a, const U32& b);
    U32 PackedSaturatedAddS8(const U32& a, const U32& b);
    U32 PackedSaturatedSubU8(const U32& a, const U32& b);
    U32 PackedSaturatedSubS8(const U32& a, const U32& b);
    U32 PackedSaturatedAddU16(const U32& a, const U32& b);
    U32 PackedSaturatedAddS16(const U32& a, const U32& b);
    U32 PackedSaturatedSubU16(const U32& a, const U32& b);
    U32 PackedSaturatedSubS16(const U32& a, const U32& b);
    U32 PackedAbsDiffSumS8(const U32& a, const U32& b);
    U32 PackedSelect(const U32& ge, const U32& a, const U32& b);

    UAny VectorGetElement(size_t esize, const U128& a, size_t index);
    U128 VectorAdd8(const U128& a, const U128& b);
    U128 VectorAdd16(const U128& a, const U128& b);
    U128 VectorAdd32(const U128& a, const U128& b);
    U128 VectorAdd64(const U128& a, const U128& b);
    U128 VectorAnd(const U128& a, const U128& b);
    U128 VectorOr(const U128& a, const U128& b);
    U128 VectorEor(const U128& a, const U128& b);
    U128 VectorNot(const U128& a);
    U128 VectorLowerBroadcast8(const U8& a);
    U128 VectorLowerBroadcast16(const U16& a);
    U128 VectorLowerBroadcast32(const U32& a);
    U128 VectorBroadcast8(const U8& a);
    U128 VectorBroadcast16(const U16& a);
    U128 VectorBroadcast32(const U32& a);
    U128 VectorBroadcast64(const U64& a);
    U128 VectorEqual8(const U128& a, const U128& b);
    U128 VectorEqual16(const U128& a, const U128& b);
    U128 VectorEqual32(const U128& a, const U128& b);
    U128 VectorEqual64(const U128& a, const U128& b);
    U128 VectorEqual128(const U128& a, const U128& b);
    U128 VectorLowerPairedAdd8(const U128& a, const U128& b);
    U128 VectorLowerPairedAdd16(const U128& a, const U128& b);
    U128 VectorLowerPairedAdd32(const U128& a, const U128& b);
    U128 VectorPairedAdd8(const U128& a, const U128& b);
    U128 VectorPairedAdd16(const U128& a, const U128& b);
    U128 VectorPairedAdd32(const U128& a, const U128& b);
    U128 VectorPairedAdd64(const U128& a, const U128& b);
    U128 VectorZeroUpper(const U128& a);

    U32 FPAbs32(const U32& a);
    U64 FPAbs64(const U64& a);
    U32 FPAdd32(const U32& a, const U32& b, bool fpscr_controlled);
    U64 FPAdd64(const U64& a, const U64& b, bool fpscr_controlled);
    void FPCompare32(const U32& a, const U32& b, bool exc_on_qnan, bool fpscr_controlled);
    void FPCompare64(const U64& a, const U64& b, bool exc_on_qnan, bool fpscr_controlled);
    U32 FPDiv32(const U32& a, const U32& b, bool fpscr_controlled);
    U64 FPDiv64(const U64& a, const U64& b, bool fpscr_controlled);
    U32 FPMul32(const U32& a, const U32& b, bool fpscr_controlled);
    U64 FPMul64(const U64& a, const U64& b, bool fpscr_controlled);
    U32 FPNeg32(const U32& a);
    U64 FPNeg64(const U64& a);
    U32 FPSqrt32(const U32& a);
    U64 FPSqrt64(const U64& a);
    U32 FPSub32(const U32& a, const U32& b, bool fpscr_controlled);
    U64 FPSub64(const U64& a, const U64& b, bool fpscr_controlled);
    U32 FPDoubleToSingle(const U64& a, bool fpscr_controlled);
    U64 FPSingleToDouble(const U32& a, bool fpscr_controlled);
    U32 FPSingleToS32(const U32& a, bool round_towards_zero, bool fpscr_controlled);
    U32 FPSingleToU32(const U32& a, bool round_towards_zero, bool fpscr_controlled);
    U32 FPDoubleToS32(const U32& a, bool round_towards_zero, bool fpscr_controlled);
    U32 FPDoubleToU32(const U32& a, bool round_towards_zero, bool fpscr_controlled);
    U32 FPS32ToSingle(const U32& a, bool round_to_nearest, bool fpscr_controlled);
    U32 FPU32ToSingle(const U32& a, bool round_to_nearest, bool fpscr_controlled);
    U64 FPS32ToDouble(const U32& a, bool round_to_nearest, bool fpscr_controlled);
    U64 FPU32ToDouble(const U32& a, bool round_to_nearest, bool fpscr_controlled);

    void Breakpoint();

    void SetTerm(const Terminal& terminal);

    void SetInsertionPoint(IR::Inst* new_insertion_point);
    void SetInsertionPoint(IR::Block::iterator new_insertion_point);

protected:
    IR::Block::iterator insertion_point;

    template<typename T = Value, typename ...Args>
    T Inst(Opcode op, Args ...args) {
        auto iter = block.PrependNewInst(insertion_point, op, {Value(args)...});
        return T(Value(&*iter));
    }
};

} // namespace Dynarmic::IR
