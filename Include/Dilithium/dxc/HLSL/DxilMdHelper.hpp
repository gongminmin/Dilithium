/**
 * @file DxilMdHelper.hpp
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

#ifndef _DILITHIUM_DXIL_MD_HELPER_HPP
#define _DILITHIUM_DXIL_MD_HELPER_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilMdHelper.hpp>

namespace Dilithium
{
	class LLVMModule;

	class DxilCBuffer;
	class DxilResource;
	class DxilSampler;
	class DxilShaderModel;
	class DxilSignature;
	class DxilSignatureElement;
	class DxilRootSignatureHandle;

	class DxilMDHelper
	{
	public:
		// Use this class to manipulate metadata of DXIL or high-level DX IR specific fields in the record.
		class ExtraPropertyHelper
		{
		public:
			ExtraPropertyHelper(LLVMModule* mod);

			virtual void LoadSRVProperties(MDOperand const & operand, DxilResource& srv) = 0;
			virtual void LoadUAVProperties(MDOperand const & operand, DxilResource& uav) = 0;
			virtual void LoadCBufferProperties(MDOperand const & operand, DxilCBuffer& cb) = 0;
			virtual void LoadSamplerProperties(MDOperand const & operand, DxilSampler& sampler) = 0;
			virtual void LoadSignatureElementProperties(MDOperand const & operand, DxilSignatureElement& se) = 0;

		protected:
			LLVMContext& context_;
			LLVMModule* module_;
		};

	public:
		DxilMDHelper(LLVMModule* mod, std::unique_ptr<ExtraPropertyHelper> eph);
		~DxilMDHelper();

		void ShaderModel(DxilShaderModel const * sm);
		DxilShaderModel const * ShaderModel() const;

		void LoadDxilVersion(uint32_t& major, uint32_t& minor);
		void LoadDxilShaderModel(DxilShaderModel const *& sm);

		NamedMDNode const * GetDxilEntryPoints();
		void GetDxilEntryPoint(MDNode const * mdn, Function*& func, std::string& name,
			MDOperand const *& signatures, MDOperand const *& resources,
			MDOperand const *& properties);

		void LoadDxilSignatures(MDOperand const & mdn, DxilSignature& input_sig, DxilSignature& output_sig, DxilSignature& pc_sig);
		void LoadSignatureMetadata(MDOperand const & mdn, DxilSignature& sig);
		void LoadSignatureElement(MDOperand const & mdn, DxilSignatureElement& se);
		void LoadRootSignature(MDOperand const & mdn, DxilRootSignatureHandle& root_sig);

		static int32_t ConstMDToInt32(MDOperand const & operand);
		static uint32_t ConstMDToUInt32(MDOperand const & operand);
		static int8_t ConstMDToInt8(MDOperand const & operand);
		static uint8_t ConstMDToUInt8(MDOperand const & operand);
		void ConstMDTupleToUInt32Vector(MDTuple* tuple_md, std::vector<uint32_t>& vec);

	private:
		LLVMContext& context_;
		LLVMModule* module_;
		DxilShaderModel const * sm_;
		std::unique_ptr<ExtraPropertyHelper> extra_property_helper_;
	};

	class DxilExtraPropertyHelper : public DxilMDHelper::ExtraPropertyHelper
	{
	public:
		explicit DxilExtraPropertyHelper(LLVMModule* mod);

		void LoadSRVProperties(MDOperand const & operand, DxilResource& srv) override;
		void LoadUAVProperties(MDOperand const & operand, DxilResource& uav) override;
		void LoadCBufferProperties(MDOperand const & operand, DxilCBuffer& cb) override;
		void LoadSamplerProperties(MDOperand const & operand, DxilSampler& sampler) override;
		void LoadSignatureElementProperties(MDOperand const & operand, DxilSignatureElement& se) override;
	};
}

#endif		// _DILITHIUM_DXIL_MD_HELPER_HPP
