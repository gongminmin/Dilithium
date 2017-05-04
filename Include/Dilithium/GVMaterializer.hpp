/**
 * @file GVMaterializer.hpp
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

#ifndef _DILITHIUM_GVMATERIALIZER_HPP
#define _DILITHIUM_GVMATERIALIZER_HPP

#pragma once

namespace Dilithium
{
	class GlobalValue;
	class LLVMModule;

	// A general interface of materializer, inspired by LLVM's.
	// TODO: Consider merging it with BitcodeReader
	class GVMaterializer
	{
	protected:
		GVMaterializer()
		{
		}

	public:
		virtual ~GVMaterializer()
		{
		}

		virtual void Materialize(GlobalValue* gv) = 0;
		virtual void MaterializeModule(LLVMModule* m) = 0;
		virtual void MaterializeMetadata() = 0;
	};
}

#endif		// _DILITHIUM_LLVM_MODULE_HPP
