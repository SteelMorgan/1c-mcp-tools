# ROCTUP native Linux provenance

> Статус поставки: `QueryLineageAnalyzer` больше не поставляется и не
> подключается расширением (макет удалён; lineage — только платформенная
> `СхемаЗапроса`). Этот документ описывает историческую сборку upstream-
> исходников, оставленных как архив provenance.

## Upstream и область

- Upstream: `ROCTUP/1c-mcp-toolkit`, pin `fe12903af7a367a9d67dd055c13f4b59bb59d83c`.
- Поставляется только `QueryLineageAnalyzer`: `src/parser.cpp` адаптирован: разбирает отдельное поле `Ссылка` как ссылку на поле, сохраняя `Поле ССЫЛКА Справочник.X` как оператор с типом; добавлен `tests/test_main.cpp`.
- `RegexHelper` **удалён**: единственный потребитель
  (`НайтиRegexСовпадения` в `mcp_ИнструментыROCTUP/ObjectModule`) был мёртвым
  кодом и удалён вместе с макетом. Соответствующий C++ source и бинарь из
  расширения исключены; PCRE2 (статическая зависимость только RegexHelper)
  больше не поставляется — его licence files сняты из этого каталога.
- Этот документ описывает только native C++ build. Provenance скопированных BSL-функций будет отдельным artifact от worker-а, который выполнял перенос; здесь BSL provenance не реконструируется.

## Linux build recipe

Сборка выполнена в task-owned Docker builders из upstream `Dockerfile.linux`, без bind mount исходного workspace:

- target: `linux/amd64` (ELF64 x86-64);
- base: `ubuntu:20.04@sha256:8feb4d8ca5354def3d8fce243717141ce31e2c428701f6682bd2fafe15388214`;
- toolchain: GCC 9.4.0, CMake 3.16.3, GNU ld 2.34, glibc 2.31;
- build concurrency: максимум `-j2`.

Для адаптированного QueryLineageAnalyzer в source catalog добавлен native test target:

```sh
mkdir -p /src/build_linux
cd /src/build_linux
cmake .. -DCMAKE_BUILD_TYPE=Release -DQUERY_LINEAGE_BUILD_TESTS=ON
cmake --build . -- -j2
ctest --output-on-failure
```

В upstream pin тестовых исходников нет. Добавленный `tests/test_main.cpp` прошёл в Ubuntu 20.04 builder (`ctest`: 1/1): отдельное `Ссылка КАК Ref`, обычное `Наименование КАК ФИО`, обогащение `schema.columns[].sources`, `Ссылка ИЗ` без алиаса и `Поле ССЫЛКА Справочник.X`. Загрузка новой библиотеки платформой 1С и 1С runtime tests не выполнялись в этой фазе.

## Dependencies

- nlohmann/json v3.12.0: `https://raw.githubusercontent.com/nlohmann/json/v3.12.0/single_include/nlohmann/json.hpp`, SHA-256 `aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63`.
- PCRE2 10.46 не требуется: была статической зависимостью только удалённого RegexHelper (upstream `pcre2-10.46.tar.gz`, SHA-256 `8d28d7f2c3b970c3a4bf3776bcbb5adfc923183ce74bc8df1ebaad8c1985bd07` — запись сохранена для истории сборки).

## Verified outputs

QueryLineageAnalyzer собран из адаптированного source catalog:

| Component | SHA-256 | Size | ABI notes |
|---|---|---:|---|
| `QueryLineageAnalyzer.so` | `e89e8b523255e22b7796b630e632a2bf6682a9e3eb6b6f843cbf709861c5ac17` | 371608 bytes | ELF64 x86-64, SONAME `QueryLineageAnalyzer.so`, max GLIBC `2.14` |

Проверены exports `GetClassNames`, `GetClassObject`, `DestroyObject`, `SetPlatformCapabilities` (4/4). Runtime dependencies: `libstdc++.so.6`, `libgcc_s.so.1`, `libc.so.6`, `libm.so.6`. Максимальные C++ ABI symbols: `GLIBCXX_3.4.21`, `CXXABI_1.3.9`.

Историческая запись: до удаления поставлялся также
`RegexHelper.so` (SHA-256 `2ebf13d907656d3340c4c842df5a7dd6e970422ae4046280dfabe6fa4f8770d9`,
425456 bytes, PCRE2 статически включён). Бинарь и соответствующий source
выведены из поставки вместе с потребителем.

## License audit

В source-каталоге сохранены проверенные полные notices:

- `ROCTUP-LICENSE` и `LICENSE.GPL-3.0.txt`: SHA-256 `3972dc9744f6499f0f9b2dbf76696f2ae7ad8af9b23dde66d6af86c9dfb36986`;
- `nlohmann-json-LICENSE.MIT`: SHA-256 `46a65cffd1ea955132d95a8dd921640714a8d6b537d2e4e482d31145ae95b603`.

Licence-файлы PCRE2 (`pcre2-10.46-COPYING` SHA-256 `99272c55f3dcfa07a8a7e15a5c1a33096e4727de74241d65fa049fccfdd59507`, `pcre2-10.46-LICENCE.md` SHA-256 `9cf7ac6976099a1d856826d3ef1b093bd6b84489dc6100628ac79e740cf9885a`) сняты: PCRE2 больше ни в один поставляемый компонент не входит.

Этот source catalog содержит полный соответствующий native C++ source, адаптацию, тест и notices под GPL-3.0. Mirror-копии каталога содержат notices и provenance, но не дублируют C++ source. При распространении одного mirror вместе с бинарным `Template.bin` нужно обеспечить получателю доступ к этому изменённому соответствующему source; исходный upstream pin сам по себе недостаточен.
