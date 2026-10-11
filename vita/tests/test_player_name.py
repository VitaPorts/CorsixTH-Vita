"""Exercise the shipped App:fixConfig player-name path under LuaJIT 2.1.

Install the host-only test runner with:
  python -m pip install --target build-vita-engine/host-tools/lupa lupa==2.8 --no-deps
"""

from pathlib import Path
import sys
import unittest


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "build-vita-engine" / "host-tools" / "lupa"))
from lupa.luajit21 import LuaRuntime  # noqa: E402


SOURCE = (ROOT / "CorsixTH" / "Lua" / "app.lua").read_text(encoding="utf-8")
START = SOURCE.index("function App:fixConfig()")
END = SOURCE.index("\nfunction App:saveConfig()", START)
FIX_CONFIG = SOURCE[START:END]


class PlayerNameTest(unittest.TestCase):
    def setUp(self):
        self.lua = LuaRuntime()
        self.lua.execute(
            "App = {MIN_WINDOW_WIDTH = 640, MIN_WINDOW_HEIGHT = 480}; "
            "os.getenv = function(key) return test_env[key] end"
        )
        self.lua.execute(FIX_CONFIG)

    def check_name(self, configured, environment, expected):
        self.lua.globals().test_env = self.lua.table_from(environment)
        config = self.lua.table_from(
            {"player_name": configured, "width": 640, "height": 480, "ui_scale": 1}
        )
        app = self.lua.table_from({"config": config})
        self.lua.globals().App.fixConfig(app)
        self.assertEqual(config.player_name, expected)

    def test_missing_environment_uses_player(self):
        self.check_name("", {}, "PLAYER")

    def test_blank_environment_uses_player(self):
        self.check_name("  ", {"USER": "  ", "USERNAME": ""}, "PLAYER")

    def test_username_fallback(self):
        self.check_name("", {"USERNAME": "  Vita Owner  "}, "Vita Owner")

    def test_user_precedes_username(self):
        self.check_name("", {"USER": "  Alice  ", "USERNAME": "Bob"}, "Alice")

    def test_configured_name_precedes_environment(self):
        self.check_name("  Doctor Ada  ", {"USER": "Alice"}, "Doctor Ada")


if __name__ == "__main__":
    unittest.main(verbosity=2)
