#pragma once

#include "ast.h"

#include <string>

namespace lineage {

std::string EnrichSchemaJson(const std::string& schema_json, const OutputLineageMap& output_lineage);

}  // namespace lineage
