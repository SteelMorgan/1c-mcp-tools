#define PCRE2_CODE_UNIT_WIDTH 16
#include "component.h"

#include <cstring>
#include <algorithm>
#include <sstream>

#ifdef _WINDOWS
#include <windows.h>
#endif

#ifndef _WINDOWS
// Invariant: all std::wstring values inside this component contain UTF-16 code units
// (0x0000-0xFFFF) stored in Linux's 32-bit wchar_t. This helper converts to
// uint16_t array that PCRE2-16 expects. Truncation to 16 bits is intentional.
static std::vector<uint16_t> ToU16(const std::wstring& s) {
    std::vector<uint16_t> v;
    v.reserve(s.size() + 1);
    for (wchar_t c : s)
        v.push_back(static_cast<uint16_t>(c));
    v.push_back(0); // null terminator
    return v;
}
#endif

namespace regex_helper {

// ============================================================================
// Name tables
// ============================================================================

const wchar_t* RegexHelperComponent::method_names_en_[] = {
    L"FindMatchesInTexts",
    L"ValidatePattern",
    L"Version",
};

const wchar_t* RegexHelperComponent::method_names_ru_[] = {
    L"НайтиСовпаденияВТекстах",
    L"ПроверитьШаблон",
    L"Версия",
};

// ============================================================================
// Destructor — free cached patterns
// ============================================================================

RegexHelperComponent::~RegexHelperComponent() {
    for (auto& kv : pattern_cache_) {
        if (kv.second) {
            pcre2_code_free_16(kv.second);
        }
    }
    pattern_cache_.clear();
}

// ============================================================================
// IInitDoneBase
// ============================================================================

bool RegexHelperComponent::Init(void* disp) {
    addin_base_ = static_cast<IAddInDefBase*>(disp);
    return addin_base_ != nullptr;
}

bool RegexHelperComponent::setMemManager(void* mem) {
    mem_manager_ = static_cast<IMemoryManager*>(mem);
    return mem_manager_ != nullptr;
}

void RegexHelperComponent::Done() {
    addin_base_ = nullptr;
    mem_manager_ = nullptr;
}

// ============================================================================
// RegisterExtensionAs
// ============================================================================

bool RegexHelperComponent::RegisterExtensionAs(WCHAR_T** wsExtensionName) {
    return AllocWCHAR_T(wsExtensionName, L"RegexHelper");
}

// ============================================================================
// Methods
// ============================================================================

long RegexHelperComponent::GetNMethods() {
    return eMethodLast;
}

long RegexHelperComponent::FindMethod(const WCHAR_T* wsMethodName) {
    std::wstring name = FromWCHAR_T(wsMethodName);
    auto lower = [](std::wstring s) {
        std::transform(s.begin(), s.end(), s.begin(), ::towlower);
        return s;
    };
    std::wstring lower_name = lower(name);
    for (long i = 0; i < eMethodLast; ++i) {
        if (lower(method_names_en_[i]) == lower_name ||
            lower(method_names_ru_[i]) == lower_name) {
            return i;
        }
    }
    return -1;
}

const WCHAR_T* RegexHelperComponent::GetMethodName(const long lMethodNum,
                                                    const long lMethodAlias) {
    if (lMethodNum < 0 || lMethodNum >= eMethodLast) return nullptr;
    const wchar_t* name = (lMethodAlias == 0) ? method_names_en_[lMethodNum]
                                               : method_names_ru_[lMethodNum];
    WCHAR_T* result = nullptr;
    if (AllocWCHAR_T(&result, std::wstring(name))) return result;
    return nullptr;
}

long RegexHelperComponent::GetNParams(const long lMethodNum) {
    switch (lMethodNum) {
        case eMethodFindMatchesInTexts: return 2;
        case eMethodValidatePattern:   return 1;
        case eMethodVersion:           return 0;
        default:                       return 0;
    }
}

bool RegexHelperComponent::GetParamDefValue(const long /*lMethodNum*/,
                                             const long /*lParamNum*/,
                                             tVariant* /*pvarParamDefValue*/) {
    return false;
}

bool RegexHelperComponent::HasRetVal(const long lMethodNum) {
    return lMethodNum == eMethodFindMatchesInTexts ||
           lMethodNum == eMethodValidatePattern ||
           lMethodNum == eMethodVersion;
}

bool RegexHelperComponent::CallAsProc(const long /*lMethodNum*/,
                                       tVariant* /*paParams*/,
                                       const long /*lSizeArray*/) {
    return false;
}

bool RegexHelperComponent::CallAsFunc(const long lMethodNum,
                                       tVariant* pvarRetValue,
                                       tVariant* paParams,
                                       const long lSizeArray) {
    if (!pvarRetValue) return false;

    switch (lMethodNum) {
        case eMethodFindMatchesInTexts: {
            if (lSizeArray < 2) return false;
            std::wstring rules_json = GetWStringFromVariant(&paParams[0]);
            std::wstring texts_json = GetWStringFromVariant(&paParams[1]);

            std::vector<Rule> rules = ParseRulesJson(rules_json);
            std::vector<std::wstring> texts = ParseTextsJson(texts_json);

            std::vector<std::vector<Match>> results;
            results.reserve(texts.size());
            for (const auto& text : texts) {
                results.push_back(ProcessText(text, rules));
            }

            return SetWStringToVariant(pvarRetValue, SerializeResults(results));
        }

        case eMethodValidatePattern: {
            if (lSizeArray < 1) return false;
            std::wstring pattern = GetWStringFromVariant(&paParams[0]);

            int error_code = 0;
            PCRE2_SIZE error_offset = 0;
#ifdef _WINDOWS
            auto* vp_pat_ptr = reinterpret_cast<PCRE2_SPTR16>(pattern.c_str());
#else
            auto vp_u16_pat = ToU16(pattern);
            auto* vp_pat_ptr = reinterpret_cast<PCRE2_SPTR16>(vp_u16_pat.data());
#endif
            pcre2_code_16* re = pcre2_compile_16(
                vp_pat_ptr,
                PCRE2_ZERO_TERMINATED,
                PCRE2_UTF | PCRE2_UCP,
                &error_code,
                &error_offset,
                nullptr);

            if (re) {
                pcre2_code_free_16(re);
                return SetWStringToVariant(pvarRetValue, L"");
            } else {
                PCRE2_UCHAR16 buffer[256];
                pcre2_get_error_message_16(error_code,
                    reinterpret_cast<PCRE2_UCHAR16*>(buffer), sizeof(buffer) / sizeof(buffer[0]));
#ifdef _WINDOWS
                std::wstring err_msg(reinterpret_cast<const wchar_t*>(buffer));
#else
                std::wstring err_msg;
                for (size_t i = 0; i < sizeof(buffer)/sizeof(buffer[0]) && buffer[i]; ++i)
                    err_msg += static_cast<wchar_t>(buffer[i]);
#endif
                err_msg += L" (at offset " + std::to_wstring(static_cast<int>(error_offset)) + L")";
                return SetWStringToVariant(pvarRetValue, err_msg);
            }
        }

        case eMethodVersion: {
            return SetWStringToVariant(pvarRetValue, L"1.0.0");
        }

        default:
            return false;
    }
}

// ============================================================================
// Core logic
// ============================================================================

pcre2_code_16* RegexHelperComponent::GetOrCompile(const std::wstring& pattern) {
    auto it = pattern_cache_.find(pattern);
    if (it != pattern_cache_.end()) {
        return it->second; // may be nullptr if previously failed
    }

    int error_code = 0;
    PCRE2_SIZE error_offset = 0;
#ifdef _WINDOWS
    auto* gc_pat_ptr = reinterpret_cast<PCRE2_SPTR16>(pattern.c_str());
#else
    auto gc_u16_pat = ToU16(pattern);
    auto* gc_pat_ptr = reinterpret_cast<PCRE2_SPTR16>(gc_u16_pat.data());
#endif
    pcre2_code_16* re = pcre2_compile_16(
        gc_pat_ptr,
        PCRE2_ZERO_TERMINATED,
        PCRE2_UTF | PCRE2_UCP,
        &error_code,
        &error_offset,
        nullptr);

    pattern_cache_[pattern] = re; // store nullptr on failure
    return re;
}

std::vector<RegexHelperComponent::Match> RegexHelperComponent::ProcessText(
        const std::wstring& text, const std::vector<Rule>& rules) {

    // protected_ranges: list of [start, end) — initially scan for existing tokens
    std::vector<std::pair<int,int>> protected_ranges;

#ifdef _WINDOWS
    auto* text_ptr = reinterpret_cast<PCRE2_SPTR16>(text.c_str());
#else
    auto text_u16 = ToU16(text);
    auto* text_ptr = reinterpret_cast<PCRE2_SPTR16>(text_u16.data());
#endif

    // Find existing tokens matching \[[A-Z]+-\d+\] using pcre2
    {
        static const wchar_t* kTokenPattern = L"\\[[A-Z]+-\\d+\\]";
        pcre2_code_16* token_re = GetOrCompile(kTokenPattern);
        if (token_re) {
            pcre2_match_data_16* md = pcre2_match_data_create_from_pattern_16(token_re, nullptr);
            if (md) {
                PCRE2_SIZE offset = 0;
                size_t text_len = text.size();
                while (offset <= text_len) {
                    int rc = pcre2_match_16(token_re,
                        text_ptr,
                        text_len, offset, 0, md, nullptr);
                    if (rc < 0) break;
                    PCRE2_SIZE* ov = pcre2_get_ovector_pointer_16(md);
                    int mstart = static_cast<int>(ov[0]);
                    int mend   = static_cast<int>(ov[1]);
                    protected_ranges.emplace_back(mstart, mend);
                    if (mend > (int)offset) {
                        offset = static_cast<PCRE2_SIZE>(mend);
                    } else {
                        offset++;
                    }
                }
                pcre2_match_data_free_16(md);
            }
        }
    }

    std::vector<Match> accepted;

    for (const auto& rule : rules) {
        if (rule.pattern.empty()) continue;

        pcre2_code_16* re = GetOrCompile(rule.pattern);
        if (!re) continue; // invalid pattern — skip

        pcre2_match_data_16* md = pcre2_match_data_create_from_pattern_16(re, nullptr);
        if (!md) continue;

        PCRE2_SIZE offset = 0;
        size_t text_len = text.size();

        while (offset <= text_len) {
            int rc = pcre2_match_16(re,
                text_ptr,
                text_len, offset, 0, md, nullptr);

            if (rc < 0) break;

            PCRE2_SIZE* ov = pcre2_get_ovector_pointer_16(md);
            int mstart = static_cast<int>(ov[0]);
            int mend   = static_cast<int>(ov[1]);
            int mlen   = mend - mstart;

            // Advance past zero-length match
            PCRE2_SIZE next_offset = (mlen == 0) ? offset + 1
                                                 : static_cast<PCRE2_SIZE>(mend);

            // Check if [mstart, mend) overlaps any protected range
            bool overlaps = false;
            for (const auto& pr : protected_ranges) {
                if (mstart < pr.second && mend > pr.first) {
                    overlaps = true;
                    break;
                }
            }

            if (!overlaps) {
                Match m;
                m.match_text = text.substr(static_cast<size_t>(mstart),
                                           static_cast<size_t>(mlen));
                m.start    = mstart;
                m.length   = mlen;
                m.category = rule.category;
                accepted.push_back(m);
                protected_ranges.emplace_back(mstart, mend);
            }

            offset = next_offset;
        }

        pcre2_match_data_free_16(md);
    }

    // Sort by start DESC (for right-to-left replacement in 1C)
    std::sort(accepted.begin(), accepted.end(),
        [](const Match& a, const Match& b) { return a.start > b.start; });

    return accepted;
}

// ============================================================================
// JSON parsing (hand-written, no dependencies)
// ============================================================================

bool RegexHelperComponent::SkipWhitespace(const std::wstring& json, size_t& pos) {
    while (pos < json.size() && (json[pos] == L' ' || json[pos] == L'\t' ||
           json[pos] == L'\r' || json[pos] == L'\n')) {
        pos++;
    }
    return pos < json.size();
}

bool RegexHelperComponent::ParseJsonString(const std::wstring& json, size_t& pos,
                                            std::wstring& out) {
    if (pos >= json.size() || json[pos] != L'"') return false;
    pos++; // skip opening "
    out.clear();
    while (pos < json.size()) {
        wchar_t ch = json[pos++];
        if (ch == L'"') return true; // done
        if (ch == L'\\' && pos < json.size()) {
            wchar_t esc = json[pos++];
            switch (esc) {
                case L'"':  out += L'"';  break;
                case L'\\': out += L'\\'; break;
                case L'/':  out += L'/';  break;
                case L'n':  out += L'\n'; break;
                case L'r':  out += L'\r'; break;
                case L't':  out += L'\t'; break;
                case L'b':  out += L'\b'; break;
                case L'f':  out += L'\f'; break;
                case L'u': {
                    if (pos + 4 <= json.size()) {
                        std::wstring hex = json.substr(pos, 4);
                        pos += 4;
                        wchar_t uchar = static_cast<wchar_t>(std::stoul(
                            std::string(hex.begin(), hex.end()), nullptr, 16));
                        out += uchar;
                    }
                    break;
                }
                default: out += esc; break;
            }
        } else {
            out += ch;
        }
    }
    return false; // unterminated
}

std::vector<RegexHelperComponent::Rule> RegexHelperComponent::ParseRulesJson(
        const std::wstring& json) {
    std::vector<Rule> rules;
    size_t pos = 0;
    if (!SkipWhitespace(json, pos) || json[pos] != L'[') return rules;
    pos++; // skip [

    while (pos < json.size()) {
        SkipWhitespace(json, pos);
        if (pos >= json.size()) break;
        if (json[pos] == L']') break;
        if (json[pos] == L',') { pos++; continue; }
        if (json[pos] != L'{') { pos++; continue; }
        pos++; // skip {

        Rule rule;
        while (pos < json.size()) {
            SkipWhitespace(json, pos);
            if (pos >= json.size()) break;
            if (json[pos] == L'}') { pos++; break; }
            if (json[pos] == L',') { pos++; continue; }
            if (json[pos] != L'"') { pos++; continue; }

            std::wstring key;
            if (!ParseJsonString(json, pos, key)) break;
            SkipWhitespace(json, pos);
            if (pos >= json.size() || json[pos] != L':') break;
            pos++; // skip :
            SkipWhitespace(json, pos);
            if (pos >= json.size()) break;

            if (json[pos] == L'"') {
                std::wstring val;
                ParseJsonString(json, pos, val);
                if (key == L"pattern") {
                    rule.pattern = val;
                } else if (key == L"category") {
                    rule.category = val;
                }
            } else {
                // skip non-string value
                while (pos < json.size() && json[pos] != L',' && json[pos] != L'}') pos++;
            }
        }
        if (!rule.pattern.empty()) {
            rules.push_back(rule);
        }
    }
    return rules;
}

std::vector<std::wstring> RegexHelperComponent::ParseTextsJson(const std::wstring& json) {
    std::vector<std::wstring> texts;
    size_t pos = 0;
    if (!SkipWhitespace(json, pos) || json[pos] != L'[') return texts;
    pos++; // skip [

    while (pos < json.size()) {
        SkipWhitespace(json, pos);
        if (pos >= json.size()) break;
        if (json[pos] == L']') break;
        if (json[pos] == L',') { pos++; continue; }
        if (json[pos] == L'"') {
            std::wstring val;
            ParseJsonString(json, pos, val);
            texts.push_back(val);
        } else if (json[pos] == L'n') {
            // null
            texts.push_back(L"");
            while (pos < json.size() && json[pos] != L',' && json[pos] != L']') pos++;
        } else {
            pos++;
        }
    }
    return texts;
}

// ============================================================================
// JSON serialization
// ============================================================================

std::wstring RegexHelperComponent::JsonEscape(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size() + 8);
    for (wchar_t ch : s) {
        switch (ch) {
            case L'"':  out += L"\\\""; break;
            case L'\\': out += L"\\\\"; break;
            case L'\n': out += L"\\n";  break;
            case L'\r': out += L"\\r";  break;
            case L'\t': out += L"\\t";  break;
            default:    out += ch;      break;
        }
    }
    return out;
}

