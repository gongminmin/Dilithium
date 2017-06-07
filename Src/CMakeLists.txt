SET(LIB_NAME Dilithium)

SET(CXX17_HEADER_FILES
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/CXX17/string_view.hpp
)

SET(DETAIL_HEADER_FILES
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Detail/AutoLink.hpp
)

SET(DXC_HLSL_HEADER_FILES
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilCompType.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilConstants.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilContainer.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilMdHelper.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilModule.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilPipelineStateValidation.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilRootSignature.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilSemantic.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilShaderModel.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilSignature.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilSignatureElement.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/dxc/HLSL/DxilSigPoint.hpp
)

SET(HEADER_FILES
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Argument.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/ArrayRef.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Attributes.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/BasicBlock.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/BitcodeReader.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/BitCodes.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/BitstreamReader.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/CallingConv.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Casting.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Compiler.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Constant.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Constants.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/DataLayout.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/DerivedType.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Dilithium.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/ErrorHandling.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Function.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/GlobalObject.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/GlobalValue.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/GVMaterializer.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Half.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Hashing.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/InstrTypes.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Instruction.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Instruction.inc
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Instructions.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/LLVMBitCodes.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/LLVMContext.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/LLVMModule.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/MathExtras.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/MemStreamBuf.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Metadata.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Metadata.inc
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/MetadataTracking.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/MPFloat.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/MPInt.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/OperandTraits.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Operator.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/PointerUnion.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/SmallString.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/SymbolTableList.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/TrackingMDRef.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Type.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/TypeTraits.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Use.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/User.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Util.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Value.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/Value.inc
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/ValueHandle.hpp
	${DILITHIUM_ROOT_DIR}/Include/Dilithium/ValueSymbolTable.hpp
	${DILITHIUM_ROOT_DIR}/Src/AttributeImpl.hpp
	${DILITHIUM_ROOT_DIR}/Src/LLVMContextImpl.hpp
)

SET(HLSL_SOURCE_FILES
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilCompType.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilContainer.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilMdHelper.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilModule.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilSemantic.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilShaderModel.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilSignature.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilSignatureElement.cpp
	${DILITHIUM_ROOT_DIR}/Src/HLSL/DxilSigPoint.cpp
)

SET(SOURCE_FILES
	${DILITHIUM_ROOT_DIR}/Src/Argument.cpp
	${DILITHIUM_ROOT_DIR}/Src/AttributeImpl.cpp
	${DILITHIUM_ROOT_DIR}/Src/Attributes.cpp
	${DILITHIUM_ROOT_DIR}/Src/BasicBlock.cpp
	${DILITHIUM_ROOT_DIR}/Src/BitcodeReader.cpp
	${DILITHIUM_ROOT_DIR}/Src/BitstreamReader.cpp
	${DILITHIUM_ROOT_DIR}/Src/Constant.cpp
	${DILITHIUM_ROOT_DIR}/Src/Constants.cpp
	${DILITHIUM_ROOT_DIR}/Src/DataLayout.cpp
	${DILITHIUM_ROOT_DIR}/Src/DerivedType.cpp
	${DILITHIUM_ROOT_DIR}/Src/ErrorHandling.cpp
	${DILITHIUM_ROOT_DIR}/Src/Function.cpp
	${DILITHIUM_ROOT_DIR}/Src/GlobalObject.cpp
	${DILITHIUM_ROOT_DIR}/Src/GlobalValue.cpp
	${DILITHIUM_ROOT_DIR}/Src/Half.cpp
	${DILITHIUM_ROOT_DIR}/Src/Instruction.cpp
	${DILITHIUM_ROOT_DIR}/Src/Instructions.cpp
	${DILITHIUM_ROOT_DIR}/Src/LLVMContext.cpp
	${DILITHIUM_ROOT_DIR}/Src/LLVMContextImpl.cpp
	${DILITHIUM_ROOT_DIR}/Src/LLVMModule.cpp
	${DILITHIUM_ROOT_DIR}/Src/MemStreamBuf.cpp
	${DILITHIUM_ROOT_DIR}/Src/Metadata.cpp
	${DILITHIUM_ROOT_DIR}/Src/MetadataTracking.cpp
	${DILITHIUM_ROOT_DIR}/Src/MPFloat.cpp
	${DILITHIUM_ROOT_DIR}/Src/MPInt.cpp
	${DILITHIUM_ROOT_DIR}/Src/Operator.cpp
	${DILITHIUM_ROOT_DIR}/Src/Type.cpp
	${DILITHIUM_ROOT_DIR}/Src/Use.cpp
	${DILITHIUM_ROOT_DIR}/Src/User.cpp
	${DILITHIUM_ROOT_DIR}/Src/Value.cpp
	${DILITHIUM_ROOT_DIR}/Src/ValueHandle.cpp
	${DILITHIUM_ROOT_DIR}/Src/ValueSymbolTable.cpp
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Source Files\\HLSL" FILES ${HLSL_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Header Files\\CXX17" FILES ${CXX17_HEADER_FILES})
SOURCE_GROUP("Header Files\\Detail" FILES ${DETAIL_HEADER_FILES})
SOURCE_GROUP("Header Files\\dxc\\HLSL" FILES ${DXC_HLSL_HEADER_FILES})

ADD_DEFINITIONS(-DDILITHIUM_SOURCE)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

ADD_LIBRARY(${LIB_NAME} STATIC
	${HEADER_FILES} ${CXX17_HEADER_FILES} ${DETAIL_HEADER_FILES} ${DXC_HLSL_HEADER_FILES}
	${SOURCE_FILES} ${HLSL_SOURCE_FILES}
)

SET(LIB_OUTPUT_NAME ${LIB_NAME}${DILITHIUM_OUTPUT_SUFFIX})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${DILITHIUM_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${DILITHIUM_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${DILITHIUM_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${DILITHIUM_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${DILITHIUM_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}
)

TARGET_LINK_LIBRARIES(${LIB_NAME})
