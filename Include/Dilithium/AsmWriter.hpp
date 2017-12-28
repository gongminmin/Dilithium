/**
 * @file AsmWriter.hpp
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

#ifndef _DILITHIUM_ASM_WRITER_HPP
#define _DILITHIUM_ASM_WRITER_HPP

#include <iosfwd>

namespace Dilithium
{
	class BasicBlock;
	class Function;
	class Instruction;
	class Value;

	class AssemblyAnnotationWriter
	{
	public:
		virtual ~AssemblyAnnotationWriter();

		/// EmitFunctionAnnot - This may be implemented to emit a string right before
		/// the start of a function.
		virtual void EmitFunctionAnnot(Function const * func, std::ostream& os);

		/// EmitBasicBlockStartAnnot - This may be implemented to emit a string right
		/// after the basic block label, but before the first instruction in the
		/// block.
		virtual void EmitBasicBlockStartAnnot(BasicBlock const * bb, std::ostream& os);

		/// EmitBasicBlockEndAnnot - This may be implemented to emit a string right
		/// after the basic block.
		virtual void EmitBasicBlockEndAnnot(BasicBlock const * bb, std::ostream& os);

		/// EmitInstructionAnnot - This may be implemented to emit a string right
		/// before an instruction is emitted.
		virtual void EmitInstructionAnnot(Instruction const * inst, std::ostream& os);

		/// PrintInfoComment - This may be implemented to emit a comment to the
		/// right of an instruction or global value.
		virtual void PrintInfoComment(Value const & value, std::ostream& os);
	};
}

#endif		// _DILITHIUM_ASM_WRITER_HPP

