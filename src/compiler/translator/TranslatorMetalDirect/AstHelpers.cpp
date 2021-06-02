//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cstring>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/TranslatorMetalDirect/AstHelpers.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

Declaration sh::ViewDeclaration(TIntermDeclaration &declNode)
{
    ASSERT(declNode.getChildCount() == 1);
    TIntermNode *childNode = declNode.getChildNode(0);
    ASSERT(childNode);
    TIntermSymbol *symbolNode;
    if ((symbolNode = childNode->getAsSymbolNode()))
    {
        return {*symbolNode, nullptr};
    }
    else
    {
        TIntermBinary *initNode = childNode->getAsBinaryNode();
        ASSERT(initNode);
        ASSERT(initNode->getOp() == TOperator::EOpInitialize);
        symbolNode = initNode->getLeft()->getAsSymbolNode();
        ASSERT(symbolNode);
        return {*symbolNode, initNode->getRight()};
    }
}

const TVariable &sh::CreateStructTypeVariable(TSymbolTable &symbolTable,
                                              const TStructure &structure)
{
    auto *type = new TType(&structure, true);
    auto *var  = new TVariable(&symbolTable, ImmutableString(""), type, SymbolType::Empty);
    return *var;
}

const TVariable &sh::CreateInstanceVariable(TSymbolTable &symbolTable,
                                            const TStructure &structure,
                                            const Name &name,
                                            TQualifier qualifier,
                                            const TSpan<const unsigned int> *arraySizes)
{
    auto *type = new TType(&structure, false);
    type->setQualifier(qualifier);
    if (arraySizes)
    {
        type->makeArrays(*arraySizes);
    }
    auto *var = new TVariable(&symbolTable, name.rawName(), type, name.symbolType());
    return *var;
}

static void AcquireFunctionExtras(TFunction &dest, const TFunction &src)
{
    if (src.isDefined())
    {
        dest.setDefined();
    }

    if (src.hasPrototypeDeclaration())
    {
        dest.setHasPrototypeDeclaration();
    }
}

TIntermSequence &sh::CloneSequenceAndPrepend(const TIntermSequence &seq, TIntermNode &node)
{
    auto *newSeq = new TIntermSequence();
    newSeq->push_back(&node);

    for (TIntermNode *oldNode : seq)
    {
        newSeq->push_back(oldNode);
    }

    return *newSeq;
}

void sh::AddParametersFrom(TFunction &dest, const TFunction &src)
{
    const size_t paramCount = src.getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        const TVariable *var = src.getParam(i);
        dest.addParameter(var);
    }
}

const TFunction &sh::CloneFunction(TSymbolTable &symbolTable,
                                   IdGen &idGen,
                                   const TFunction &oldFunc)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined);

    Name newName = idGen.createNewName(Name(oldFunc));

    auto &newFunc = *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                                   &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

const TFunction &sh::CloneFunctionAndPrependParam(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const TVariable &newParam)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined ||
           oldFunc.symbolType() == SymbolType::AngleInternal);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    auto &newFunc = *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                                   &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    newFunc.addParameter(&newParam);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

const TFunction &sh::CloneFunctionAndAppendParams(TSymbolTable &symbolTable,
                                                  IdGen *idGen,
                                                  const TFunction &oldFunc,
                                                  const std::vector<const TVariable *> &newParams)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined ||
           oldFunc.symbolType() == SymbolType::AngleInternal);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    auto &newFunc = *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                                   &oldFunc.getReturnType(), oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);
    for (auto *param : newParams)
    {
        newFunc.addParameter(param);
    }

    return newFunc;
}

const TFunction &sh::CloneFunctionAndChangeReturnType(TSymbolTable &symbolTable,
                                                      IdGen *idGen,
                                                      const TFunction &oldFunc,
                                                      const TStructure &newReturn)
{
    ASSERT(oldFunc.symbolType() == SymbolType::UserDefined);

    Name newName = idGen ? idGen->createNewName(Name(oldFunc)) : Name(oldFunc);

    auto *newReturnType = new TType(&newReturn, true);
    auto &newFunc       = *new TFunction(&symbolTable, newName.rawName(), newName.symbolType(),
                                   newReturnType, oldFunc.isKnownToNotHaveSideEffects());

    AcquireFunctionExtras(newFunc, oldFunc);
    AddParametersFrom(newFunc, oldFunc);

    return newFunc;
}

