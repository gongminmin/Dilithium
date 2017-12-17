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
#include <Dilithium/GlobalVariable.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/dxc/HLSL/DxilCBuffer.hpp>
#include <Dilithium/dxc/HLSL/DxilCompType.hpp>
#include <Dilithium/dxc/HLSL/DxilMdHelper.hpp>
#include <Dilithium/dxc/HLSL/DxilResource.hpp>
#include <Dilithium/dxc/HLSL/DxilResourceBase.hpp>
#include <Dilithium/dxc/HLSL/DxilRootSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilSampler.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>
#include <Dilithium/dxc/HLSL/DxilSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilTypeSystem.hpp>

namespace Dilithium
{
	char const DxilMDHelper::DxilTypeSystemMDName[] = "dx.typeAnnotations";

	MDTuple const * CastToTupleOrNull(MDOperand const & mdn)
	{
		if (mdn.Get() == nullptr)
		{
			return nullptr;
		}

		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		return tuple_md;
	}

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
		std::string shader_model_name = std::string(shader_type_md->String());
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
		name = std::string(md_name->String());

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

			this->LoadSignatureMetadata(tuple_md->Operand(DS_Input), input_sig);
			this->LoadSignatureMetadata(tuple_md->Operand(DS_Output), output_sig);
			this->LoadSignatureMetadata(tuple_md->Operand(DS_PatchConstant), pc_sig);
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
		enum DxilSignatureElement
		{
			DSE_ID = 0,
			DSE_Name,
			DSE_Type,
			DSE_SystemValue,
			DSE_IndexVector,
			DSE_InterpMode,
			DSE_Rows,
			DSE_Cols,
			DSE_StartRow,
			DSE_StartCol,
			DSE_NameValueList,

			DSE_NumFields
		};

