/**
 * @file Value.inc
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of Dilithium
 * For the latest info, see https://github.com/gongminmin/Dilithium
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Minmin Gong. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Provide definitions of macros so that users of this file do not have to
// define everything to use it...
//
#if !(defined HANDLE_GLOBAL_VALUE || defined HANDLE_CONSTANT ||                \
	  defined HANDLE_INSTRUCTION ||                                            \
	  defined HANDLE_METADATA_VALUE || defined HANDLE_VALUE ||                 \
	  defined HANDLE_CONSTANT_MARKER)
	#error "Missing macro definition of HANDLE_VALUE*"
#endif

#ifndef HANDLE_GLOBAL_VALUE
	#define HANDLE_GLOBAL_VALUE(ValueName) HANDLE_CONSTANT(ValueName)
#endif

#ifndef HANDLE_CONSTANT
	#define HANDLE_CONSTANT(ValueName) HANDLE_VALUE(ValueName)
#endif

#ifndef HANDLE_INSTRUCTION
	#define HANDLE_INSTRUCTION(ValueName) HANDLE_VALUE(ValueName)
#endif

#ifndef HANDLE_METADATA_VALUE
	#define HANDLE_METADATA_VALUE(ValueName) HANDLE_VALUE(ValueName)
#endif

#ifndef HANDLE_VALUE
	#define HANDLE_VALUE(ValueName)
#endif

#ifndef HANDLE_CONSTANT_MARKER
	#define HANDLE_CONSTANT_MARKER(MarkerName, ValueName)
#endif

HANDLE_VALUE(Argument)
HANDLE_VALUE(BasicBlock)

HANDLE_GLOBAL_VALUE(Function)
HANDLE_GLOBAL_VALUE(GlobalVariable)
HANDLE_CONSTANT(UndefValue)
HANDLE_CONSTANT(BlockAddress)
HANDLE_CONSTANT(ConstantExpr)
HANDLE_CONSTANT(ConstantAggregateZero)
HANDLE_CONSTANT(ConstantDataArray)
HANDLE_CONSTANT(ConstantDataVector)
HANDLE_CONSTANT(ConstantInt)
HANDLE_CONSTANT(ConstantFP)
HANDLE_CONSTANT(ConstantArray)
HANDLE_CONSTANT(ConstantStruct)
HANDLE_CONSTANT(ConstantVector)
HANDLE_CONSTANT(ConstantPointerNull)

HANDLE_METADATA_VALUE(MetadataAsValue)

HANDLE_INSTRUCTION(Instruction)
// Enum values starting at InstructionVal are used for Instructions;
// don't add new values here!

HANDLE_CONSTANT_MARKER(ConstantFirstVal, Function)
HANDLE_CONSTANT_MARKER(ConstantLastVal, ConstantPointerNull)

#undef HANDLE_GLOBAL_VALUE
#undef HANDLE_CONSTANT
#undef HANDLE_INSTRUCTION
#undef HANDLE_METADATA_VALUE
#undef HANDLE_INLINE_ASM_VALUE
#undef HANDLE_VALUE
#undef HANDLE_CONSTANT_MARKER