TIntermTyped &sh::GetArg(const TIntermAggregate &call, size_t index)
{
    ASSERT(index < call.getChildCount());
    auto *arg = call.getChildNode(index);
    ASSERT(arg);
    auto *targ = arg->getAsTyped();
    ASSERT(targ);
    return *targ;
}

void sh::SetArg(TIntermAggregate &call, size_t index, TIntermTyped &arg)
{
    ASSERT(index < call.getChildCount());
    (*call.getSequence())[index] = &arg;
}

int sh::GetFieldIndex(const TStructure &structure, const ImmutableString &fieldName)
{
    const TFieldList &fieldList = structure.fields();

    int i = 0;
    for (TField *field : fieldList)
    {
        if (field->name() == fieldName)
        {
            return i;
        }
        ++i;
    }

    return -1;
}

TIntermBinary &sh::AccessField(const TVariable &structInstanceVar, const ImmutableString &fieldName)
{
    return AccessField(*new TIntermSymbol(&structInstanceVar), fieldName);
}

TIntermBinary &sh::AccessField(TIntermTyped &object, const ImmutableString &fieldName)
{
    const TStructure *structure = object.getType().getStruct();
    ASSERT(structure);

    const int index = GetFieldIndex(*structure, fieldName);
    ASSERT(index >= 0);
    return AccessFieldByIndex(object, index);
}

TIntermBinary &sh::AccessFieldByIndex(TIntermTyped &object, int index)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const TType &type = object.getType();
    ASSERT(!type.isArray());
    const TStructure *structure = type.getStruct();
    ASSERT(structure);
    ASSERT(0 <= index);
    ASSERT(static_cast<size_t>(index) < structure->fields().size());
#endif

    return *new TIntermBinary(
        TOperator::EOpIndexDirectStruct, &object,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
}

TIntermBinary &sh::AccessIndex(TIntermTyped &indexableNode, int index)
{
#if defined(ANGLE_ENABLE_ASSERTS)
    const TType &type = indexableNode.getType();
    ASSERT(type.isArray() || type.isVector() || type.isMatrix());
#endif

    auto *accessNode = new TIntermBinary(
        TOperator::EOpIndexDirect, &indexableNode,
        new TIntermConstantUnion(new TConstantUnion(index), *new TType(TBasicType::EbtInt)));
    return *accessNode;
}

TIntermTyped &sh::AccessIndex(TIntermTyped &node, const int *index)
{
    if (index)
    {
        return AccessIndex(node, *index);
    }
    return node;
}

TIntermTyped &sh::SubVector(TIntermTyped &vectorNode, int begin, int end)
{
    ASSERT(vectorNode.getType().isVector());
    ASSERT(0 <= begin);
    ASSERT(end <= 4);
    ASSERT(begin <= end);
    if (begin == 0 && end == vectorNode.getType().getNominalSize())
    {
        return vectorNode;
    }
    TVector<int> offsets(static_cast<size_t>(end - begin));
    std::iota(offsets.begin(), offsets.end(), begin);
    auto *swizzle = new TIntermSwizzle(&vectorNode, offsets);
    return *swizzle;
}

bool sh::IsScalarBasicType(const TType &type)
{
    if (!type.isScalar())
    {
        return false;
    }
    return HasScalarBasicType(type);
}

bool sh::IsVectorBasicType(const TType &type)
{
    if (!type.isVector())
    {
        return false;
    }
    return HasScalarBasicType(type);
}

bool sh::HasScalarBasicType(TBasicType type)
{
    switch (type)
    {
        case TBasicType::EbtFloat:
        case TBasicType::EbtDouble:
        case TBasicType::EbtInt:
        case TBasicType::EbtUInt:
        case TBasicType::EbtBool:
            return true;

        default:
            return false;
    }
}

bool sh::HasScalarBasicType(const TType &type)
{
    return HasScalarBasicType(type.getBasicType());
}

static void InitType(TType &type)
{
    if (type.isArray())
    {
        auto sizes = type.getArraySizes();
        type.toArrayBaseType();
        type.makeArrays(sizes);
    }
}

