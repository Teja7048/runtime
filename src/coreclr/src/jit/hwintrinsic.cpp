// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "jitpch.h"
#include "hwintrinsic.h"

#ifdef FEATURE_HW_INTRINSICS

static const HWIntrinsicInfo hwIntrinsicInfoArray[] = {
// clang-format off
#if defined(TARGET_XARCH)
#define HARDWARE_INTRINSIC(isa, name, size, numarg, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, category, flag) \
    {NI_##isa##_##name, #name, InstructionSet_##isa, size, numarg, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, category, static_cast<HWIntrinsicFlag>(flag)},
#include "hwintrinsiclistxarch.h"
#elif defined (TARGET_ARM64)
#define HARDWARE_INTRINSIC(isa, name, size, numarg, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, category, flag) \
    {NI_##isa##_##name, #name, InstructionSet_##isa, size, numarg, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, category, static_cast<HWIntrinsicFlag>(flag)},
#include "hwintrinsiclistarm64.h"
#else
#error Unsupported platform
#endif
    // clang-format on
};

//------------------------------------------------------------------------
// lookup: Gets the HWIntrinsicInfo associated with a given NamedIntrinsic
//
// Arguments:
//    id -- The NamedIntrinsic associated with the HWIntrinsic to lookup
//
// Return Value:
//    The HWIntrinsicInfo associated with id
const HWIntrinsicInfo& HWIntrinsicInfo::lookup(NamedIntrinsic id)
{
    assert(id != NI_Illegal);

    assert(id > NI_HW_INTRINSIC_START);
    assert(id < NI_HW_INTRINSIC_END);

    return hwIntrinsicInfoArray[id - NI_HW_INTRINSIC_START - 1];
}

//------------------------------------------------------------------------
// getBaseTypeFromArgIfNeeded: Get baseType of intrinsic from 1st or 2nd argument depending on the flag
//
// Arguments:
//    intrinsic -- id of the intrinsic function.
//    clsHnd    -- class handle containing the intrinsic function.
//    method    -- method handle of the intrinsic function.
//    sig       -- signature of the intrinsic call.
//    baseType  -- Predetermined baseType, could be TYP_UNKNOWN
//
// Return Value:
//    The basetype of intrinsic of it can be fetched from 1st or 2nd argument, else return baseType unmodified.
//
var_types Compiler::getBaseTypeFromArgIfNeeded(NamedIntrinsic       intrinsic,
                                               CORINFO_CLASS_HANDLE clsHnd,
                                               CORINFO_SIG_INFO*    sig,
                                               var_types            baseType)
{
    if (HWIntrinsicInfo::BaseTypeFromSecondArg(intrinsic) || HWIntrinsicInfo::BaseTypeFromFirstArg(intrinsic))
    {
        CORINFO_ARG_LIST_HANDLE arg = sig->args;

        if (HWIntrinsicInfo::BaseTypeFromSecondArg(intrinsic))
        {
            arg = info.compCompHnd->getArgNext(arg);
        }

        CORINFO_CLASS_HANDLE argClass = info.compCompHnd->getArgClass(sig, arg);
        baseType                      = getBaseTypeAndSizeOfSIMDType(argClass);

        if (baseType == TYP_UNKNOWN) // the argument is not a vector
        {
            CORINFO_CLASS_HANDLE tmpClass;
            CorInfoType          corInfoType = strip(info.compCompHnd->getArgType(sig, arg, &tmpClass));

            if (corInfoType == CORINFO_TYPE_PTR)
            {
                corInfoType = info.compCompHnd->getChildType(argClass, &tmpClass);
            }

            baseType = JITtype2varType(corInfoType);
        }
        assert(baseType != TYP_UNKNOWN);
    }

    return baseType;
}

CORINFO_CLASS_HANDLE Compiler::gtGetStructHandleForHWSIMD(var_types simdType, var_types simdBaseType)
{
    if (simdType == TYP_SIMD16)
    {
        switch (simdBaseType)
        {
            case TYP_FLOAT:
                return m_simdHandleCache->Vector128FloatHandle;
            case TYP_DOUBLE:
                return m_simdHandleCache->Vector128DoubleHandle;
            case TYP_INT:
                return m_simdHandleCache->Vector128IntHandle;
            case TYP_USHORT:
                return m_simdHandleCache->Vector128UShortHandle;
            case TYP_UBYTE:
                return m_simdHandleCache->Vector128UByteHandle;
            case TYP_SHORT:
                return m_simdHandleCache->Vector128ShortHandle;
            case TYP_BYTE:
                return m_simdHandleCache->Vector128ByteHandle;
            case TYP_LONG:
                return m_simdHandleCache->Vector128LongHandle;
            case TYP_UINT:
                return m_simdHandleCache->Vector128UIntHandle;
            case TYP_ULONG:
                return m_simdHandleCache->Vector128ULongHandle;
            default:
                assert(!"Didn't find a class handle for simdType");
        }
    }
#ifdef TARGET_XARCH
    else if (simdType == TYP_SIMD32)
    {
        switch (simdBaseType)
        {
            case TYP_FLOAT:
                return m_simdHandleCache->Vector256FloatHandle;
            case TYP_DOUBLE:
                return m_simdHandleCache->Vector256DoubleHandle;
            case TYP_INT:
                return m_simdHandleCache->Vector256IntHandle;
            case TYP_USHORT:
                return m_simdHandleCache->Vector256UShortHandle;
            case TYP_UBYTE:
                return m_simdHandleCache->Vector256UByteHandle;
            case TYP_SHORT:
                return m_simdHandleCache->Vector256ShortHandle;
            case TYP_BYTE:
                return m_simdHandleCache->Vector256ByteHandle;
            case TYP_LONG:
                return m_simdHandleCache->Vector256LongHandle;
            case TYP_UINT:
                return m_simdHandleCache->Vector256UIntHandle;
            case TYP_ULONG:
                return m_simdHandleCache->Vector256ULongHandle;
            default:
                assert(!"Didn't find a class handle for simdType");
        }
    }
#endif // TARGET_XARCH
#ifdef TARGET_ARM64
    else if (simdType == TYP_SIMD8)
    {
        switch (simdBaseType)
        {
            case TYP_FLOAT:
                return m_simdHandleCache->Vector64FloatHandle;
            case TYP_DOUBLE:
                return m_simdHandleCache->Vector64DoubleHandle;
            case TYP_INT:
                return m_simdHandleCache->Vector64IntHandle;
            case TYP_USHORT:
                return m_simdHandleCache->Vector64UShortHandle;
            case TYP_UBYTE:
                return m_simdHandleCache->Vector64UByteHandle;
            case TYP_SHORT:
                return m_simdHandleCache->Vector64ShortHandle;
            case TYP_BYTE:
                return m_simdHandleCache->Vector64ByteHandle;
            case TYP_UINT:
                return m_simdHandleCache->Vector64UIntHandle;
            case TYP_LONG:
                return m_simdHandleCache->Vector64LongHandle;
            case TYP_ULONG:
                return m_simdHandleCache->Vector64ULongHandle;
            default:
                assert(!"Didn't find a class handle for simdType");
        }
    }
#endif // TARGET_ARM64

    return NO_CLASS_HANDLE;
}

//------------------------------------------------------------------------
// vnEncodesResultTypeForHWIntrinsic(NamedIntrinsic hwIntrinsicID):
//
// Arguments:
//    hwIntrinsicID -- The id for the HW intrinsic
//
// Return Value:
//   Returns true if this intrinsic requires value numbering to add an
//   extra SimdType argument that encodes the resulting type.
//   If we don't do this overloaded versions can return the same VN
//   leading to incorrect CSE subsitutions.
//
/* static */ bool Compiler::vnEncodesResultTypeForHWIntrinsic(NamedIntrinsic hwIntrinsicID)
{
    int numArgs = HWIntrinsicInfo::lookupNumArgs(hwIntrinsicID);

    // HW Instrinsic's with -1 for numArgs have a varying number of args, so we currently
    // give themm a unique value number them, and don't add an extra argument.
    //
    if (numArgs == -1)
    {
        return false;
    }

    // We iterate over all of the different baseType's for this instrinsic in the HWIntrinsicInfo table
    // We set  diffInsCount to the number of instructions that can execute differently.
    //
    unsigned diffInsCount = 0;
#ifdef TARGET_XARCH
    instruction lastIns = INS_invalid;
#endif
    for (var_types baseType = TYP_BYTE; (baseType <= TYP_DOUBLE); baseType = (var_types)(baseType + 1))
    {
        instruction curIns = HWIntrinsicInfo::lookupIns(hwIntrinsicID, baseType);
        if (curIns != INS_invalid)
        {
#ifdef TARGET_XARCH
            if (curIns != lastIns)
            {
                diffInsCount++;
                // remember the last valid instruction that we saw
                lastIns = curIns;
            }
#elif defined(TARGET_ARM64)
            // On ARM64 we use the same instruction and specify an insOpt arrangement
            // so we always consider the instruction operation to be different
            //
            diffInsCount++;
#endif // TARGET
            if (diffInsCount >= 2)
            {
                // We can  early exit the loop now
                break;
            }
        }
    }

    // If we see two (or more) different instructions we need the extra VNF_SimdType arg
    return (diffInsCount >= 2);
}

//------------------------------------------------------------------------
// lookupId: Gets the NamedIntrinsic for a given method name and InstructionSet
//
// Arguments:
//    comp       -- The compiler
//    sig        -- The signature of the intrinsic
//    className  -- The name of the class associated with the HWIntrinsic to lookup
//    methodName -- The name of the method associated with the HWIntrinsic to lookup
//    enclosingClassName -- The name of the enclosing class of X64 classes
//
// Return Value:
//    The NamedIntrinsic associated with methodName and isa
NamedIntrinsic HWIntrinsicInfo::lookupId(Compiler*         comp,
                                         CORINFO_SIG_INFO* sig,
                                         const char*       className,
                                         const char*       methodName,
                                         const char*       enclosingClassName)
{
    // TODO-Throughput: replace sequential search by binary search
    CORINFO_InstructionSet isa = lookupIsa(className, enclosingClassName);

    if (isa == InstructionSet_ILLEGAL)
    {
        return NI_Illegal;
    }

    bool isIsaSupported = comp->compExactlyDependsOn(isa) && comp->compSupportsHWIntrinsic(isa);

    if (strcmp(methodName, "get_IsSupported") == 0)
    {
        return isIsaSupported ? NI_IsSupported_True : NI_IsSupported_False;
    }
    else if (!isIsaSupported)
    {
        return NI_Throw_PlatformNotSupportedException;
    }

    for (int i = 0; i < (NI_HW_INTRINSIC_END - NI_HW_INTRINSIC_START - 1); i++)
    {
        const HWIntrinsicInfo& intrinsicInfo = hwIntrinsicInfoArray[i];

        if (isa != hwIntrinsicInfoArray[i].isa)
        {
            continue;
        }

        int numArgs = static_cast<unsigned>(intrinsicInfo.numArgs);

        if ((numArgs != -1) && (sig->numArgs != static_cast<unsigned>(intrinsicInfo.numArgs)))
        {
            continue;
        }

        if (strcmp(methodName, intrinsicInfo.name) == 0)
        {
            return intrinsicInfo.id;
        }
    }

    // There are several helper intrinsics that are implemented in managed code
    // Those intrinsics will hit this code path and need to return NI_Illegal
    return NI_Illegal;
}

//------------------------------------------------------------------------
// lookupSimdSize: Gets the SimdSize for a given HWIntrinsic and signature
//
// Arguments:
//    id -- The ID associated with the HWIntrinsic to lookup
//   sig -- The signature of the HWIntrinsic to lookup
//
// Return Value:
//    The SIMD size for the HWIntrinsic associated with id and sig
//
// Remarks:
//    This function is only used by the importer. After importation, we can
//    get the SIMD size from the GenTreeHWIntrinsic node.
unsigned HWIntrinsicInfo::lookupSimdSize(Compiler* comp, NamedIntrinsic id, CORINFO_SIG_INFO* sig)
{
    unsigned simdSize = 0;

    if (tryLookupSimdSize(id, &simdSize))
    {
        return simdSize;
    }

    CORINFO_CLASS_HANDLE typeHnd = nullptr;

    if (HWIntrinsicInfo::BaseTypeFromFirstArg(id))
    {
        typeHnd = comp->info.compCompHnd->getArgClass(sig, sig->args);
    }
    else if (HWIntrinsicInfo::BaseTypeFromSecondArg(id))
    {
        CORINFO_ARG_LIST_HANDLE secondArg = comp->info.compCompHnd->getArgNext(sig->args);
        typeHnd                           = comp->info.compCompHnd->getArgClass(sig, secondArg);
    }
    else
    {
        assert(JITtype2varType(sig->retType) == TYP_STRUCT);
        typeHnd = sig->retTypeSigClass;
    }

    var_types baseType = comp->getBaseTypeAndSizeOfSIMDType(typeHnd, &simdSize);
    assert((simdSize > 0) && (baseType != TYP_UNKNOWN));
    return simdSize;
}

//------------------------------------------------------------------------
// lookupNumArgs: Gets the number of args for a given HWIntrinsic node
//
// Arguments:
//    node -- The HWIntrinsic node to get the number of args for
//
// Return Value:
//    The number of args for the HWIntrinsic associated with node
int HWIntrinsicInfo::lookupNumArgs(const GenTreeHWIntrinsic* node)
{
    assert(node != nullptr);

    NamedIntrinsic id      = node->gtHWIntrinsicId;
    int            numArgs = lookupNumArgs(id);

    if (numArgs >= 0)
    {
        return numArgs;
    }

    assert(numArgs == -1);

    GenTree* op1 = node->gtGetOp1();

    if (op1 == nullptr)
    {
        return 0;
    }

    if (op1->OperIsList())
    {
        GenTreeArgList* list = op1->AsArgList();
        numArgs              = 0;

        do
        {
            numArgs++;
            list = list->Rest();
        } while (list != nullptr);

        return numArgs;
    }

    GenTree* op2 = node->gtGetOp2();

    return (op2 == nullptr) ? 1 : 2;
}

//------------------------------------------------------------------------
// lookupLastOp: Gets the last operand for a given HWIntrinsic node
//
// Arguments:
//    node   -- The HWIntrinsic node to get the last operand for
//
// Return Value:
//     The last operand for node
GenTree* HWIntrinsicInfo::lookupLastOp(const GenTreeHWIntrinsic* node)
{
    assert(node != nullptr);

    NamedIntrinsic id  = node->gtHWIntrinsicId;
    GenTree*       op1 = node->gtGetOp1();

    if (op1 == nullptr)
    {
        return nullptr;
    }

    if (op1->OperIsList())
    {
        GenTreeArgList* list = op1->AsArgList();
        GenTree*        last;

        do
        {
            last = list->Current();
            list = list->Rest();
        } while (list != nullptr);

        return last;
    }

    GenTree* op2 = node->gtGetOp2();

    return (op2 == nullptr) ? op1 : op2;
}

//------------------------------------------------------------------------
// isImmOp: Checks whether the HWIntrinsic node has an imm operand
//
// Arguments:
//    id -- The NamedIntrinsic associated with the HWIntrinsic to lookup
//    op -- The operand to check
//
// Return Value:
//     true if the node has an imm operand; otherwise, false
bool HWIntrinsicInfo::isImmOp(NamedIntrinsic id, const GenTree* op)
{
    if (HWIntrinsicInfo::lookupCategory(id) != HW_Category_IMM)
    {
        return false;
    }

    if (!HWIntrinsicInfo::MaybeImm(id))
    {
        return true;
    }

    if (genActualType(op->TypeGet()) != TYP_INT)
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
// getArgForHWIntrinsic: pop an argument from the stack and validate its type
//
// Arguments:
//    argType    -- the required type of argument
//    argClass   -- the class handle of argType
//    expectAddr --  if true indicates we are expecting type stack entry to be a TYP_BYREF.
//
// Return Value:
//     the validated argument
//
GenTree* Compiler::getArgForHWIntrinsic(var_types argType, CORINFO_CLASS_HANDLE argClass, bool expectAddr)
{
    GenTree* arg = nullptr;
    if (varTypeIsStruct(argType))
    {
        if (!varTypeIsSIMD(argType))
        {
            unsigned int argSizeBytes;
            var_types    base = getBaseTypeAndSizeOfSIMDType(argClass, &argSizeBytes);
            argType           = getSIMDTypeForSize(argSizeBytes);
        }
        assert(varTypeIsSIMD(argType));
        arg = impSIMDPopStack(argType, expectAddr);
        assert(varTypeIsSIMD(arg->TypeGet()));
    }
    else
    {
        assert(varTypeIsArithmetic(argType));
        arg = impPopStack().val;
        assert(varTypeIsArithmetic(arg->TypeGet()));
        assert(genActualType(arg->gtType) == genActualType(argType));
    }
    return arg;
}

//------------------------------------------------------------------------
// addRangeCheckIfNeeded: add a GT_HW_INTRINSIC_CHK node for non-full-range imm-intrinsic
//
// Arguments:
//    intrinsic     -- intrinsic ID
//    immOp         -- the immediate operand of the intrinsic
//    mustExpand    -- true if the compiler is compiling the fallback(GT_CALL) of this intrinsics
//    immLowerBound -- lower incl. bound for a value of the immediate operand (for a non-full-range imm-intrinsic)
//    immUpperBound -- upper incl. bound for a value of the immediate operand (for a non-full-range imm-intrinsic)
//
// Return Value:
//     add a GT_HW_INTRINSIC_CHK node for non-full-range imm-intrinsic, which would throw ArgumentOutOfRangeException
//     when the imm-argument is not in the valid range
//
GenTree* Compiler::addRangeCheckIfNeeded(
    NamedIntrinsic intrinsic, GenTree* immOp, bool mustExpand, int immLowerBound, int immUpperBound)
{
    assert(immOp != nullptr);
    // Full-range imm-intrinsics do not need the range-check
    // because the imm-parameter of the intrinsic method is a byte.
    // AVX2 Gather intrinsics no not need the range-check
    // because their imm-parameter have discrete valid values that are handle by managed code
    if (mustExpand && !HWIntrinsicInfo::HasFullRangeImm(intrinsic) && HWIntrinsicInfo::isImmOp(intrinsic, immOp)
#ifdef TARGET_XARCH
        && !HWIntrinsicInfo::isAVX2GatherIntrinsic(intrinsic)
#endif
            )
    {
        assert(!immOp->IsCnsIntOrI());
        assert(varTypeIsUnsigned(immOp));

        // Bounds check for value of an immediate operand
        //   (immLowerBound <= immOp) && (immOp <= immUpperBound)
        //
        // implemented as a single comparison in the form of
        //
        // if ((immOp - immLowerBound) >= (immUpperBound - immLowerBound + 1))
        // {
        //     throw new ArgumentOutOfRangeException();
        // }
        //
        // The value of (immUpperBound - immLowerBound + 1) is denoted as adjustedUpperBound.

        const ssize_t adjustedUpperBound     = (ssize_t)immUpperBound - immLowerBound + 1;
        GenTree*      adjustedUpperBoundNode = gtNewIconNode(adjustedUpperBound, TYP_INT);

        GenTree* immOpDup = nullptr;

        immOp = impCloneExpr(immOp, &immOpDup, NO_CLASS_HANDLE, (unsigned)CHECK_SPILL_ALL,
                             nullptr DEBUGARG("Clone an immediate operand for immediate value bounds check"));

        if (immLowerBound != 0)
        {
            immOpDup = gtNewOperNode(GT_SUB, TYP_INT, immOpDup, gtNewIconNode(immLowerBound, TYP_INT));
        }

        GenTreeBoundsChk* hwIntrinsicChk = new (this, GT_HW_INTRINSIC_CHK)
            GenTreeBoundsChk(GT_HW_INTRINSIC_CHK, TYP_VOID, immOpDup, adjustedUpperBoundNode, SCK_RNGCHK_FAIL);
        hwIntrinsicChk->gtThrowKind = SCK_ARG_RNG_EXCPN;

        return gtNewOperNode(GT_COMMA, immOp->TypeGet(), hwIntrinsicChk, immOp);
    }
    else
    {
        return immOp;
    }
}

//------------------------------------------------------------------------
// compSupportsHWIntrinsic: check whether a given instruction set is supported
//
// Arguments:
//    isa - Instruction set
//
// Return Value:
//    true iff the given instruction set is supported in the current compilation.
bool Compiler::compSupportsHWIntrinsic(CORINFO_InstructionSet isa)
{
    return JitConfig.EnableHWIntrinsic() && (featureSIMD || HWIntrinsicInfo::isScalarIsa(isa)) &&
           (
#ifdef DEBUG
               JitConfig.EnableIncompleteISAClass() ||
#endif
               HWIntrinsicInfo::isFullyImplementedIsa(isa));
}

//------------------------------------------------------------------------
// impIsTableDrivenHWIntrinsic:
//
// Arguments:
//    intrinsicId - HW intrinsic id
//    category - category of a HW intrinsic
//
// Return Value:
//    returns true if this category can be table-driven in the importer
//
static bool impIsTableDrivenHWIntrinsic(NamedIntrinsic intrinsicId, HWIntrinsicCategory category)
{
    return (category != HW_Category_Special) && HWIntrinsicInfo::RequiresCodegen(intrinsicId) &&
           !HWIntrinsicInfo::HasSpecialImport(intrinsicId);
}

//------------------------------------------------------------------------
// isSupportedBaseType
//
// Arguments:
//    intrinsicId - HW intrinsic id
//    baseType - Base type of the intrinsic.
//
// Return Value:
//    returns true if the baseType is supported for given intrinsic.
//
static bool isSupportedBaseType(NamedIntrinsic intrinsic, var_types baseType)
{
    // We don't actually check the intrinsic outside of the false case as we expect
    // the exposed managed signatures are either generic and support all types
    // or they are explicit and support the type indicated.
    if (varTypeIsArithmetic(baseType))
    {
        return true;
    }

#ifdef TARGET_XARCH
    assert((intrinsic == NI_Vector128_As) || (intrinsic == NI_Vector128_AsByte) ||
           (intrinsic == NI_Vector128_AsDouble) || (intrinsic == NI_Vector128_AsInt16) ||
           (intrinsic == NI_Vector128_AsInt32) || (intrinsic == NI_Vector128_AsInt64) ||
           (intrinsic == NI_Vector128_AsSByte) || (intrinsic == NI_Vector128_AsSingle) ||
           (intrinsic == NI_Vector128_AsUInt16) || (intrinsic == NI_Vector128_AsUInt32) ||
           (intrinsic == NI_Vector128_AsUInt64) || (intrinsic == NI_Vector128_get_AllBitsSet) ||
           (intrinsic == NI_Vector128_get_Count) || (intrinsic == NI_Vector128_get_Zero) ||
           (intrinsic == NI_Vector128_GetElement) || (intrinsic == NI_Vector128_WithElement) ||
           (intrinsic == NI_Vector128_ToScalar) || (intrinsic == NI_Vector128_ToVector256) ||
           (intrinsic == NI_Vector128_ToVector256Unsafe) || (intrinsic == NI_Vector256_As) ||
           (intrinsic == NI_Vector256_AsByte) || (intrinsic == NI_Vector256_AsDouble) ||
           (intrinsic == NI_Vector256_AsInt16) || (intrinsic == NI_Vector256_AsInt32) ||
           (intrinsic == NI_Vector256_AsInt64) || (intrinsic == NI_Vector256_AsSByte) ||
           (intrinsic == NI_Vector256_AsSingle) || (intrinsic == NI_Vector256_AsUInt16) ||
           (intrinsic == NI_Vector256_AsUInt32) || (intrinsic == NI_Vector256_AsUInt64) ||
           (intrinsic == NI_Vector256_get_AllBitsSet) || (intrinsic == NI_Vector256_get_Count) ||
           (intrinsic == NI_Vector256_get_Zero) || (intrinsic == NI_Vector256_GetElement) ||
           (intrinsic == NI_Vector256_WithElement) || (intrinsic == NI_Vector256_GetLower) ||
           (intrinsic == NI_Vector256_ToScalar));
#endif // TARGET_XARCH
#ifdef TARGET_ARM64
    assert((intrinsic == NI_Vector64_AsByte) || (intrinsic == NI_Vector64_AsDouble) ||
           (intrinsic == NI_Vector64_AsInt16) || (intrinsic == NI_Vector64_AsInt32) ||
           (intrinsic == NI_Vector64_AsInt64) || (intrinsic == NI_Vector64_AsSByte) ||
           (intrinsic == NI_Vector64_AsSingle) || (intrinsic == NI_Vector64_AsUInt16) ||
           (intrinsic == NI_Vector64_AsUInt32) || (intrinsic == NI_Vector64_AsUInt64) ||
           (intrinsic == NI_Vector64_get_AllBitsSet) || (intrinsic == NI_Vector64_get_Count) ||
           (intrinsic == NI_Vector64_get_Zero) || (intrinsic == NI_Vector64_GetElement) ||
           (intrinsic == NI_Vector64_ToScalar) || (intrinsic == NI_Vector64_ToVector128) ||
           (intrinsic == NI_Vector64_ToVector128Unsafe) || (intrinsic == NI_Vector128_As) ||
           (intrinsic == NI_Vector128_AsByte) || (intrinsic == NI_Vector128_AsDouble) ||
           (intrinsic == NI_Vector128_AsInt16) || (intrinsic == NI_Vector128_AsInt32) ||
           (intrinsic == NI_Vector128_AsInt64) || (intrinsic == NI_Vector128_AsSByte) ||
           (intrinsic == NI_Vector128_AsSingle) || (intrinsic == NI_Vector128_AsUInt16) ||
           (intrinsic == NI_Vector128_AsUInt32) || (intrinsic == NI_Vector128_AsUInt64) ||
           (intrinsic == NI_Vector128_get_AllBitsSet) || (intrinsic == NI_Vector128_get_Count) ||
           (intrinsic == NI_Vector128_get_Zero) || (intrinsic == NI_Vector128_GetElement) ||
           (intrinsic == NI_Vector128_GetLower) || (intrinsic == NI_Vector128_ToScalar));
#endif // TARGET_ARM64
    return false;
}

//------------------------------------------------------------------------
// impHWIntrinsic: Import a hardware intrinsic as a GT_HWINTRINSIC node if possible
//
// Arguments:
//    intrinsic  -- id of the intrinsic function.
//    clsHnd     -- class handle containing the intrinsic function.
//    method     -- method handle of the intrinsic function.
//    sig        -- signature of the intrinsic call
//    mustExpand -- true if the intrinsic must return a GenTree*; otherwise, false

// Return Value:
//    The GT_HWINTRINSIC node, or nullptr if not a supported intrinsic
//
GenTree* Compiler::impHWIntrinsic(NamedIntrinsic        intrinsic,
                                  CORINFO_CLASS_HANDLE  clsHnd,
                                  CORINFO_METHOD_HANDLE method,
                                  CORINFO_SIG_INFO*     sig,
                                  bool                  mustExpand)
{
    CORINFO_InstructionSet isa      = HWIntrinsicInfo::lookupIsa(intrinsic);
    HWIntrinsicCategory    category = HWIntrinsicInfo::lookupCategory(intrinsic);
    int                    numArgs  = sig->numArgs;
    var_types              retType  = JITtype2varType(sig->retType);
    var_types              baseType = TYP_UNKNOWN;

    if ((retType == TYP_STRUCT) && featureSIMD)
    {
        unsigned int sizeBytes;
        baseType = getBaseTypeAndSizeOfSIMDType(sig->retTypeSigClass, &sizeBytes);
        retType  = getSIMDTypeForSize(sizeBytes);
        assert(sizeBytes != 0);

        // We want to return early here for cases where retType was TYP_STRUCT as per method signature and
        // rather than deferring the decision after getting the baseType of arg.
        if (!isSupportedBaseType(intrinsic, baseType))
        {
            return nullptr;
        }
    }

    baseType = getBaseTypeFromArgIfNeeded(intrinsic, clsHnd, sig, baseType);

    if (baseType == TYP_UNKNOWN)
    {
        if (category != HW_Category_Scalar)
        {
            unsigned int sizeBytes;
            baseType = getBaseTypeAndSizeOfSIMDType(clsHnd, &sizeBytes);
            assert((category == HW_Category_Special) || (sizeBytes != 0));
        }
        else
        {
            baseType = retType;
        }
    }

    // Immediately return if the category is other than scalar/special and this is not a supported base type.
    if ((category != HW_Category_Special) && (category != HW_Category_Scalar) &&
        !isSupportedBaseType(intrinsic, baseType))
    {
        return nullptr;
    }

    unsigned simdSize = HWIntrinsicInfo::lookupSimdSize(this, intrinsic, sig);

    GenTree* immOp = nullptr;

#ifdef TARGET_ARM64
    if (intrinsic == NI_AdvSimd_Insert)
    {
        assert(sig->numArgs == 3);
        immOp = impStackTop(1).val;
        assert(HWIntrinsicInfo::isImmOp(intrinsic, immOp));
    }
    else
#endif
        if ((sig->numArgs > 0) && HWIntrinsicInfo::isImmOp(intrinsic, impStackTop().val))
    {
        // NOTE: The following code assumes that for all intrinsics
        // taking an immediate operand, that operand will be last.
        immOp = impStackTop().val;
    }

    if (immOp != nullptr)
    {
        if (!HWIntrinsicInfo::HasFullRangeImm(intrinsic) && immOp->IsCnsIntOrI())
        {
            const int ival = (int)immOp->AsIntCon()->IconValue();

            if (!HWIntrinsicInfo::isInImmRange(intrinsic, ival, simdSize, baseType))
            {
                assert(!mustExpand);
                // The imm-HWintrinsics that do not accept all imm8 values may throw
                // ArgumentOutOfRangeException when the imm argument is not in the valid range
                return nullptr;
            }
        }
        else if (!immOp->IsCnsIntOrI())
        {
            if (HWIntrinsicInfo::NoJmpTableImm(intrinsic))
            {
                return impNonConstFallback(intrinsic, retType, baseType);
            }
            else if (!mustExpand)
            {
                // When the imm-argument is not a constant and we are not being forced to expand, we need to
                // return nullptr so a GT_CALL to the intrinsic method is emitted instead. The
                // intrinsic method is recursive and will be forced to expand, at which point
                // we emit some less efficient fallback code.
                return nullptr;
            }
        }
    }

    if (HWIntrinsicInfo::IsFloatingPointUsed(intrinsic))
    {
        // Set `compFloatingPointUsed` to cover the scenario where an intrinsic is operating on SIMD fields, but
        // where no SIMD local vars are in use. This is the same logic as is used for FEATURE_SIMD.
        compFloatingPointUsed = true;
    }

    // table-driven importer of simple intrinsics
    if (impIsTableDrivenHWIntrinsic(intrinsic, category))
    {
        bool                    isScalar = category == HW_Category_Scalar;
        CORINFO_ARG_LIST_HANDLE argList  = sig->args;
        var_types               argType  = TYP_UNKNOWN;
        CORINFO_CLASS_HANDLE    argClass;

        int immLowerBound = 0;
        int immUpperBound = 0;

        if (immOp != nullptr)
        {
#if defined(TARGET_XARCH)
            immUpperBound = HWIntrinsicInfo::lookupImmUpperBound(intrinsic);
#elif defined(TARGET_ARM64)
            HWIntrinsicInfo::lookupImmBounds(intrinsic, simdSize, baseType, &immLowerBound, &immUpperBound);
#endif
        }

        assert(numArgs >= 0);

        if (!isScalar && ((HWIntrinsicInfo::lookupIns(intrinsic, baseType) == INS_invalid) ||
                          ((simdSize != 8) && (simdSize != 16) && (simdSize != 32))))
        {
            assert(!"Unexpected HW Intrinsic");
            return nullptr;
        }

        GenTree*            op1     = nullptr;
        GenTree*            op2     = nullptr;
        GenTree*            op3     = nullptr;
        GenTreeHWIntrinsic* retNode = nullptr;

        switch (numArgs)
        {
            case 0:
            {
                assert(!isScalar);
                retNode = gtNewSimdHWIntrinsicNode(retType, intrinsic, baseType, simdSize);
                break;
            }

            case 1:
            {
                argType = JITtype2varType(strip(info.compCompHnd->getArgType(sig, argList, &argClass)));
                op1     = getArgForHWIntrinsic(argType, argClass);

                if (category == HW_Category_MemoryLoad && op1->OperIs(GT_CAST))
                {
                    // Although the API specifies a pointer, if what we have is a BYREF, that's what
                    // we really want, so throw away the cast.
                    if (op1->gtGetOp1()->TypeGet() == TYP_BYREF)
                    {
                        op1 = op1->gtGetOp1();
                    }
                }

                retNode = isScalar ? gtNewScalarHWIntrinsicNode(retType, op1, intrinsic)
                                   : gtNewSimdHWIntrinsicNode(retType, op1, intrinsic, baseType, simdSize);
                break;
            }

            case 2:
            {
                CORINFO_ARG_LIST_HANDLE arg2 = info.compCompHnd->getArgNext(argList);
                argType = JITtype2varType(strip(info.compCompHnd->getArgType(sig, arg2, &argClass)));
                op2     = getArgForHWIntrinsic(argType, argClass);

                op2 = addRangeCheckIfNeeded(intrinsic, op2, mustExpand, immLowerBound, immUpperBound);

                argType = JITtype2varType(strip(info.compCompHnd->getArgType(sig, argList, &argClass)));
                op1     = getArgForHWIntrinsic(argType, argClass);

                retNode = isScalar ? gtNewScalarHWIntrinsicNode(retType, op1, op2, intrinsic)
                                   : gtNewSimdHWIntrinsicNode(retType, op1, op2, intrinsic, baseType, simdSize);
#ifdef TARGET_XARCH
                if (intrinsic == NI_SSE42_Crc32 || intrinsic == NI_SSE42_X64_Crc32)
#endif
#ifdef TARGET_ARM64
                    if (intrinsic == NI_Crc32_ComputeCrc32 || intrinsic == NI_Crc32_ComputeCrc32C ||
                        intrinsic == NI_Crc32_Arm64_ComputeCrc32 || intrinsic == NI_Crc32_Arm64_ComputeCrc32C)
#endif
                    {
                        // type of the second argument
                        CorInfoType corType = strip(info.compCompHnd->getArgType(sig, arg2, &argClass));

                        // TODO - currently we use the BaseType to bring the type of the second argument
                        // to the code generator. May encode the overload info in other way.
                        retNode->AsHWIntrinsic()->gtSIMDBaseType = JITtype2varType(corType);
                    }
#ifdef TARGET_ARM64
                if ((intrinsic == NI_AdvSimd_AddWideningUpper) || (intrinsic == NI_AdvSimd_SubtractWideningUpper))
                {
                    assert(varTypeIsSIMD(op1->TypeGet()));
                    retNode->AsHWIntrinsic()->SetOtherBaseType(getBaseTypeOfSIMDType(argClass));
                }
#endif
                break;
            }

            case 3:
            {
                CORINFO_ARG_LIST_HANDLE arg2 = info.compCompHnd->getArgNext(argList);
                CORINFO_ARG_LIST_HANDLE arg3 = info.compCompHnd->getArgNext(arg2);

                argType      = JITtype2varType(strip(info.compCompHnd->getArgType(sig, arg3, &argClass)));
                GenTree* op3 = getArgForHWIntrinsic(argType, argClass);

                argType = JITtype2varType(strip(info.compCompHnd->getArgType(sig, arg2, &argClass)));
                op2     = getArgForHWIntrinsic(argType, argClass);

                CORINFO_CLASS_HANDLE op2ArgClass = argClass;

                argType = JITtype2varType(strip(info.compCompHnd->getArgType(sig, argList, &argClass)));
                op1     = getArgForHWIntrinsic(argType, argClass);

#ifdef TARGET_ARM64
                if (intrinsic == NI_AdvSimd_Insert)
                {
                    op2 = addRangeCheckIfNeeded(intrinsic, op2, mustExpand, immLowerBound, immUpperBound);
                }
                else
#endif
                {
                    op3 = addRangeCheckIfNeeded(intrinsic, op3, mustExpand, immLowerBound, immUpperBound);
                }

                retNode = isScalar ? gtNewScalarHWIntrinsicNode(retType, op1, op2, op3, intrinsic)
                                   : gtNewSimdHWIntrinsicNode(retType, op1, op2, op3, intrinsic, baseType, simdSize);

#ifdef TARGET_XARCH
                if (intrinsic == NI_AVX2_GatherVector128 || intrinsic == NI_AVX2_GatherVector256)
                {
                    assert(varTypeIsSIMD(op2->TypeGet()));
                    retNode->AsHWIntrinsic()->SetOtherBaseType(getBaseTypeOfSIMDType(op2ArgClass));
                }
#endif
                break;
            }

            default:
                return nullptr;
        }

        bool isMemoryStore = retNode->OperIsMemoryStore();
        if (isMemoryStore || retNode->OperIsMemoryLoad())
        {
            if (isMemoryStore)
            {
                // A MemoryStore operation is an assignment
                retNode->gtFlags |= GTF_ASG;
            }

            // This operation contains an implicit indirection
            //   it could point into the gloabal heap or
            //   it could throw a null reference exception.
            //
            retNode->gtFlags |= (GTF_GLOB_REF | GTF_EXCEPT);
        }
        return retNode;
    }

    return impSpecialIntrinsic(intrinsic, clsHnd, method, sig, baseType, retType, simdSize);
}

#endif // FEATURE_HW_INTRINSICS