std::wstring RegexHelperComponent::SerializeResults(
        const std::vector<std::vector<Match>>& results) {
    std::wstring out;
    out.reserve(256);
    out += L'[';
    for (size_t i = 0; i < results.size(); ++i) {
        if (i > 0) out += L',';
        out += L'[';
        const auto& matches = results[i];
        for (size_t j = 0; j < matches.size(); ++j) {
            if (j > 0) out += L',';
            const auto& m = matches[j];
            out += L"{\"match\":\"";
            out += JsonEscape(m.match_text);
            out += L"\",\"start\":";
            out += std::to_wstring(m.start);
            out += L",\"length\":";
            out += std::to_wstring(m.length);
            out += L",\"category\":\"";
            out += JsonEscape(m.category);
            out += L"\"}";
        }
        out += L']';
    }
    out += L']';
    return out;
}

// ============================================================================
// Helpers
// ============================================================================

bool RegexHelperComponent::SetWStringToVariant(tVariant* var, const std::wstring& str) {
    if (!var || !mem_manager_) return false;

    TV_VT(var) = VTYPE_PWSTR;
    size_t byte_count = (str.size() + 1) * sizeof(WCHAR_T);
    if (!mem_manager_->AllocMemory(reinterpret_cast<void**>(&var->pwstrVal),
                                    static_cast<unsigned long>(byte_count))) {
        return false;
    }

#ifdef _WINDOWS
    memcpy(var->pwstrVal, str.c_str(), byte_count);
#else
    for (size_t i = 0; i <= str.size(); ++i)
        var->pwstrVal[i] = static_cast<WCHAR_T>(str[i]);
#endif

    var->wstrLen = static_cast<uint32_t>(str.size());
    return true;
}

