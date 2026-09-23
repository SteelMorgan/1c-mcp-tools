# RegexHelper — 1C Native API Component

C++ компонента для 1С:Предприятие. Выполняет поиск совпадений по регулярным выражениям в массиве текстов. Используется для анонимизации данных — находит персональные данные по набору паттернов.

Движок регулярных выражений — [PCRE2](https://github.com/PCRE2Project/pcre2) v10.46 (UTF-16, флаги PCRE2_UTF | PCRE2_UCP).

## Требования

- Windows (x86 или x64)
- CMake 3.16+
- Visual Studio 2022 (MSVC)

## Сборка

Собирать нужно под разрядность платформы 1С, на которой будет работать компонента.

```bash
cd native_components/RegexHelper

# x64
cmake -B build -A x64
cmake --build build --config Release
# Результат: build/Release/RegexHelper.dll

# x86
cmake -B build_x86 -A Win32
cmake --build build_x86 --config Release
# Результат: build_x86/Release/RegexHelper.dll
```

PCRE2 скачивается автоматически через CMake FetchContent при первой сборке.

## Сборка под Linux (через Docker)

Требования: [Docker Desktop](https://www.docker.com/products/docker-desktop/) на Windows.

```
native_components\RegexHelper\build_linux.bat
```

Результат: `build_linux/RegexHelper.so`

База образа — Ubuntu 20.04 (glibc 2.31). Подходит для RedOS 8.x и большинства Linux-серверов 1С.
Для RedOS 7.x замените базу в `Dockerfile.linux` на `debian:buster`.

### Установка на Linux-сервер 1С

```bash
cp build_linux/RegexHelper.so \
   ../../1c/MCPToolkit/MCPToolkit/Templates/RegexHelper/Ext/Template.bin
```

## Установка в обработку

Скопируйте DLL нужной разрядности в макет обработки под именем `Template.bin`:

```bash
# x64
cp build/Release/RegexHelper.dll \
   ../../1c/MCPToolkit/MCPToolkit/Templates/RegexHelper/Ext/Template.bin

# x86
cp build_x86/Release/RegexHelper.dll \
   ../../1c/MCPToolkit/MCPToolkit/Templates/RegexHelper/Ext/Template.bin
```

После обновления Template.bin нужно пересохранить обработку в конфигураторе 1С.

## API

### Методы

| Метод (EN) | Метод (RU) | Параметры | Возврат | Описание |
|-----------|-----------|-----------|---------|----------|
| `FindMatchesInTexts(rulesJson, textsJson)` | `НайтиСовпаденияВТекстах` | 2 строки JSON | Строка JSON | Найти все совпадения паттернов в текстах |
| `ValidatePattern(pattern)` | `ПроверитьШаблон` | Строка | Строка | Проверить корректность регулярного выражения; возвращает "" если OK, иначе текст ошибки |
| `Version()` | `Версия` | — | Строка | Версия компоненты |

### FindMatchesInTexts / НайтиСовпаденияВТекстах

**rulesJson** — JSON-массив правил:
```json
[
  {"pattern": "\\d{3}-\\d{2}-\\d{2}", "category": "phone"},
  {"pattern": "[A-Z][a-z]+ [A-Z][a-z]+", "category": "name"}
]
```

**textsJson** — JSON-массив строк для поиска:
```json
["Иван Иванов, тел 123-45-67", "другой текст"]
```

**Возвращает** JSON-массив массивов совпадений (по одному массиву на каждый текст), отсортированных по убыванию позиции (для замены справа налево):
```json
[
  [
    {"match": "Иван Иванов", "start": 0, "length": 11, "category": "name"},
    {"match": "123-45-67",   "start": 17, "length": 9,  "category": "phone"}
  ],
  []
]
```

> Уже существующие токены вида `[ABC-123]` автоматически защищаются от повторного совпадения.

### ValidatePattern / ПроверитьШаблон

Возвращает пустую строку если паттерн корректен, иначе — описание ошибки PCRE2 с позицией:

```
nothing to repeat (at offset 3)
```
