# ROCTUP 1c-mcp-toolkit — TASK-221

Imported from `https://github.com/ROCTUP/1c-mcp-toolkit` at commit
`fe12903af7a367a9d67dd055c13f4b59bb59d83c` under GPL-3.0.

The selected BSL tool and lineage algorithm bodies are mechanically extracted by
`tasks/221-roctup-mcp-tools-port/extract_port_stage.py`. Context adapters replace
the original form/client transport. The current 1C boundary retains six selected
tools, descriptive schema/lineage, and irreversible `[SECRET_REMOVED]` secret cut.
Mutable masking policy, reversible mapping, history, submit and reveal controls are
not part of this boundary. The upstream HTTP/proxy/restart/screenshot/business tools
are not registered.

`native_components/` contains the pinned upstream native sources and the TASK-221
QueryLineageAnalyzer parser adaptation with its regression test. Runtime templates
contain ELF builds made in Ubuntu 20.04; the adapted QueryLineageAnalyzer binary
matches the source recorded in `native-provenance.md`. Build hashes and test results
are recorded there; build logs are preserved in the task artifacts.

Redistribution of the imported/adapted work carries GPL-3.0 obligations. This
notice does not relicense unrelated framework code.
