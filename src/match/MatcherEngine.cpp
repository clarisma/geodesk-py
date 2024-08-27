// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MatcherEngine.h"
#include "OpGraph.h"
#include "feature/Feature.h"
#include "feature/FeatureStore.h"
#include <common/math/Math.h>


// IN: pTag_, ip_; OUT: tagKey_, valueOfs_
int MatcherEngine::scanGlobalKeys()
{
    assert((pTag_.pointerAsULong() & 1) == 0);

    // TODO: decide if operand should be the global-key code or the key-bits (already shifted)
    uint16_t operand = ip_.getUnsignedShort() << 2;
    ip_ += 2;   // move to 2nd operand (jump target)
    for (;;)
    {
        uint16_t key = pTag_.getUnsignedShort();
        int step = 4 + (key & 2);
        if (key >= operand)
        {
            valueOfs_ = step - 2;
            tagKey_ = key;
            int matched = ((key & 0x7ffc) == operand);
            pTag_ += step * matched;
            return matched;
        }
        pTag_ += step;
    }
}

// IN: pTagTable_, pTag_, ip_; OUT: tagKey_
int MatcherEngine::scanLocalKeys()
{
    pointer pTagTableAligned = pointer::ofTagged(pTagTable_, -4);
    std::string_view operand = getStringOperand();
    pointer pTagOld = pTag_;
    for (;;)
    {
        int32_t key = pTag_.getUnalignedInt();
        pTag_ -= 6 + (key & 2);
        pointer pKey = pTagTableAligned + ((key >> 3) << 2);
        LocalString keyString(pKey);
        if (keyString.equals(operand.data(), operand.length()))
        {
            tagKey_ = key;
            return 1;
        }
        if (key & 4)            // Last-tag flag means we're done, no match    
        {
            pTag_ = pTagOld;    // We rewind the tag pointer to the previous pos
            return 0;  
        }
    }
}

std::string_view MatcherEngine::getStringOperand()
{
    uint16_t opOfs = ip_.getUnsignedShort();
    pointer p = ip_.asBytePointer() - opOfs; // TODO: relative to Matcher*?
    ip_ += 2;
    int len = p.getUnsignedShort();
    return std::string_view(p + 2, len);
}

inline double MatcherEngine::getDoubleOperand()
{
    uint16_t opOfs = ip_.getUnsignedShort();
    double d = *((double*)(ip_.asBytePointer() - opOfs));
    ip_ += 2;
    return d;
}

inline const std::regex *MatcherEngine::getRegexOperand()
{
    uint16_t opOfs = ip_.getUnsignedShort();
    std::regex* regex = (std::regex*)(ip_.asBytePointer() - opOfs); // TODO: relative to Matcher*?
    ip_ += 2;
    return regex;
}

inline uint32_t MatcherEngine::getFeatureTypeOperand()
{
    uint32_t types = ip_.getUnalignedUnsignedInt();
        // This should use native-Endianness, as types are written
        // like that by the emitter (we may make pointer little-Endian)
    ip_ += 4;
    return types;
}