		TIFBOOL(mdn.Get() != nullptr);
		auto tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DSE_NumFields);

		uint32_t id = ConstMDToUInt32(tuple_md->Operand(DSE_ID));
		auto name = dyn_cast<MDString>(tuple_md->Operand(DSE_Name));
		DxilCompType ct = DxilCompType(ConstMDToUInt8(tuple_md->Operand(DSE_Type)));
		SemanticKind sem_kind = static_cast<SemanticKind>(ConstMDToUInt8(tuple_md->Operand(DSE_SystemValue)));
		auto semantic_index_vector_md = dyn_cast<MDTuple>(tuple_md->Operand(DSE_IndexVector));
		auto im = DxilInterpolationMode(static_cast<DxilInterpolationMode>(ConstMDToUInt8(tuple_md->Operand(DSE_InterpMode))));
		uint32_t num_rows = ConstMDToUInt32(tuple_md->Operand(DSE_Rows));
		uint8_t num_cols = ConstMDToUInt8(tuple_md->Operand(DSE_Cols));
		int32_t start_row = ConstMDToInt32(tuple_md->Operand(DSE_StartRow));
		int8_t start_col = ConstMDToInt8(tuple_md->Operand(DSE_StartCol));

		TIFBOOL(name != nullptr);
		TIFBOOL(semantic_index_vector_md != nullptr);

		std::vector<unsigned> semantic_index_vector;
		ConstMDTupleToUInt32Vector(semantic_index_vector_md, semantic_index_vector);

		se.Initialize(name->String(), ct, im, num_rows, num_cols, start_row, start_col, id, semantic_index_vector);
		se.SetKind(sem_kind);

		extra_property_helper_->LoadSignatureElementProperties(tuple_md->Operand(DSE_NameValueList), se);
	}

	void DxilMDHelper::LoadRootSignature(MDOperand const & mdn, DxilRootSignatureHandle& root_sig)
	{
		if (mdn.Get() != nullptr)
		{
			auto metadata = dyn_cast<ConstantAsMetadata>(mdn.Get());
			TIFBOOL(metadata != nullptr);
			auto data = dyn_cast<ConstantDataArray>(metadata->GetValue());
			TIFBOOL(data != nullptr);
			TIFBOOL(data->GetElementType() == Type::Int8Type(context_));

			root_sig.Clear();
			root_sig.LoadSerialized(reinterpret_cast<uint8_t const *>(data->GetRawDataValues().data()),
				static_cast<uint32_t>(data->GetRawDataValues().size()));
		}
	}

	void DxilMDHelper::GetDxilResources(MDOperand const & mdn, MDTuple const *& srvs, MDTuple const *& uavs,
		MDTuple const *& cbuffers, MDTuple const *& samplers)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DxilNumResourceFields);

		srvs = CastToTupleOrNull(tuple_md->Operand(DxilResourceSRVs));
		uavs = CastToTupleOrNull(tuple_md->Operand(DxilResourceUAVs));
		cbuffers = CastToTupleOrNull(tuple_md->Operand(DxilResourceCBuffers));
		samplers = CastToTupleOrNull(tuple_md->Operand(DxilResourceSamplers));
	}

	void DxilMDHelper::LoadDxilResourceBase(MDOperand const & mdn, DxilResourceBase& res)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() >= DxilResourceBaseNumFields);

		res.SetID(ConstMDToUInt32(tuple_md->Operand(DxilResourceBaseID)));
		res.SetGlobalSymbol(dyn_cast<Constant>(ValueMDToValue(tuple_md->Operand(DxilResourceBaseVariable))));
		res.SetGlobalName(StringMDToString(tuple_md->Operand(DxilResourceBaseName)));
		res.SetSpaceID(ConstMDToUInt32(tuple_md->Operand(DxilResourceBaseSpaceID)));
		res.SetLowerBound(ConstMDToUInt32(tuple_md->Operand(DxilResourceBaseLowerBound)));
		res.SetRangeSize(ConstMDToUInt32(tuple_md->Operand(DxilResourceBaseRangeSize)));
	}

	void DxilMDHelper::LoadDxilSRV(MDOperand const & mdn, DxilResource& srv)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DxilSRVNumFields);

		srv.SetReadWrite(false);

		this->LoadDxilResourceBase(mdn, srv);

		// SRV-specific fields.
		srv.SetKind(static_cast<ResourceKind>(ConstMDToUInt32(tuple_md->Operand(DxilSRVShape))));
		srv.SetSampleCount(ConstMDToUInt32(tuple_md->Operand(DxilSRVSampleCount)));

		// Name-value list of extended properties.
		extra_property_helper_->LoadSRVProperties(tuple_md->Operand(DxilSRVNameValueList), srv);
	}

	void DxilMDHelper::LoadDxilUAV(MDOperand const & mdn, DxilResource& uav)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DxilUAVNumFields);

		uav.SetReadWrite(true);

		this->LoadDxilResourceBase(mdn, uav);

		// UAV-specific fields.
		uav.SetKind(static_cast<ResourceKind>(ConstMDToUInt32(tuple_md->Operand(DxilUAVShape))));
		uav.SetGloballyCoherent(ConstMDToBool(tuple_md->Operand(DxilUAVGloballyCoherent)));
		uav.SetHasCounter(ConstMDToBool(tuple_md->Operand(DxilUAVCounter)));
		uav.SetRasterizerOrderedView(ConstMDToBool(tuple_md->Operand(DxilUAVRasterizerOrderedView)));

		// Name-value list of extended properties.
		extra_property_helper_->LoadUAVProperties(tuple_md->Operand(DxilUAVNameValueList), uav);
	}

	void DxilMDHelper::LoadDxilCBuffer(MDOperand const & mdn, DxilCBuffer& cbuffer)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DxilCBufferNumFields);

		this->LoadDxilResourceBase(mdn, cbuffer);

		// CBuffer-specific fields.
		cbuffer.SetSize(ConstMDToUInt32(tuple_md->Operand(DxilCBufferSizeInBytes)));

		// Name-value list of extended properties.
		extra_property_helper_->LoadCBufferProperties(tuple_md->Operand(DxilCBufferNameValueList), cbuffer);
	}

	void DxilMDHelper::LoadDxilSampler(MDOperand const & mdn, DxilSampler& sampler)
	{
		TIFBOOL(mdn.Get() != nullptr);
		MDTuple const * tuple_md = dyn_cast<MDTuple>(mdn.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == DxilSamplerNumFields);

		this->LoadDxilResourceBase(mdn, sampler);

		// Sampler-specific fields.
		sampler.SetSamplerKind(static_cast<SamplerKind>(ConstMDToUInt32(tuple_md->Operand(DxilSamplerType))));

		// Name-value list of extended properties.
		extra_property_helper_->LoadSamplerProperties(tuple_md->Operand(DxilSamplerNameValueList), sampler);
	}

	void DxilMDHelper::LoadDxilTypeSystem(DxilTypeSystem& type_system)
	{
		auto dxil_type_annotations_md = module_->GetNamedMetadata(DxilTypeSystemMDName);
		if (dxil_type_annotations_md != nullptr)
		{
			TIFBOOL(dxil_type_annotations_md->NumOperands() <= 2);
			for (uint32_t i = 0; i < dxil_type_annotations_md->NumOperands(); ++ i)
			{
				auto const * tuple_md = dyn_cast<MDTuple>(dxil_type_annotations_md->Operand(i));
				TIFBOOL(tuple_md != nullptr);
				this->LoadDxilTypeSystemNode(*tuple_md, type_system);
			}
		}
	}

	void DxilMDHelper::LoadDxilTypeSystemNode(MDTuple const & mdt, DxilTypeSystem& type_system)
	{
		uint32_t tag = ConstMDToUInt32(mdt.Operand(0));
		if (tag == DxilTypeSystemStructTag)
		{
			TIFBOOL((mdt.NumOperands() & 0x1) == 1);

			for (uint32_t i = 1; i < mdt.NumOperands(); i += 2)
			{
				auto gv = dyn_cast<GlobalVariable>(ValueMDToValue(mdt.Operand(i)));
				TIFBOOL(gv != nullptr);
				auto gv_type = dyn_cast<StructType>(gv->GetType()->PointerElementType());
				TIFBOOL(gv_type != nullptr);

				auto sa = type_system.AddStructAnnotation(gv_type);
				this->LoadDxilStructAnnotation(mdt.Operand(i + 1), *sa);
			}
		}
		else
		{
			TIFBOOL(tag == DxilTypeSystemFunctionTag);
			TIFBOOL((mdt.NumOperands() & 0x1) == 1);

			for (uint32_t i = 1; i < mdt.NumOperands(); i += 2)
			{
				auto func = dyn_cast<Function>(ValueMDToValue(mdt.Operand(i)));
				auto fa = type_system.AddFunctionAnnotation(func);
				this->LoadDxilFunctionAnnotation(mdt.Operand(i + 1), *fa);
			}
		}
	}

	void DxilMDHelper::LoadDxilStructAnnotation(MDOperand const & mdo, DxilStructAnnotation& sa)
	{
		TIFBOOL(mdo.Get() != nullptr);
		auto const * tuple_md = dyn_cast<MDTuple>(mdo.Get());
		TIFBOOL(tuple_md != nullptr);
		if (tuple_md->NumOperands() == 1)
		{
			auto const * st = sa.GetStructType();
			if (st->NumElements() == 1)
			{
				auto elem_type = st->ElementType(0);
				if (elem_type == Type::Int8Type(st->Context()))
				{
					sa.MarkEmptyStruct();
				}
			}
		}
		TIFBOOL(tuple_md->NumOperands() == sa.NumFields() + 1);

		sa.CBufferSize(ConstMDToUInt32(tuple_md->Operand(0)));
		for (uint32_t i = 0; i < sa.NumFields(); ++ i)
		{
			auto const & tmdo = tuple_md->Operand(i + 1);
			auto& fa = sa.FieldAnnotation(i);
			this->LoadDxilFieldAnnotation(tmdo, fa);
		}
	}

	void DxilMDHelper::LoadDxilFieldAnnotation(MDOperand const & mdo, DxilFieldAnnotation& fa)
	{
		TIFBOOL(mdo.Get() != nullptr);
		auto const * tuple_md = dyn_cast<MDTuple>(mdo.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL((tuple_md->NumOperands() & 0x1) == 0);

		for (unsigned i = 0; i < tuple_md->NumOperands(); i += 2)
		{
			uint32_t tag = ConstMDToUInt32(tuple_md->Operand(i));
			auto const & tmdo = tuple_md->Operand(i + 1);
			TIFBOOL(tmdo.Get() != nullptr);

			switch (tag)
			{
			case DxilFieldAnnotationPreciseTag:
				fa.SetPrecise(ConstMDToBool(tmdo));
				break;

			case DxilFieldAnnotationMatrixTag:
				{
					DxilMatrixAnnotation ma;
					auto const * ma_tuple_md = dyn_cast<MDTuple>(tmdo.Get());
					TIFBOOL(ma_tuple_md != nullptr);
					TIFBOOL(ma_tuple_md->NumOperands() == 3);
					ma.rows = ConstMDToUInt32(ma_tuple_md->Operand(0));
					ma.cols = ConstMDToUInt32(ma_tuple_md->Operand(1));
					ma.orientation = static_cast<MatrixOrientation>(ConstMDToUInt32(ma_tuple_md->Operand(2)));
					fa.SetMatrixAnnotation(ma);
				}
				break;

			case DxilFieldAnnotationCBufferOffsetTag:
				fa.SetCBufferOffset(ConstMDToUInt32(tmdo));
				break;

			case DxilFieldAnnotationSemanticStringTag:
				fa.SetSemanticString(StringMDToString(tmdo));
				break;

			case DxilFieldAnnotationInterpolationModeTag:
				fa.SetInterpolationMode(DxilInterpolationMode(static_cast<InterpolationMode>(ConstMDToUInt32(tmdo))));
				break;

			case DxilFieldAnnotationFieldNameTag:
				fa.SetFieldName(StringMDToString(tmdo));
				break;

			case DxilFieldAnnotationCompTypeTag:
				fa.SetCompType(static_cast<ComponentType>(ConstMDToUInt32(tmdo)));
				break;

			default:
				// TODO:  I don't think we should be failing unrecognized extended tags.
				//        Perhaps we can flag this case in the module and fail validation
				//        if flagged.
				//        That way, an existing loader will not fail on an additional tag
				//        and the blob would not be signed if the extra tag was not legal.
				TIFBOOL(false);
			}
		}
	}

	void DxilMDHelper::LoadDxilFunctionAnnotation(MDOperand const & mdo, DxilFunctionAnnotation& fa)
	{
		TIFBOOL(mdo.Get() != nullptr);
		auto const * tuple_md = dyn_cast<MDTuple>(mdo.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == fa.NumParameters() + 1);

		auto& ret_type_annotation = fa.RetTypeAnnotation();
		this->LoadDxilParamAnnotation(tuple_md->Operand(0), ret_type_annotation);
		for (uint32_t i = 0; i < fa.NumParameters(); ++ i)
		{
			auto const & tmdo = tuple_md->Operand(i + 1);
			auto& pa = fa.ParameterAnnotation(i);
			this->LoadDxilParamAnnotation(tmdo, pa);
		}
	}

	void DxilMDHelper::LoadDxilParamAnnotation(MDOperand const & mdo, DxilParameterAnnotation& pa)
	{
		TIFBOOL(mdo.Get() != nullptr);
		auto const * tuple_md = dyn_cast<MDTuple>(mdo.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL(tuple_md->NumOperands() == 3);

		pa.SetParamInputQual(static_cast<DxilParamInputQual>(ConstMDToUInt32(tuple_md->Operand(0))));
		this->LoadDxilFieldAnnotation(tuple_md->Operand(1), pa);
		MDTuple *pSemanticIndexVectorMD = dyn_cast<MDTuple>(tuple_md->Operand(2));
		std::vector<unsigned> SemanticIndexVector;
		ConstMDTupleToUInt32Vector(pSemanticIndexVectorMD, SemanticIndexVector);
		pa.SetSemanticIndexVec(SemanticIndexVector);
	}

	void DxilMDHelper::LoadDxilGSState(MDOperand const & mdn, InputPrimitive& primitive, uint32_t& max_vertex_count,
		uint32_t& active_stream_mask, PrimitiveTopology& stream_primitive_topology,
		uint32_t& gs_instance_count)
	{
		DILITHIUM_UNUSED(mdn);
		DILITHIUM_UNUSED(primitive);
		DILITHIUM_UNUSED(max_vertex_count);
		DILITHIUM_UNUSED(active_stream_mask);
		DILITHIUM_UNUSED(stream_primitive_topology);
		DILITHIUM_UNUSED(gs_instance_count);

		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilMDHelper::LoadDxilDSState(MDOperand const & mdn, TessellatorDomain& domain, uint32_t& input_control_point_count)
	{
		DILITHIUM_UNUSED(mdn);
		DILITHIUM_UNUSED(domain);
		DILITHIUM_UNUSED(input_control_point_count);

		DILITHIUM_NOT_IMPLEMENTED;
	}

	void DxilMDHelper::LoadDxilHSState(MDOperand const & mdn, Function*& patch_constant_function, uint32_t& input_control_point_count,
		uint32_t& output_control_point_count, TessellatorDomain& tess_domain, TessellatorPartitioning& tess_partitioning,
		TessellatorOutputPrimitive& tess_output_primitive, float& max_tess_factor)
	{
		DILITHIUM_UNUSED(mdn);
		DILITHIUM_UNUSED(patch_constant_function);
		DILITHIUM_UNUSED(input_control_point_count);
		DILITHIUM_UNUSED(output_control_point_count);
		DILITHIUM_UNUSED(tess_domain);
		DILITHIUM_UNUSED(tess_partitioning);
		DILITHIUM_UNUSED(tess_output_primitive);
		DILITHIUM_UNUSED(max_tess_factor);

		DILITHIUM_NOT_IMPLEMENTED;
	}

	int32_t DxilMDHelper::ConstMDToInt32(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return static_cast<int32_t>(ci->ZExtValue());
	}

	uint32_t DxilMDHelper::ConstMDToUInt32(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return static_cast<uint32_t>(ci->ZExtValue());
	}

	uint64_t DxilMDHelper::ConstMDToUInt64(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return ci->ZExtValue();
	}

	int8_t DxilMDHelper::ConstMDToInt8(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return static_cast<int8_t>(ci->ZExtValue());
	}

	uint8_t DxilMDHelper::ConstMDToUInt8(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return static_cast<uint8_t>(ci->ZExtValue());
	}

	bool DxilMDHelper::ConstMDToBool(MDOperand const & operand)
	{
		ConstantInt* ci = cast<ConstantInt>(cast<ConstantAsMetadata>(operand)->GetValue());
		TIFBOOL(ci != nullptr);
		return ci->ZExtValue() != 0;
	}

	std::string DxilMDHelper::StringMDToString(MDOperand const & operand)
	{
		MDString* md_string = dyn_cast<MDString>(operand.Get());
		TIFBOOL(md_string != nullptr);
		return std::string(md_string->String());
	}

	Value* DxilMDHelper::ValueMDToValue(MDOperand const & operand)
	{
		TIFBOOL(operand.Get() != nullptr);
		ValueAsMetadata* val_as_md = dyn_cast<ValueAsMetadata>(operand.Get());
		TIFBOOL(val_as_md != nullptr);
		Value* value = val_as_md->GetValue();
		TIFBOOL(value != nullptr);
		return value;
	}

	void DxilMDHelper::ConstMDTupleToUInt32Vector(MDTuple* tuple_md, std::vector<uint32_t>& vec)
	{
		TIFBOOL(tuple_md != nullptr);

		vec.resize(tuple_md->NumOperands());
		for (uint32_t i = 0; i < tuple_md->NumOperands(); ++ i)
		{
			vec[i] = ConstMDToUInt32(tuple_md->Operand(i));
		}
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
		if (operand.Get() == nullptr)
		{
			return;
		}

		MDTuple const * tuple_md = dyn_cast<MDTuple>(operand.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL((tuple_md->NumOperands() & 0x1) == 0);

		for (uint32_t i = 0; i < tuple_md->NumOperands(); i += 2)
		{
			uint32_t tag = DxilMDHelper::ConstMDToUInt32(tuple_md->Operand(i));
			MDOperand const & mdn = tuple_md->Operand(i + 1);
			switch (tag)
			{
			case DxilMDHelper::DxilSignatureElementOutputStreamTag:
				se.SetOutputStream(DxilMDHelper::ConstMDToUInt32(mdn));
				break;
			case DxilMDHelper::DxilSignatureElementGlobalSymbolTag:
				break;

			default:
				DILITHIUM_UNREACHABLE("Unknown signature element tag");
			}
		}
	}
}
