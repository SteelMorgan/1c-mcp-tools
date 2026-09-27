//++agent TASK-221 [23.09.2026 18:42:07]
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "lineage_resolver.h"
#include "parser.h"
#include "schema_enricher.h"

namespace {

bool HasOnlySource(const nlohmann::json& result, const std::string& column,
                   const std::string& expected) {
    for (const auto& item : result.at("columns")) {
        if (item.at("name") == column) {
            return item.contains("sources")
                && item.at("sources") == nlohmann::json::array({expected});
        }
    }
    return false;
}

bool Check(bool condition, const std::string& case_name) {
    if (!condition) std::cerr << "FAILED: " << case_name << '\n';
    return condition;
}

}  // namespace

int main() {
    const std::string source = "Справочник.big_MarketAccounts";
    const std::string query =
        "ВЫБРАТЬ Ссылка КАК Ref, Наименование КАК ФИО "
        "ИЗ Справочник.big_MarketAccounts";
    const auto batch = lineage::ParseQueryBatch(query);
    const auto resolved = lineage::ResolveBatchLineage(batch);
    bool ok = true;
    ok &= Check(resolved.at("Ref") == lineage::LineageSet{source + ".Ссылка"},
                "bare Ссылка КАК Ref resolves as field");
    ok &= Check(resolved.at("ФИО") == lineage::LineageSet{source + ".Наименование"},
                "ordinary field alias remains resolved");

    const auto schema = nlohmann::json::parse(lineage::AnalyzeSourcesImpl(
        query, R"({"columns":[{"name":"Ref","types":["СправочникСсылка.big_MarketAccounts"]},{"name":"ФИО","types":["Строка"]}]})"));
    ok &= Check(HasOnlySource(schema, "Ref", source + ".Ссылка"),
                "schema enrichment includes reference source");
    ok &= Check(HasOnlySource(schema, "ФИО", source + ".Наименование"),
                "schema enrichment includes ordinary source");

    const auto unaliased = lineage::ResolveBatchLineage(lineage::ParseQueryBatch(
        "ВЫБРАТЬ Ссылка ИЗ Справочник.big_MarketAccounts"));
    ok &= Check(unaliased.at("Ссылка") == lineage::LineageSet{source + ".Ссылка"},
                "bare Ссылка before ИЗ is a field");

    const auto operator_batch = lineage::ParseQueryBatch(
        "ВЫБРАТЬ Поле ССЫЛКА Справочник.X КАК Совпадает "
        "ИЗ РегистрСведений.Y");
    const auto& expression = operator_batch.statements.at(0).select_stmt->select_items.at(0).expr;
    ok &= Check(expression->children.size() == 2
                && expression->children.at(0)->kind == lineage::ExprNode::Kind::FieldRef
                && expression->children.at(1)->kind == lineage::ExprNode::Kind::Literal
                && expression->children.at(1)->text == "ССЫЛКА Справочник.X",
                "ССЫЛКА followed by metadata type retains operator/type parse");
    const auto operator_lineage = lineage::ResolveBatchLineage(operator_batch);
    ok &= Check(operator_lineage.at("Совпадает") ==
                lineage::LineageSet{"РегистрСведений.Y.Поле"},
                "operator type is not mistaken for a field source");

    return ok ? 0 : 1;
}
//++agent TASK-221