int MatcherEngine::accept(const Matcher* matcher, const uint8_t* pFeature)
{
    MatcherEngine ctx;
    uint32_t codeValue;
    const uint8_t* stringValue;
    double doubleValue;

    // The matcher's bytecode begins right after the Matcher structure
    ctx.ip_ = pointer(reinterpret_cast<const uint8_t*>(matcher) + sizeof(Matcher));
    ctx.pTagTable_ = pointer(pFeature + 8).follow();
    for (;;)
    {
        int matched;
        int op = ctx.ip_.getUnsignedShort();
        int opcode = op & 0xff;
        ctx.ip_ += 2;   // move to first operand
        switch (opcode)
        {
            case NOP:
                // do nothing
                continue;

            case EQ_CODE:
            {
                uint16_t operand = ctx.ip_.getUnsignedShort();
                ctx.ip_ += 2;
                matched = (codeValue == operand);
            }
            break;

            case EQ_STR:
            {
                LocalString str(stringValue);
                matched = str.equals(ctx.getStringOperand());
            }
            break;

            case STARTS_WITH:
                matched = asStringView(stringValue).starts_with(ctx.getStringOperand());
                break;

            case ENDS_WITH:
                matched = asStringView(stringValue).ends_with(ctx.getStringOperand());
                break;

            case CONTAINS:
                matched = asStringView(stringValue).find(ctx.getStringOperand())
                    != std::string_view::npos;
                break;

            case REGEX:
            {
                std::string_view val = asStringView(stringValue);
                matched = std::regex_match(val.begin(), val.end(), *ctx.getRegexOperand());
            }
            break;

            case EQ_NUM:
                matched = (doubleValue == ctx.getDoubleOperand());
                break;

            case LE:
                matched = (doubleValue <= ctx.getDoubleOperand());
                break;

            case LT:
                matched = (doubleValue < ctx.getDoubleOperand());
                break;

            case GE:
                matched = (doubleValue >= ctx.getDoubleOperand());
                break;

            case GT:
                matched = (doubleValue > ctx.getDoubleOperand());
                break;

            case GLOBAL_KEY:
                if (ctx.tagKey_ & 0x8000)
                {
                    // We're at last tag --> match failed 
                    // `matched` already has the correct value
                    ctx.ip_ += 2;       // move to 2nd operand (jump target)
                    matched = 0;
                }
                else
                {
                    matched = ctx.scanGlobalKeys();
                }
                break;

            case FIRST_GLOBAL_KEY:
            {
                pointer p = ctx.pTagTable_;
                ctx.pTag_ = p & (uint64_t)~1;       // mask off the local-keys flag
                matched = ctx.scanGlobalKeys();
            }
            break;

            case LOCAL_KEY:
                if (ctx.tagKey_ & 4)
                {
                    // We're at last tag --> match failed 
                    ctx.ip_ += 2;       // move to 2nd operand (jump target)
                    matched = 0;
                }
                else
                {
                    matched = ctx.scanLocalKeys();
                }
                break;

            case FIRST_LOCAL_KEY:
                assert(reinterpret_cast<uintptr_t>(ctx.pTagTable_) & 1); // must have local-keys flag
                ctx.pTag_ = ctx.pTagTable_ - 5;    // position pTag_ to first key (at -4, but
                                                   // the local_keys flag is set (1), so -5  
                ctx.valueOfs_ = -4;                // remains constant for all local-key tags  
                                                   // is negative because valueOfs_ is *subtracted*
                                                   // from pTag_ (but local key/values are laid out
                                                   // in reverse)       
                matched = ctx.scanLocalKeys();
                break;

            case HAS_LOCAL_KEYS:
                matched = reinterpret_cast<uintptr_t>(ctx.pTagTable_) & 1;    // test local-keys flag
                break;

            case LOAD_CODE:
                codeValue = ctx.pTag_.getUnsignedShort(-ctx.valueOfs_); // TODO: decide on sign
                matched = ((ctx.tagKey_ & 3) == 1);
                assert(!matched || matcher->store()->strings().isValidCode(codeValue));
                break;

            case LOAD_STRING:
            {
                // A local-string value is stored as a 4-byte value (wide string)
                // which represents an offset that, when added to the location
                // of the value, yields a pointer to a local string.
                // We can only read a 4-byte value from the value location if the
                // value is, in fact, a wide value, because the GOL spec does not
                // guarantee that memory can legally be read at (pVal + 2) if the
                // value is narrow. Hence, we read the value as two 16-bit values,
                // reading the upper half at pVal+2 if wide-string, or at pVal+0
                // if any other type. This pointer arithmetic avoids a branch.
                // The resulting pointer may therefore be invalid, but we are only 
                // dereferencing it if the wide-string check succeeds

                pointer pVal = ctx.pTag_ - (int)ctx.valueOfs_;
                matched = ((ctx.tagKey_ & 3) == 3);      // wide string
                int32_t rel = pVal.getUnsignedShort();   // read lower half
                rel |= (pVal + matched * 2).getShort() << 16;
                // read upper half (or lower again if this isn't a wide-string value)
                stringValue = pVal.asBytePointer() + rel;
                // The resulting string pointer (may be invalid, but instructions
                // will only use it if matched==1)
            }
            break;

            case LOAD_NUM:
            {
                int type = (ctx.tagKey_ & 3);
                if (type == 0)      // narrow number
                {
                    pointer pVal = ctx.pTag_ - (int)ctx.valueOfs_;
                    doubleValue = ((int32_t)pVal.getUnsignedShort()) + TagsRef::MIN_NUMBER;
                    matched = 1;
                }
                else if (type == 2) // wide number
                {
                    pointer pVal = ctx.pTag_ - (int)ctx.valueOfs_;
                    doubleValue = TagValue::doubleFromWideNumber(pVal.getUnsignedInt());
                    matched = 1;
                }
                else
                {
                    matched = 0;
                }
            }
            break;

            case CODE_TO_STR:
                stringValue = matcher->store()->strings()
                    .getGlobalString(codeValue).asBytePointer();
                // Cannot fail, therefore does not branch
                continue;

            case STR_TO_NUM:
                matched = Math::parseDouble(asStringView(stringValue), &doubleValue);
                break;

            case FEATURE_TYPE:
            {
                FeatureTypes types(ctx.getFeatureTypeOperand());
                matched = (int)types.acceptFlags(pointer(pFeature).getInt());
            }
            break;

            case GOTO:
                ctx.ip_ += ctx.ip_.getShort();
                continue;

            case RETURN:
                return op >> 8;

            default:
#ifdef _DEBUG
                assert(false);
                break;
#else
    #ifdef __GNUC__  // GCC compiler check
                __builtin_unreachable();
    #elif defined(_MSC_VER)  // MSVC compiler check
                __assume(0);
    #endif
#endif
            // We tell the compiler not to perform bounds checks;
            // if there's an invalid opcode, this means the bytecode
            // is corrupted and the bounds check won't save us
        }
        matched ^= isNegated(op);
        ctx.jumpIf(matched);
    }
}

