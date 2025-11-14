SignExt
=======

Because RISC V supports instructions in multiples of 16-bits (two bytes), we cannot assume the next-to-LSB is also zero, as it would be with MIPS (which has 32-bit instructions).

The register operand in JAL is optionally used to capture the return address in addition to performing the branch.

JAL's function is to perform modestly far pc-relative branches or calls using its 20-bit range.  (Contrast with RISC V conditional branch instructions that have only have 12 bits for +/- 4 KB range.)

JALR has a 12 bit offset and two registers as operands.

Its operation is pc := ( rs1 + sxt ( imm12 ) ) & -2.

As you can see by the formula, the branch is register indirect, relative to the value in rs1.

Like JAL, JALR can also capture the the return address.

JALR is used to return from a function (aka RET in assembly.  In this form $ra is used as the source register, and no return address is captured).  This uses zero for the offset (i.e. an offset is not required).

JALR is also used to perform indirect function calls: calls via function pointer, virtual method dispatches, etc..   These use also use zero for the offset.

JALR can also be used in sequence with AUIPC.

AUIPC has a 20 bit offset and a register as operands.

Its operation is rd := pc + ( imm20 << 12 ).

It computes the upper part of a pc-relative immediate (while also providing a lower part of the pc that is not relative).

Combined with JALR, this can accomplish a 32-bit pc-relative branch or call.

AUIPC r5, labelFarAway      # AUIPC encodes upper 20 bits of label's distance from pc
JALR r5, $ra, labelFarAway  # JALR encodes the lower 12 bits of same

