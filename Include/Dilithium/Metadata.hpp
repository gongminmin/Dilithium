/**
 * @file Metadata.hpp
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

#ifndef _DILITHIUM_METADATA_HPP
#define _DILITHIUM_METADATA_HPP

#pragma once

#include <memory>

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class MDNode;
	class MDTuple;

	class Metadata
	{
	public:
		enum MetadataKind
		{
			MDTupleKind,
			ConstantAsMetadataKind,
			LocalAsMetadataKind,
			MDStringKind
		};

	public:
		uint32_t GetMetadataId() const
		{
			return subclass_id_;
		}

	private:
		uint8_t const subclass_id_;

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class MetadataAsValue : public Value
	{
	public:
		static MetadataAsValue* Get(LLVMContext& context, Metadata* md);

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ReplaceableMetadataImpl
	{
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class ValueAsMetadata : public Metadata, ReplaceableMetadataImpl
	{
	public:
		static ValueAsMetadata* Get(Value* val);

		static void HandleDeletion(Value* val);
		static void HandleRAUW(Value* from, Value* to);
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class MDString : boost::noncopyable, public Metadata
	{
	public:
		static MDString* Get(LLVMContext& context, std::string_view sv);
		// DILITHIUM_NOT_IMPLEMENTED
	};


	struct TempMDNodeDeleter
	{
		inline void operator()(MDNode *Node) const;
	};

#define HANDLE_MDNODE_LEAF(CLASS) typedef std::unique_ptr<CLASS, TempMDNodeDeleter> Temp##CLASS;
#define HANDLE_MDNODE_BRANCH(CLASS) HANDLE_MDNODE_LEAF(CLASS)
#include "Metadata.inc"

	class MDNode : boost::noncopyable, public Metadata
	{
	public:
		static MDTuple* Get(LLVMContext& context, ArrayRef<Metadata*> mds);
		static MDTuple* GetIfExists(LLVMContext& context, ArrayRef<Metadata*> mds);
		static MDTuple* GetDistinct(LLVMContext& context, ArrayRef<Metadata*> mds);
		//static TempMDTuple GetTemporary(LLVMContext& context, ArrayRef<Metadata*> mds);

		void ReplaceAllUsesWith(Metadata* md);

		static bool classof(Metadata const * md)
		{
			switch (md->GetMetadataId()) {
			default:
				return false;
#define HANDLE_MDNODE_LEAF(CLASS)			\
			case CLASS##Kind:				\
				return true;
#include "Metadata.inc"
			}
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class MDTuple : public MDNode
	{
	public:
		static bool classof(Metadata const * md)
		{
			return md->GetMetadataId() == MDTupleKind;
		}
		// DILITHIUM_NOT_IMPLEMENTED
	};

	class NamedMDNode : boost::noncopyable
	{
	public:
		void AddOperand(MDNode* mn);
		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_METADATA_HPP
