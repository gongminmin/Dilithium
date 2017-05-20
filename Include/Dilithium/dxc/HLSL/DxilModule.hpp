/**
 * @file DxilModule.hpp
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

#ifndef _DILITHIUM_DXIL_MODULE_HPP
#define _DILITHIUM_DXIL_MODULE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilMdHelper.hpp>
#include <Dilithium/dxc/HLSL/DxilSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilRootSignature.hpp>

namespace Dilithium
{
	class LLVMModule;
	class Function;

	class DxilModule
	{
	public:
		explicit DxilModule(LLVMModule* mod);
		~DxilModule();

		void LoadDxilMetadata();

	private:
		LLVMContext& context_;
		LLVMModule* module_;
		Function* entry_func_;
		Function* patch_constant_func_;
		std::string entry_name_;
		std::unique_ptr<DxilMDHelper> md_helper_;
		DxilShaderModel const * sm_;
		uint32_t dxil_major_;
		uint32_t dxil_minor_;

		std::unique_ptr<DxilSignature> input_signature_;
		std::unique_ptr<DxilSignature> output_signature_;
		std::unique_ptr<DxilSignature> patch_constant_signature_;
		std::unique_ptr<DxilRootSignatureHandle> root_signature_;
	};
}

#endif		// _DILITHIUM_DXIL_MODULE_HPP
