/**
 * @file DxilMdHelper.cpp
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
#include <Dilithium/Constants.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/dxc/HLSL/DxilMdHelper.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>
#include <Dilithium/dxc/HLSL/DxilSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilRootSignature.hpp>

namespace Dilithium
{
	DxilMDHelper::ExtraPropertyHelper::ExtraPropertyHelper(LLVMModule* mod)
		: context_(mod->Context()), module_(mod)
	{
	}


	DxilMDHelper::DxilMDHelper(LLVMModule* mod, std::unique_ptr<ExtraPropertyHelper> eph)
		: context_(mod->Context()), module_(mod), sm_(nullptr), extra_property_helper_(std::move(eph))
	{
	}

	DxilMDHelper::~DxilMDHelper()
	{
	}

	void DxilMDHelper::ShaderModel(DxilShaderModel const * sm)
	{
		sm_ = sm;
	}

	DxilShaderModel const * DxilMDHelper::ShaderModel() const
	{
		return sm_;
	}

	void DxilMDHelper::LoadDxilVersion(uint32_t& major, uint32_t& minor)
	{
		uint32_t constexpr DXIL_VERSION_NUM_FIELDS = 2;
		uint32_t constexpr DXIL_VERSION_MAJOR_IDX = 0;
		uint32_t constexpr DXIL_VERSION_MINOR_IDX = 1;

		NamedMDNode* dxil_version_md = module_->GetNamedMetadata("dx.version");
		TIFBOOL(dxil_version_md != nullptr);
		TIFBOOL(dxil_version_md->NumOperands() == 1);

		MDNode* version_md = dxil_version_md->Operand(0);
		TIFBOOL(version_md->NumOperands() == DXIL_VERSION_NUM_FIELDS);

		major = ConstMDToUInt32(version_md->Operand(DXIL_VERSION_MAJOR_IDX));
		minor = ConstMDToUInt32(version_md->Operand(DXIL_VERSION_MINOR_IDX));
	}

	void DxilMDHelper::LoadDxilShaderModel(DxilShaderModel const *& sm)
	{
		uint32_t constexpr DXIL_SHADER_MODEL_NUM_FIELDS = 3;
		uint32_t constexpr DXIL_SHADER_MODEL_TYPE_IDX = 0;
		uint32_t constexpr DXIL_SHADER_MODEL_MAJOR_IDX = 1;
		uint32_t constexpr DXIL_SHADER_MODEL_MINOR_IDX = 2;

		NamedMDNode* shader_model_named_md = module_->GetNamedMetadata("dx.shaderModel");
		TIFBOOL(shader_model_named_md != nullptr);
		TIFBOOL(shader_model_named_md->NumOperands() == 1);

		MDNode* shader_model_md = shader_model_named_md->Operand(0);
		TIFBOOL(shader_model_md->NumOperands() == DXIL_SHADER_MODEL_NUM_FIELDS);

		MDString* shader_type_md = dyn_cast<MDString>(shader_model_md->Operand(DXIL_SHADER_MODEL_TYPE_IDX));
		TIFBOOL(shader_type_md != nullptr);
		uint32_t major = ConstMDToUInt32(shader_model_md->Operand(DXIL_SHADER_MODEL_MAJOR_IDX));
		uint32_t minor = ConstMDToUInt32(shader_model_md->Operand(DXIL_SHADER_MODEL_MINOR_IDX));
		std::string shader_model_name = shader_type_md->String().to_string();
		shader_model_name += "_" + std::to_string(major) + "_" + std::to_string(minor);
		sm = DxilShaderModel::GetByName(shader_model_name);
		if (!sm->IsValid())
		{
			TERROR(("Unknown shader model '" + shader_model_name + "'").c_str());
		}
	}

	NamedMDNode const * DxilMDHelper::GetDxilEntryPoints()
	{
		auto entry_points_named_md = module_->GetNamedMetadata("dx.entryPoints");
		TIFBOOL(entry_points_named_md != nullptr);
		return entry_points_named_md;
	}

	void DxilMDHelper::GetDxilEntryPoint(MDNode const * mdn, Function*& func, std::string& name,
		MDOperand const *& signatures, MDOperand const *& resources,
		MDOperand const *& properties)
	{
		enum DxilEntryPoint
		{
			DEP_Function = 0,
			DEP_Name,
			DEP_Signatures,
			DEP_Resources,
			DEP_Properties,

			DEP_NumFields,
		};

		TIFBOOL(mdn != nullptr);
		auto tuple_md = dyn_cast<MDTuple>(mdn);
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DEP_NumFields);

		auto const & mdn_func = tuple_md->Operand(DEP_Function);
		if (mdn_func.Get() != nullptr)
		{
			auto value_func = dyn_cast<ValueAsMetadata>(mdn_func.Get());
			TIFBOOL(value_func != nullptr);
			func = dyn_cast<Function>(value_func->GetValue());
			TIFBOOL(func != nullptr);
		}
		else
		{
			func = nullptr;
		}

		auto const & mdn_name = tuple_md->Operand(DEP_Name);
		TIFBOOL(mdn_name.Get() != nullptr);
		auto md_name = dyn_cast<MDString>(mdn_name);
		TIFBOOL(md_name != nullptr);
		name = md_name->String().to_string();

		signatures = &tuple_md->Operand(DEP_Signatures);
		resources = &tuple_md->Operand(DEP_Resources);
		properties = &tuple_md->Operand(DEP_Properties);
	}

	void DxilMDHelper::LoadDxilSignatures(MDOperand const & mdn, DxilSignature& input_sig, DxilSignature& output_sig,
		DxilSignature& pc_sig)
	{
		enum DxilSignature
		{
			DS_Input = 0,
			DS_Output,
			DS_PatchConstant,

			DS_NumFields
		};

		if (mdn.Get() != nullptr)
		{
			auto tuple_md = dyn_cast<MDTuple>(mdn.Get());
			TIFBOOL(tuple_md != nullptr);
			TIFBOOL(tuple_md->NumOperands() == DS_NumFields);

			LoadSignatureMetadata(tuple_md->Operand(DS_Input), input_sig);
			LoadSignatureMetadata(tuple_md->Operand(DS_Output), output_sig);
			LoadSignatureMetadata(tuple_md->Operand(DS_PatchConstant), pc_sig);
		}
	}

	void DxilMDHelper::LoadSignatureMetadata(MDOperand const & mdn, DxilSignature& sig)
	{
		if (mdn.Get() != nullptr)
		{
			auto tuple_md = dyn_cast<MDTuple>(mdn.Get());
			TIFBOOL(tuple_md != nullptr);

			for (uint32_t i = 0; i < tuple_md->NumOperands(); ++ i)
			{
				auto se = sig.CreateElement();
				this->LoadSignatureElement(tuple_md->Operand(i), *se);
				sig.AppendElement(std::move(se));
			}
		}
	}

	void DxilMDHelper::LoadSignatureElement(MDOperand const & mdn, DxilSignatureElement& se)
	{
		DILITHIUM_UNUSED(mdn);
		DILITHIUM_UNUSED(se);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilMDHelper::LoadRootSignature(MDOperand const & mdn, DxilRootSignatureHandle& root_sig)
	{
		DILITHIUM_UNUSED(mdn);
		DILITHIUM_UNUSED(root_sig);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	uint32_t DxilMDHelper::ConstMDToUInt32(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return static_cast<uint32_t>(ci->ZExtValue());
	}


	DxilExtraPropertyHelper::DxilExtraPropertyHelper(LLVMModule* mod)
		: ExtraPropertyHelper(mod)
	{
	}

	void DxilExtraPropertyHelper::LoadSRVProperties(MDOperand const & operand, DxilResource& srv)
	{
		DILITHIUM_UNUSED(operand);
		DILITHIUM_UNUSED(srv);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilExtraPropertyHelper::LoadUAVProperties(MDOperand const & operand, DxilResource& uav)
	{
		DILITHIUM_UNUSED(operand);
		DILITHIUM_UNUSED(uav);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilExtraPropertyHelper::LoadCBufferProperties(MDOperand const & operand, DxilCBuffer& cb)
	{
		DILITHIUM_UNUSED(operand);
		DILITHIUM_UNUSED(cb);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilExtraPropertyHelper::LoadSamplerProperties(MDOperand const & operand, DxilSampler& sampler)
	{
		DILITHIUM_UNUSED(operand);
		DILITHIUM_UNUSED(sampler);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilExtraPropertyHelper::LoadSignatureElementProperties(MDOperand const & operand, DxilSignatureElement& se)
	{
		DILITHIUM_UNUSED(operand);
		DILITHIUM_UNUSED(se);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
