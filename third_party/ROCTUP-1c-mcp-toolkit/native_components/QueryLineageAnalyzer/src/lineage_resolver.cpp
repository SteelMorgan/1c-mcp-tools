#include "lineage_resolver.h"

#include <unordered_map>
#include <vector>

namespace lineage {

namespace {

struct SourceContext {
    enum class Kind {
        Metadata,
        TempTable,
        Subquery
    };

    Kind kind = Kind::Metadata;
    std::string source_name;
    OutputLineageMap subquery_output;
};

using ScopeMap = std::unordered_map<std::string, SourceContext>;

void MergeLineage(LineageSet& target, const LineageSet& source) {
    target.insert(source.begin(), source.end());
}

LineageSet ResolveExpr(const std::shared_ptr<ExprNode>& expr, const ScopeMap& scope);

LineageSet ResolveField(const ExprNode& expr, const ScopeMap& scope) {
    LineageSet result;
    if (expr.parts.empty()) return result;

    if (expr.parts.size() >= 2) {
        auto it = scope.find(expr.parts.front());
        if (it != scope.end()) {
            // Ступень 1: явный алиас найден в scope
            const SourceContext& source = it->second;
            if (source.kind == SourceContext::Kind::Subquery ||
                source.kind == SourceContext::Kind::TempTable) {
                // Ищем по parts[1], затем дописываем хвост parts[2..n].
                // Корректно для ВТ.Поле и для ВТ.СчетДт.Представление.
                auto sub = source.subquery_output.find(expr.parts[1]);
                if (sub != source.subquery_output.end()) {
                    LineageSet expanded = sub->second;
                    for (size_t i = 2; i < expr.parts.size(); ++i) {
                        LineageSet next;
                        for (const auto& s : expanded) next.insert(s + "." + expr.parts[i]);
                        expanded = std::move(next);
                    }
                    return expanded;
                }
                return result;
            }
            std::string path = source.source_name;
            for (size_t i = 1; i < expr.parts.size(); ++i) path += "." + expr.parts[i];
            result.insert(path);
            return result;
        }

        // Ступень 2: неявный алиас — parts[0] совпадает с последним сегментом source_name.
        // Пример: "Товары.Номенклатура" из "Документ.X.Товары" без КАК Товары.
        {
            std::vector<LineageSet> implicit_matches;
            for (const auto& pair : scope) {
                const SourceContext& source = pair.second;
                if (source.kind != SourceContext::Kind::Metadata) continue;
                size_t dot = source.source_name.rfind('.');
                std::string last_seg = (dot != std::string::npos)
                    ? source.source_name.substr(dot + 1)
                    : source.source_name;
                if (last_seg != expr.parts.front()) continue;
                std::string path = source.source_name;
                for (size_t i = 1; i < expr.parts.size(); ++i) path += "." + expr.parts[i];
                LineageSet ls;
                ls.insert(path);
                implicit_matches.push_back(std::move(ls));
            }
            if (implicit_matches.size() == 1) return implicit_matches.front();
            if (!implicit_matches.empty()) return result;  // ambiguous implicit alias
        }

        // Ступень 3: поле источника с субатрибутом.
        // Пример: "СчетДт.Представление" из "РегистрБухгалтерии.Хозрасчетный"
        // → "РегистрБухгалтерии.Хозрасчетный.СчетДт.Представление"
        std::string suffix;
        for (size_t i = 1; i < expr.parts.size(); ++i) suffix += "." + expr.parts[i];

        std::vector<LineageSet> matches;
        for (const auto& pair : scope) {
            const SourceContext& source = pair.second;
            if (source.kind == SourceContext::Kind::Subquery ||
                source.kind == SourceContext::Kind::TempTable) {
                auto sub = source.subquery_output.find(expr.parts.front());
                if (sub != source.subquery_output.end()) {
                    LineageSet expanded;
                    for (const auto& s : sub->second) expanded.insert(s + suffix);
                    matches.push_back(std::move(expanded));
                }
            } else {
                LineageSet lineage;
                lineage.insert(source.source_name + "." + expr.parts.front() + suffix);
                matches.push_back(std::move(lineage));
            }
        }
        if (matches.size() == 1) return matches.front();
        return result;  // ambiguous — оставляем пустым
    }

    std::string field_name = expr.parts.front();
    std::vector<LineageSet> matches;
    for (const auto& pair : scope) {
        const SourceContext& source = pair.second;
        if (source.kind == SourceContext::Kind::Subquery || source.kind == SourceContext::Kind::TempTable) {
            auto sub = source.subquery_output.find(field_name);
            if (sub != source.subquery_output.end()) matches.push_back(sub->second);
        } else {
            LineageSet lineage;
            lineage.insert(source.source_name + "." + field_name);
            matches.push_back(std::move(lineage));
        }
    }
    if (matches.size() == 1) return matches.front();
    return result;
}

LineageSet ResolveExpr(const std::shared_ptr<ExprNode>& expr, const ScopeMap& scope) {
    LineageSet result;
    if (!expr) return result;

    switch (expr->kind) {
        case ExprNode::Kind::FieldRef:
            return ResolveField(*expr, scope);
        case ExprNode::Kind::FunctionCall:
        case ExprNode::Kind::CaseExpr:
        case ExprNode::Kind::BinaryOp:
        case ExprNode::Kind::Aggregate:
        case ExprNode::Kind::Composite:
            for (const auto& child : expr->children) MergeLineage(result, ResolveExpr(child, scope));
            return result;
        case ExprNode::Kind::Literal:
        case ExprNode::Kind::Parameter:
        case ExprNode::Kind::Wildcard:
            return result;
    }
    return result;
}

OutputLineageMap ResolveSelect(const SelectStatementNode& stmt, TempTableMap& temp_tables) {
    ScopeMap scope;
    for (const auto& source : stmt.from_sources) {
        SourceContext context;
        std::string alias = source.alias.empty() ? source.source_name : source.alias;
        if (source.kind == SourceNode::Kind::Subquery && source.subquery) {
            TempTableMap nested_temp = temp_tables;
            context.kind = SourceContext::Kind::Subquery;
            context.subquery_output = ResolveSelect(*source.subquery, nested_temp);
        } else if (temp_tables.find(source.source_name) != temp_tables.end()) {
            context.kind = SourceContext::Kind::TempTable;
            context.source_name = source.source_name;
            for (const auto& pair : temp_tables.at(source.source_name)) context.subquery_output[pair.first] = pair.second;
        } else {
            context.kind = SourceContext::Kind::Metadata;
            context.source_name = source.source_name;
        }
        scope[alias] = context;
    }

    OutputLineageMap output;
    std::vector<std::string> ordered_names;
    for (const auto& item : stmt.select_items) {
        if (item.result_name.empty()) continue;
        output[item.result_name] = ResolveExpr(item.expr, scope);
        ordered_names.push_back(item.result_name);
    }

    for (const auto& part : stmt.union_parts) {
        TempTableMap nested_temp = temp_tables;
        OutputLineageMap part_output = ResolveSelect(part, nested_temp);
        size_t index = 0;
        for (const auto& item : part.select_items) {
            if (index >= ordered_names.size()) break;
            auto part_it = part_output.find(item.result_name);
            if (part_it != part_output.end()) MergeLineage(output[ordered_names[index]], part_it->second);
            ++index;
        }
    }

    if (!stmt.into_temp_table.empty()) {
        temp_tables[stmt.into_temp_table].clear();
        for (const auto& pair : output) temp_tables[stmt.into_temp_table][pair.first] = pair.second;
    }

    return output;
}

}  // namespace

OutputLineageMap ResolveBatchLineage(const QueryBatchNode& batch) {
    TempTableMap temp_tables;
    OutputLineageMap last_output;
    for (const auto& statement : batch.statements) {
        if (statement.kind == StatementNode::Kind::Destroy && statement.destroy_stmt) {
            temp_tables.erase(statement.destroy_stmt->table_name);
            continue;
        }
        if (statement.kind == StatementNode::Kind::Select && statement.select_stmt) {
            last_output = ResolveSelect(*statement.select_stmt, temp_tables);
        }
    }
    return last_output;
}

}  // namespace lineage