std::wstring RegexHelperComponent::GetWStringFromVariant(const tVariant* var) {
    if (!var) return L"";
    if (TV_VT(var) == VTYPE_PWSTR && var->pwstrVal) {
#ifdef _WINDOWS
        return std::wstring(var->pwstrVal, var->wstrLen);
#else
        std::wstring result;
        result.reserve(var->wstrLen);
        for (uint32_t i = 0; i < var->wstrLen; ++i)
            result += static_cast<wchar_t>(var->pwstrVal[i]);
        return result;
#endif
    }
    return L"";
}

std::wstring RegexHelperComponent::FromWCHAR_T(const WCHAR_T* src, size_t len) {
    if (!src) return L"";
#ifdef _WINDOWS
    if (len > 0) return std::wstring(src, len);
    return std::wstring(src);
#else
    std::wstring result;
    if (len > 0) {
        result.reserve(len);
        for (size_t i = 0; i < len; ++i)
            result += static_cast<wchar_t>(src[i]);
    } else {
        while (*src) {
            result += static_cast<wchar_t>(*src);
            ++src;
        }
    }
    return result;
#endif
}

bool RegexHelperComponent::AllocWCHAR_T(WCHAR_T** dest, const std::wstring& src) {
    if (!dest || !mem_manager_) return false;
    size_t byte_count = (src.size() + 1) * sizeof(WCHAR_T);
    if (!mem_manager_->AllocMemory(reinterpret_cast<void**>(dest),
                                    static_cast<unsigned long>(byte_count))) {
        return false;
    }
#ifdef _WINDOWS
    memcpy(*dest, src.c_str(), byte_count);
#else
    for (size_t i = 0; i <= src.size(); ++i)
        (*dest)[i] = static_cast<WCHAR_T>(src[i]);
#endif
    return true;
}

} // namespace regex_helper
