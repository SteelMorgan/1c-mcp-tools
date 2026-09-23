#include "schema_enricher.h"

#include "json_utils.h"
#include "lineage_resolver.h"
#include "parser.h"

namespace lineage {

std::string AnalyzeSourcesImpl(const std::string& query_text, const std::string& schema_json) {
    if (query_text.empty() || schema_json.empty()) return schema_json;

    QueryBatchNode batch = ParseQueryBatch(query_text);
    OutputLineageMap output = ResolveBatchLineage(batch);
    if (output.empty()) return schema_json;

    return EnrichSchemaJson(schema_json, output);
}

}  // namespace lineage
