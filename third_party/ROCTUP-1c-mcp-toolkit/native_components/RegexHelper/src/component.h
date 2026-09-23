#pragma once

#define PCRE2_CODE_UNIT_WIDTH 16
#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

#include <pcre2.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace regex_helper {

class RegexHelperComponent : public IComponentBase {
public:
    RegexHelperComponent() = default;
    ~RegexHelperComponent() override;

    // IInitDoneBase
    bool ADDIN_API Init(void* disp) override;
    bool ADDIN_API setMemManager(void* mem) override;
    long ADDIN_API GetInfo() override { return 2000; }
    void ADDIN_API Done() override;

    // ILanguageExtenderBase
    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) override;

    long ADDIN_API GetNProps() override { return 0; }
    long ADDIN_API FindProp(const WCHAR_T* /*wsPropName*/) override { return -1; }
    const WCHAR_T* ADDIN_API GetPropName(long /*lPropNum*/, long /*lPropAlias*/) override { return nullptr; }
    bool ADDIN_API GetPropVal(const long /*lPropNum*/, tVariant* /*pvarPropVal*/) override { return false; }
    bool ADDIN_API SetPropVal(const long /*lPropNum*/, tVariant* /*varPropVal*/) override { return false; }
    bool ADDIN_API IsPropReadable(const long /*lPropNum*/) override { return false; }
    bool ADDIN_API IsPropWritable(const long /*lPropNum*/) override { return false; }

    long ADDIN_API GetNMethods() override;
    long ADDIN_API FindMethod(const WCHAR_T* wsMethodName) override;
    const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum, const long lMethodAlias) override;
    long ADDIN_API GetNParams(const long lMethodNum) override;
    bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
                                     tVariant* pvarParamDefValue) override;
    bool ADDIN_API HasRetVal(const long lMethodNum) override;
    bool ADDIN_API CallAsProc(const long lMethodNum, tVariant* paParams,
                               const long lSizeArray) override;
    bool ADDIN_API CallAsFunc(const long lMethodNum, tVariant* pvarRetValue,
                               tVariant* paParams, const long lSizeArray) override;

    // LocaleBase
    void ADDIN_API SetLocale(const WCHAR_T* /*loc*/) override {}

private:
    enum Methods {
        eMethodFindMatchesInTexts = 0,
        eMethodValidatePattern,
        eMethodVersion,
        eMethodLast
    };

    struct Rule {
        std::wstring pattern;
        std::wstring category;
    };

    struct Match {
        std::wstring match_text;
        int start;   // 0-based code unit index
        int length;  // in code units
        std::wstring category;
    };

    // Parse JSON array of rules: [{"pattern":"...","category":"..."},...]
    std::vector<Rule> ParseRulesJson(const std::wstring& json);

    // Parse JSON array of strings: ["text1","text2",...]
    std::vector<std::wstring> ParseTextsJson(const std::wstring& json);

    // Process one text with given rules, respecting protected_ranges
    std::vector<Match> ProcessText(const std::wstring& text, const std::vector<Rule>& rules);

    // Get or compile regex (cached); returns nullptr on compile error
    pcre2_code_16* GetOrCompile(const std::wstring& pattern);

    // Serialize result to JSON:
    // [[{"match":"...","start":N,"length":N,"category":"..."},...],...]
    std::wstring SerializeResults(const std::vector<std::vector<Match>>& results);

    // JSON helpers
    std::wstring JsonEscape(const std::wstring& s);
    bool ParseJsonString(const std::wstring& json, size_t& pos, std::wstring& out);
    bool SkipWhitespace(const std::wstring& json, size_t& pos);

    // 1C variant helpers
    bool SetWStringToVariant(tVariant* var, const std::wstring& str);
    std::wstring GetWStringFromVariant(const tVariant* var);
    std::wstring FromWCHAR_T(const WCHAR_T* src, size_t len = 0);
    bool AllocWCHAR_T(WCHAR_T** dest, const std::wstring& src);

    IAddInDefBase* addin_base_ = nullptr;
    IMemoryManager* mem_manager_ = nullptr;

    // Pattern cache: wstring → pcre2_code_16*
    std::unordered_map<std::wstring, pcre2_code_16*> pattern_cache_;

    static const wchar_t* method_names_en_[];
    static const wchar_t* method_names_ru_[];
};

} // namespace regex_helper
