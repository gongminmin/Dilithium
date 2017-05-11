/**
 * @file Function.cpp
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

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/Function.hpp>

namespace Dilithium 
{
	Function* Function::Create(FunctionType* ty, LinkageTypes linkage, std::string_view name, LLVMModule* mod)
	{
		DILITHIUM_UNUSED(ty);
		DILITHIUM_UNUSED(linkage);
		DILITHIUM_UNUSED(name);
		DILITHIUM_UNUSED(mod);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Function::HasPersonalityFn() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* Function::PersonalityFn() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Function::PersonalityFn(Constant* c)
	{
		DILITHIUM_UNUSED(c);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Function::IsVarArg() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Function::IsMaterializable() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Function::IsMaterializable(bool m)
	{
		DILITHIUM_UNUSED(m);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	CallingConv::ID Function::GetCallingConv() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Function::SetCallingConv(CallingConv::ID cc)
	{
		DILITHIUM_UNUSED(cc);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Function::arg_iterator Function::ArgBegin()
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Function::const_arg_iterator Function::ArgBegin() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Function::arg_iterator Function::ArgEnd()
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Function::const_arg_iterator Function::ArgEnd() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Function::HasPrefixData() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* Function::PrefixData() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Function::PrefixData(Constant* prefix_data)
	{
		DILITHIUM_UNUSED(prefix_data);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	bool Function::HasPrologueData() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	Constant* Function::PrologueData() const
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Function::PrologueData(Constant* prologue_data)
	{
		DILITHIUM_UNUSED(prologue_data);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
