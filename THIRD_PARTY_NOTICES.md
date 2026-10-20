# Сторонние источники и границы лицензирования

Основная GPL-3.0 относится только к 1С-расширению этого репозитория. Этот документ дополняет и не удаляет существующие лицензии/notices.

## ROCTUP

Источник: https://github.com/ROCTUP/1c-mcp-toolkit/tree/fe12903af7a367a9d67dd055c13f4b59bb59d83c, commit `fe12903af7a367a9d67dd055c13f4b59bb59d83c`, GPL-3.0. Уже включённые tools/serializer, schema/lineage и QueryLineageAnalyzer имеют скопированные/адаптированные части. Для дословных тел manager удалены context directives; текущие границы адаптации и manifest описаны в `third_party/ROCTUP-1c-mcp-toolkit/README.md`. Их исходные notices сохраняются.

## tools_ui_1c: самостоятельные обёртки

Источник возможностей: https://github.com/cpr1c/tools_ui_1c/tree/6794068f5bd59f292382102652473b35180b8b3e, commit `6794068f5bd59f292382102652473b35180b8b3e`, repository declaration GPL-3.0. Компоненты: Консоль заданий и просмотр структуры хранения. Новые JOBS helpers — самостоятельная обёртка локальных БСП API; storage helper — самостоятельная обёртка платформенного API. UI-код этих компонентов не копировался и не адаптировался.

README upstream называет Консоль заданий fork `kuzyara/JobsConsole2019.epf` и ссылается на permission request https://github.com/kuzyara/JobsConsole2019.epf/issues/6. Ответ автора `+++` зафиксирован как опубликованное согласие в контексте запроса, отдельная MIT-лицензия не установлена.

Консоль отчётов upstream описана как переработанная консоль с диска ИТС; первичные права отдельно не установлены, её код не переносился. СКД-wrapper реализован независимо, без переноса кода консоли.

Концептуальный источник trusted-gateway и ограничения его использования остаются зафиксированы в существующем `third_party/ROCTUP-1c-mcp-toolkit/README.md`; его исходники в расширение не копировались.

## Чтение настроек СКД

`get_report_settings` — самостоятельная серверная обёртка platform schema/report-user storage API, без копирования ITS-derived консоли. Feature inspiration: https://github.com/cpr1c/tools_ui_1c/tree/6794068f5bd59f292382102652473b35180b8b3e (commit `6794068f5bd59f292382102652473b35180b8b3e`, repo GPL-3.0), компоненты УИ_КонсольОтчетов, УИ_МенеджерХранилищНастроек, УИ_РедакторСКД. Существующие ROCTUP helpers переиспользуются, их notices не снимаются.
