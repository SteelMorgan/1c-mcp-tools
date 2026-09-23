#pragma once

#include "ast.h"

namespace lineage {

OutputLineageMap ResolveBatchLineage(const QueryBatchNode& batch);

}  // namespace lineage
