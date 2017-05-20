/**
 * @file DxilModule.cpp
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
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/dxc/HLSL/DxilModule.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>

namespace Dilithium
{
	DxilModule::DxilModule(LLVMModule* mod)
		: context_(mod->Context()), module_(mod),
			md_helper_(std::make_unique<DxilMDHelper>(mod, std::make_unique<DxilExtraPropertyHelper>(mod))),
			sm_(nullptr)
	{
		BOOST_ASSERT(mod != nullptr);
	}

	DxilModule::~DxilModule()
	{
	}

	void DxilModule::LoadDxilMetadata()
	{
		BOOST_ASSERT_MSG(sm_ == nullptr, "Shader model must not change for the module");

		md_helper_->LoadDxilVersion(dxil_major_, dxil_minor_);

		DxilShaderModel const * sm;
		md_helper_->LoadDxilShaderModel(sm);
		sm_ = sm;
		md_helper_->ShaderModel(sm_);

		auto shader_kind = sm_->GetKind();
		input_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::Input);
		output_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::Output);
		patch_constant_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::PatchConstant);
		root_signature_ = std::make_unique<DxilRootSignatureHandle>();

		auto entries = md_helper_->GetDxilEntryPoints();
		TIFBOOL(entries->NumOperands() == 1);

		MDOperand const * signatures;
		MDOperand const * resources;
		MDOperand const * properties;
		md_helper_->GetDxilEntryPoint(entries->Operand(0), entry_func_, entry_name_, signatures, resources, properties);

		md_helper_->LoadDxilSignatures(*signatures, *input_signature_,
			*output_signature_, *patch_constant_signature_);

		DILITHIUM_NOT_IMPLEMENTED;
	}
}
