/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include "frontend/A64/translate/impl/impl.h"

namespace Dynarmic {
namespace A64 {

bool TranslatorVisitor::SVC(Imm<16> imm16) {
	printf("translator %x\n", imm16.ZeroExtend());
    // ir.PushRSB(ir.current_location.AdvancePC(4)); // TODO
    ir.SetPC(ir.Imm64(ir.current_location.PC() + 4));
    ir.CallSupervisor(imm16.ZeroExtend());
    ir.SetTerm(IR::Term::CheckHalt{IR::Term::PopRSBHint{}});
    return false;
}

} // namespace A64
} // namespace Dynarmic