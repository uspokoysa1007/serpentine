# 🐍 Serpentine

> **Cross-Platform Remote Administration & System Diagnostics Toolkit with Telegram Management**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Python 3.11+](https://img.shields.io/badge/Python-3.11+-3776AB.svg?logo=python&logoColor=white)](https://python.org)
[![aiogram 3.x](https://img.shields.io/badge/aiogram-3.x-2CA5E0.svg?logo=telegram&logoColor=white)](https://github.com/aiogram/aiogram)
[![Platform: Windows](https://img.shields.io/badge/Client-Windows_x64-0078D6.svg?logo=windows&logoColor=white)](https://microsoft.com/windows)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 📌 О проекте / Overview

**Serpentine** — исследовательский проект архитектуры клиент-серверного удалённого администрирования и мониторинга рабочих станций под управлением Windows с интерфейсом Telegram-бота.

Архитектура системы состоит из двух компонентов:
1. **Windows Diagnostic Agent (C++20)** — легковесный фоновый агент сбора системной информации и телеметрии на WinHTTP API с демонстрационным консольным интерфейсом (Snake mini-game).
2. **Management Server & Telegram Bot (Python / aiohttp / aiogram)** — централизованный сервер диспетчеризации задач, предоставляющий REST API для агентов и графический пульт управления в Telegram.

```
       [ Windows Workstation ]
      ┌─────────────────────────┐
      │   Console UI / Snake    │
      │  ┌───────────────────┐  │
      │  │ WinHTTP Transport │  │
      └──┴─────────┬─────────┴──┘
                   │  POST /telemetry, /result, /data
                   ▼
      ┌─────────────────────────┐
      │    Central Hub API      │  (aiohttp REST API)
      │     (Python 3.11)       │
      └────────────┬────────────┘
                   │  aiogram 3.x Long-Polling
                   ▼
      ┌─────────────────────────┐
      │      Telegram Bot       │  (Admin Dashboard)
      │  ┌───────────────────┐  │
      │  │   Inline Control  │  │
      └──┴───────────────────┴──┘
```

---

## ✨ Основные возможности

### 🖥 C++ Client Agent
- **📊 Системная диагностика**: проверка текущих привилегий процесса (`CheckTokenMembership`) и окружения пользователя.
- **📸 Захват рабочего стола**: создание снимков экрана через GDI+ API для удалённой поддержки и диагностики неполадок.
- **📂 Управление файлами и отчётами (miniz)**: локальная компрессия файлов журналов и диагностических данных в ZIP-архив без сторонних зависимостей.
- **⚡ Удалённое исполнение команд**: выполнение системных утилит через `cmd.exe` и `powershell.exe` с передачей вывода администратору.
- **🌐 Сетевой транспорт**: надёжная связь по HTTP/HTTPS через нативный WinHTTP API.
- **🎮 Консольный интерфейс**: встроенный пример интерактивной консольной игры для демонстрации работы фонового потока.

### 🤖 Telegram Management Server
- **📱 Мобильный пульт управления**: мониторинг подключённых систем через защищённый Telegram-чат.
- **📊 Список активных узлов**: отображение статуса подключённых рабочих станций и уровня доступа.
- **🎛 Удобные Inline-команды**:
  - `CMD` / `PowerShell` — отправка команд на выполнение.
  - `📸 Скриншот` — оперативный просмотр состояния экрана.
  - `📂 Архивация` — сбор диагностических логов и выгрузка отчёта.
- **☁️ Облачное развёртывание**: поддержка Docker, Amvera Cloud (`amvera.yml`) или любого Linux VPS.

---

## 🚀 Быстрый старт

### 1. Настройка и запуск Сервера управления

#### Переменные окружения:
| Переменная | Описание | Пример |
|---|---|---|
| `BOT_TOKEN` | Токен Telegram-бота от `@BotFather` | `123456789:ABCdefGHI...` |
| `ADMIN_ID` | Ваш Telegram User ID (число) | `987654321` |
| `PORT` | Порт для прослушивания HTTP API (по умолчанию 8080) | `8080` |

#### Запуск:
```bash
# Клонирование репозитория
git clone https://github.com/uspokoysa1007/serpentine.git
cd serpentine

# Установка зависимостей
pip install -r requirements.txt

# Экспорт переменных (Linux / macOS)
export BOT_TOKEN="ВАШ_ТОКЕН_БОТА"
export ADMIN_ID="ВАШ_TELEGRAM_ID"
export PORT="8080"

# Запуск
python bot/bot.py
```

---

### 2. Сборка C++ Клиента

```bash
cd cpp
mkdir build && cd build
cmake .. -A x64
cmake --build . --config Release
```

#### Требования:
- Компилятор с поддержкой **C++20** (Visual Studio 2022 / MSVC или LLVM-MinGW).
- CMake 3.20+.
- Windows SDK.

---

## 📁 Структура проекта

```
serpentine/
├── bot/
│   └── bot.py            # Сервер REST API (aiohttp) и Telegram бот (aiogram 3)
├── cpp/
│   ├── CMakeLists.txt    # Конфигурация сборки CMake
│   ├── main.cpp          # Точка входа клиента и цикл связи
│   ├── snake_game.cpp    # Консольная игра
│   ├── http.h            # HTTP-клиент WinHTTP API
│   ├── admin.h           # Проверка системных привилегий
│   ├── screenshot.h      # Модуль создания снимка экрана (GDI+)
│   ├── shell.h           # Исполнение диагностических команд
│   ├── users.h           # Получение списка учётных записей
│   ├── zip.h             # Архивирование данных (miniz)
│   └── util.h            # Вспомогательные функции
├── third_party/
│   ├── miniz.c / miniz.h # Библиотека Deflate/ZIP компрессии
│   └── nlohmann/json.hpp # JSON парсер
├── amvera.yml            # Конфигурация облачного деплоя Amvera
├── requirements.txt      # Зависимости Python
└── README.md             # Документация проекта
```

---

## 📄 Лицензия
Распространяется под лицензией [MIT](LICENSE).
