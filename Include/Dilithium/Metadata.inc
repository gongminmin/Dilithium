/**
 * @file Metadata.inc
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

#if !(defined HANDLE_METADATA || defined HANDLE_METADATA_LEAF ||               \
	  defined HANDLE_METADATA_BRANCH || defined HANDLE_MDNODE_LEAF ||          \
	  defined HANDLE_MDNODE_BRANCH ||                                          \
	  defined HANDLE_SPECIALIZED_MDNODE_BRANCH)
	#error "Missing macro definition of HANDLE_METADATA*"
#endif

// Handler for all types of metadata.
#ifndef HANDLE_METADATA
	#define HANDLE_METADATA(CLASS)
#endif

// Handler for leaf nodes in the class hierarchy.
#ifndef HANDLE_METADATA_LEAF
	#define HANDLE_METADATA_LEAF(CLASS) HANDLE_METADATA(CLASS)
#endif

// Handler for non-leaf nodes in the class hierarchy.
#ifndef HANDLE_METADATA_BRANCH
	#define HANDLE_METADATA_BRANCH(CLASS) HANDLE_METADATA(CLASS)
#endif

// Handler for leaf nodes under MDNode.
#ifndef HANDLE_MDNODE_LEAF
	#define HANDLE_MDNODE_LEAF(CLASS) HANDLE_METADATA_LEAF(CLASS)
#endif

// Handler for non-leaf nodes under MDNode.
#ifndef HANDLE_MDNODE_BRANCH
	#define HANDLE_MDNODE_BRANCH(CLASS) HANDLE_METADATA_BRANCH(CLASS)
#endif

// Handler for specialized non-leaf nodes under MDNode.
#ifndef HANDLE_SPECIALIZED_MDNODE_BRANCH
	#define HANDLE_SPECIALIZED_MDNODE_BRANCH(CLASS) HANDLE_MDNODE_BRANCH(CLASS)
#endif

HANDLE_METADATA_LEAF(MDString)
HANDLE_METADATA_BRANCH(ValueAsMetadata)
HANDLE_METADATA_LEAF(ConstantAsMetadata)
HANDLE_METADATA_LEAF(LocalAsMetadata)
HANDLE_MDNODE_BRANCH(MDNode)
HANDLE_MDNODE_LEAF(MDTuple)

#undef HANDLE_METADATA
#undef HANDLE_METADATA_LEAF
#undef HANDLE_METADATA_BRANCH
#undef HANDLE_MDNODE_LEAF
#undef HANDLE_MDNODE_BRANCH
#undef HANDLE_SPECIALIZED_MDNODE_BRANCH