TType &sh::CloneType(const TType &type)
{
    auto &clone = *new TType(type);
    InitType(clone);
    return clone;
}

TType &sh::InnermostType(const TType &type)
{
    auto &inner = *new TType(type);
    inner.toArrayBaseType();
    InitType(inner);
    return inner;
}

TType &sh::DropColumns(const TType &matrixType)
{
    ASSERT(matrixType.isMatrix());
    ASSERT(HasScalarBasicType(matrixType));
    const char *mangledName = nullptr;

    auto &vectorType =
        *new TType(matrixType.getBasicType(), matrixType.getPrecision(), matrixType.getQualifier(),
                   matrixType.getRows(), 1, matrixType.getArraySizes(), mangledName);
    InitType(vectorType);
    return vectorType;
}

TType &sh::DropOuterDimension(const TType &arrayType)
{
    ASSERT(arrayType.isArray());
    const char *mangledName = nullptr;
    const auto &arraySizes  = arrayType.getArraySizes();

    auto &innerType =
        *new TType(arrayType.getBasicType(), arrayType.getPrecision(), arrayType.getQualifier(),
                   arrayType.getNominalSize(), arrayType.getSecondarySize(),
                   arraySizes.subspan(0, arraySizes.size() - 1), mangledName);
    InitType(innerType);
    return innerType;
}

static TType &SetTypeDimsImpl(const TType &type, int primary, int secondary)
{
    ASSERT(1 < primary && primary <= 4);
    ASSERT(1 <= secondary && secondary <= 4);
    ASSERT(HasScalarBasicType(type));
    const char *mangledName = nullptr;

    auto &newType = *new TType(type.getBasicType(), type.getPrecision(), type.getQualifier(),
                               primary, secondary, type.getArraySizes(), mangledName);
    InitType(newType);
    return newType;
}

TType &sh::SetVectorDim(const TType &type, int newDim)
{
    ASSERT(type.isRank0() || type.isVector());
    return SetTypeDimsImpl(type, newDim, 1);
}

TType &sh::SetMatrixRowDim(const TType &matrixType, int newDim)
{
    ASSERT(matrixType.isMatrix());
    ASSERT(1 < newDim && newDim <= 4);
    return SetTypeDimsImpl(matrixType, matrixType.getCols(), newDim);
}

bool sh::HasMatrixField(const TStructure &structure)
{
    for (const TField *field : structure.fields())
    {
        const TType &type = *field->type();
        if (type.isMatrix())
        {
            return true;
        }
    }
    return false;
}

bool sh::HasArrayField(const TStructure &structure)
{
    for (const TField *field : structure.fields())
    {
        const TType &type = *field->type();
        if (type.isArray())
        {
            return true;
        }
    }
    return false;
}

TIntermTyped &sh::CoerceSimple(TBasicType toType, TIntermTyped &fromNode)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(!fromType.isArray());

    if (toType != fromType.getBasicType())
    {
        return *TIntermAggregate::CreateConstructor(
            *new TType(toType, fromType.getNominalSize(), fromType.getSecondarySize()),
            new TIntermSequence{&fromNode});
    }
    return fromNode;
}

TIntermTyped &sh::CoerceSimple(const TType &toType, TIntermTyped &fromNode)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(toType.getNominalSize() == fromType.getNominalSize());
    ASSERT(toType.getSecondarySize() == fromType.getSecondarySize());
    ASSERT(!toType.isArray());
    ASSERT(!fromType.isArray());

    if (toType.getBasicType() != fromType.getBasicType())
    {
        return *TIntermAggregate::CreateConstructor(toType, new TIntermSequence{&fromNode});
    }
    return fromNode;
}

TIntermTyped &sh::AsType(SymbolEnv &symbolEnv, const TType &toType, TIntermTyped &fromNode)
{
    const TType &fromType = fromNode.getType();

    ASSERT(HasScalarBasicType(toType));
    ASSERT(HasScalarBasicType(fromType));
    ASSERT(!toType.isArray());
    ASSERT(!fromType.isArray());

    if (toType == fromType)
    {
        return fromNode;
    }
    TemplateArg targ(toType);
    return symbolEnv.callFunctionOverload(Name("as_type", SymbolType::BuiltIn), toType,
                                          *new TIntermSequence{&fromNode}, 1, &targ);
}