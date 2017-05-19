/**
 * @file GlobalValue.hpp
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

#ifndef _DILITHIUM_GLOBAL_VALUE_HPP
#define _DILITHIUM_GLOBAL_VALUE_HPP

#pragma once

#include <Dilithium/Util.hpp>
#include <Dilithium/Constant.hpp>

namespace Dilithium
{
	class PointerType;

	class GlobalValue : public Constant
	{
	public:
		enum LinkageTypes
		{
			ExternalLinkage = 0,
			AvailableExternallyLinkage,
			LinkOnceAnyLinkage,
			LinkOnceODRLinkage,
			WeakAnyLinkage,
			WeakODRLinkage,
			AppendingLinkage,
			InternalLinkage,
			PrivateLinkage,
			ExternalWeakLinkage,
			CommonLinkage
		};

		enum VisibilityTypes
		{
			DefaultVisibility = 0,
			HiddenVisibility,
			ProtectedVisibility
		};

		enum DLLStorageClassTypes
		{
			DefaultStorageClass = 0,
			DLLImportStorageClass = 1,
			DLLExportStorageClass = 2
		};

		enum ThreadLocalMode
		{
			NotThreadLocal = 0,
			GeneralDynamicTLSModel,
			LocalDynamicTLSModel,
			InitialExecTLSModel,
			LocalExecTLSModel
		};

	public:
		bool HasUnnamedAddr() const
		{
			return unnamed_addr_;
		}
		void UnnamedAddr(bool Val)
		{
			unnamed_addr_ = Val;
		}

		VisibilityTypes Visibility() const
		{
			return static_cast<VisibilityTypes>(visibility_);
		}
		void Visibility(VisibilityTypes v)
		{
			BOOST_ASSERT_MSG(!this->HasLocalLinkage() || (v == DefaultVisibility), "local linkage requires default visibility");
			visibility_ = v;
		}
		bool HasDefaultVisibility() const
		{
			return visibility_ == DefaultVisibility;
		}
		bool HasHiddenVisibility() const
		{
			return visibility_ == HiddenVisibility;
		}
		bool HasProtectedVisibility() const
		{
			return visibility_ == ProtectedVisibility;
		}

		DLLStorageClassTypes DLLStorageClass() const
		{
			return DLLStorageClassTypes(dll_storage_class_);
		}
		void DLLStorageClass(DLLStorageClassTypes c)
		{
			dll_storage_class_ = c;
		}

		static bool IsInternalLinkage(LinkageTypes lt)
		{
			return lt == InternalLinkage;
		}
		static bool IsPrivateLinkage(LinkageTypes lt)
		{
			return lt == PrivateLinkage;
		}
		static bool IsLocalLinkage(LinkageTypes lt)
		{
			return IsInternalLinkage(lt) || IsPrivateLinkage(lt);
		}

		bool HasLocalLinkage() const
		{
			return IsLocalLinkage(linkage_);
		}

		LinkageTypes Linkage() const
		{
			return linkage_;
		}
		void Linkage(LinkageTypes lt)
		{
			if (IsLocalLinkage(lt))
			{
				visibility_ = DefaultVisibility;
			}
			linkage_ = lt;
		}

		LLVMModule const * Parent() const
		{
			return parent_;
		}
		LLVMModule* Parent()
		{
			return parent_;
		}

		static bool classof(Value const * val)
		{
			return (val->GetValueId() == Value::FunctionVal) || (val->GetValueId() == Value::GlobalVariableVal);
		}

	protected:
		GlobalValue(PointerType* ty, ValueTy vty, uint32_t num_ops, uint32_t num_uses, LinkageTypes linkage, std::string_view name);
		uint32_t GlobalValueSubClassData() const
		{
			return sub_class_data_;
		}
		void GlobalValueSubClassData(uint32_t v);

	protected:
		LLVMModule* parent_;

		// Note: VC++ treats enums as signed, so an extra bit is required to prevent
		// Linkage and Visibility from turning into negative values.
		LinkageTypes linkage_ : 5;
		uint8_t visibility_ : 2;
		uint8_t unnamed_addr_ : 1;
		uint8_t dll_storage_class_ : 2;

		static uint32_t constexpr GLOBAL_VALUE_SUB_CLASS_DATA_BITS = 19;

	private:
		// Give subclasses access to what otherwise would be wasted padding.
		// (19 + 3 + 2 + 1 + 2 + 5) == 32.
		uint32_t sub_class_data_ : GLOBAL_VALUE_SUB_CLASS_DATA_BITS;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_GLOBAL_VALUE_HPP
