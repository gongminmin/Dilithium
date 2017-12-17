/**
 * @file DxilTypeSystem.hpp
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

#ifndef DILITHIUM_DXIL_TYPE_SYSTEM_HPP
#define DILITHIUM_DXIL_TYPE_SYSTEM_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilCompType.hpp>
#include <Dilithium/dxc/HLSL/DxilInterpolationMode.hpp>

namespace Dilithium
{
	class Function;
	class LLVMModule;
	class StructType;

	enum class MatrixOrientation
	{
		Undefined = 0,
		RowMajor,
		ColumnMajor,
		LastEntry
	};

	struct DxilMatrixAnnotation
	{
		uint32_t rows;
		uint32_t cols;
		MatrixOrientation orientation;

		DxilMatrixAnnotation()
			: rows(0), cols(0), orientation(MatrixOrientation::Undefined)
		{
		}
	};


	class DxilFieldAnnotation
	{
	public:
		DxilFieldAnnotation()
			: precise_(false)
			, cbuffer_offset_(UINT_MAX)
		{
		}

		bool IsPrecise() const
		{
			return precise_;
		}
		void SetPrecise(bool b = true)
		{
			precise_ = b;
		}

		bool HasMatrixAnnotation() const
		{
			return matrix_.cols != 0;
		}
		DxilMatrixAnnotation const & GetMatrixAnnotation() const
		{
			return matrix_;
		}
		void SetMatrixAnnotation(DxilMatrixAnnotation const & ma)
		{
			matrix_ = ma;
		}

		bool HasCBufferOffset() const
		{
			return cbuffer_offset_ != UINT_MAX;
		}
		uint32_t GetCBufferOffset() const
		{
			return cbuffer_offset_;
		}
		void SetCBufferOffset(uint32_t offset)
		{
			cbuffer_offset_ = offset;
		}

		bool HasCompType() const
		{
			return comp_type_.GetKind() != ComponentType::Invalid;
		}
		DxilCompType const & GetCompType() const
		{
			return comp_type_;
		}
		void SetCompType(ComponentType kind)
		{
			comp_type_ = DxilCompType(kind);
		}

		bool HasSemanticString() const
		{
			return !semantic_.empty();
		}
		std::string const & GetSemanticString() const
		{
			return semantic_;
		}
		void SetSemanticString(std::string const & sem_string)
		{
			semantic_ = sem_string;
		}

		bool HasInterpolationMode() const
		{
			return !interp_mode_.IsUndefined();
		}
		DxilInterpolationMode const & GetInterpolationMode() const
		{
			return interp_mode_;
		}
		void SetInterpolationMode(DxilInterpolationMode const & im)
		{
			interp_mode_ = im;
		}

		bool HasFieldName() const
		{
			return !field_name_.empty();
		}
		std::string const & GetFieldName() const
		{
			return field_name_;
		}
		void SetFieldName(std::string const & field_name)
		{
			field_name_ = field_name;
		}

	private:
		bool precise_;
		DxilCompType comp_type_;
		DxilMatrixAnnotation matrix_;
		uint32_t cbuffer_offset_;
		std::string semantic_;
		DxilInterpolationMode interp_mode_;
		std::string field_name_;
	};

	class DxilStructAnnotation
	{
	public:
		uint32_t NumFields() const;
		DxilFieldAnnotation& FieldAnnotation(uint32_t index);
		DxilFieldAnnotation const & FieldAnnotation(uint32_t index) const;
		StructType const * GetStructType() const;
		uint32_t CBufferSize() const;
		void CBufferSize(uint32_t size);
		void MarkEmptyStruct();
		bool IsEmptyStruct();

	private:
		StructType const * struct_type_;
		std::vector<DxilFieldAnnotation> field_annotations_;
		uint32_t cbuffer_size_;  // The size of struct if inside constant buffer.
	};

	enum class DxilParamInputQual
	{
		In,
		Out,
		Inout,
		InputPatch,
		OutputPatch,
		OutStream0,
		OutStream1,
		OutStream2,
		OutStream3,
		InputPrimitive,
	};

	class DxilParameterAnnotation : public DxilFieldAnnotation
	{
	public:
		DxilParameterAnnotation()
			: input_qual_(DxilParamInputQual::In)
		{
		}

		DxilParamInputQual GetParamInputQual() const
		{
			return input_qual_;
		}
		void SetParamInputQual(DxilParamInputQual qual)
		{
			input_qual_ = qual;
		}
		std::vector<uint32_t> const & GetSemanticIndexVec() const
		{
			return semantic_index_;
		}
		void SetSemanticIndexVec(std::vector<uint32_t> const & vec)
		{
			semantic_index_ = vec;
		}
		void AppendSemanticIndex(uint32_t sem_idx)
		{
			semantic_index_.push_back(sem_idx);
		}

	private:
		DxilParamInputQual input_qual_;
		std::vector<uint32_t> semantic_index_;
	};

	class DxilFunctionAnnotation
	{
		friend class DxilTypeSystem;

	public:
		uint32_t NumParameters() const
		{
			return static_cast<uint32_t>(parameter_annotations_.size());
		}
		DxilParameterAnnotation& ParameterAnnotation(uint32_t index);
		DxilParameterAnnotation const & ParameterAnnotation(uint32_t index) const;
		Function const * GetFunction() const;
		DxilParameterAnnotation& RetTypeAnnotation()
		{
			return ret_type_annotation_;
		}
		DxilParameterAnnotation const & RetTypeAnnotation() const
		{
			return ret_type_annotation_;
		}

	private:
		Function const * function_;
		std::vector<DxilParameterAnnotation> parameter_annotations_;
		DxilParameterAnnotation ret_type_annotation_;
	};

	class DxilTypeSystem
	{
	public:
		using StructAnnotationMap = std::map<StructType const *, std::unique_ptr<DxilStructAnnotation>>;
		using FunctionAnnotationMap = std::map<Function const*, std::unique_ptr<DxilFunctionAnnotation>>;

	public:
		explicit DxilTypeSystem(LLVMModule* mod);

		DxilStructAnnotation* AddStructAnnotation(StructType const * struct_type);
		DxilStructAnnotation* GetStructAnnotation(StructType const * struct_type);
		void EraseStructAnnotation(StructType const * struct_type);
		StructAnnotationMap& GetStructAnnotationMap()
		{
			return struct_annotations_;
		}

		DxilFunctionAnnotation* AddFunctionAnnotation(Function const * function);
		DxilFunctionAnnotation* GetFunctionAnnotation(Function const * function);
		void EraseFunctionAnnotation(Function* const function);
		FunctionAnnotationMap& GetFunctionAnnotationMap()
		{
			return function_annotations_;
		}

	private:
		LLVMModule* module_;
		StructAnnotationMap struct_annotations_;
		FunctionAnnotationMap function_annotations_;
	};
}

#endif		// DILITHIUM_DXIL_TYPE_SYSTEM_HPP
