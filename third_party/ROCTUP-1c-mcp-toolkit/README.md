# ROCTUP 1c-mcp-toolkit

Imported from `https://github.com/ROCTUP/1c-mcp-toolkit` at commit
`fe12903af7a367a9d67dd055c13f4b59bb59d83c` under GPL-3.0.

The selected BSL tool bodies were mechanically extracted from this pin.
Context adapters replace
the original form/client transport. The current 1C boundary retains six selected
tools, descriptive schema/lineage, and irreversible `[SECRET_REMOVED]` secret cut.
Mutable masking policy, reversible mapping, history, submit and reveal controls are
not part of this boundary. The upstream HTTP/proxy/restart/screenshot/business tools
are not registered.

**Supply status:** `QueryLineageAnalyzer` is no longer shipped or loaded by the
extension — its template was removed and query column lineage is derived only
from the platform `СхемаЗапроса`. This catalog remains as the upstream source
archive (GPL-3.0 notices and provenance), not a runtime dependency.

`native_components/` contains the QueryLineageAnalyzer parser
adaptation with its regression test. ELF builds were made
in Ubuntu 20.04; the adapted QueryLineageAnalyzer binary matches the source
recorded in `native-provenance.md`. Build hashes and test results are recorded
there; build logs are preserved in the task artifacts.

`RegexHelper` (source, binary template and its PCRE2 dependency) was removed
together with its sole consumer — the dead regex path of
`mcp_ИнструментыROCTUP/ObjectModule`. No native component is shipped now.

Redistribution of the imported/adapted work carries GPL-3.0 obligations. This
notice does not relicense unrelated framework code.
