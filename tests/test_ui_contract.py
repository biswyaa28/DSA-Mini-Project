from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
WEB_DIR = ROOT / "web"


def read_web(filename: str) -> str:
    return (WEB_DIR / filename).read_text(encoding="utf-8")


class TestExhibitionHtmlContract(unittest.TestCase):
    def test_index_contains_exhibition_shell(self):
        html = read_web("index.html")
        required_snippets = [
            'class="app-layout exhibition-layout"',
            'class="hero-strip"',
            'id="hero-metrics"',
            'id="graph-legend"',
            'id="graph-callout"',
            'class="input-label"',
            'class="input-helper"',
        ]
        for snippet in required_snippets:
            self.assertIn(snippet, html, f"Missing HTML snippet: {snippet}")

    def test_index_keeps_six_navigation_tabs(self):
        html = read_web("index.html")
        tabs = re.findall(
            r'data-page="(overview|stations|booking|routing|mst|analysis)"', html
        )
        self.assertEqual(len(tabs), 6, "Expected exactly six primary navigation tabs")


class TestExhibitionCssContract(unittest.TestCase):
    def test_styles_define_required_tokens_and_motion(self):
        css = read_web("styles.css")
        required_tokens = [
            "--bg-app:",
            "--bg-nav:",
            "--accent-primary:",
            "--accent-support:",
            "--success:",
            "--danger:",
        ]
        for token in required_tokens:
            self.assertIn(token, css, f"Missing CSS token: {token}")

        self.assertIn("@keyframes heroReveal", css)
        self.assertIn("@keyframes cardReveal", css)
        self.assertIn(".state-panel", css)
        self.assertIn("@media (max-width: 1024px)", css)
        self.assertIn("@media (max-width: 768px)", css)


class TestExhibitionJsContract(unittest.TestCase):
    def test_app_contains_state_and_hero_helpers(self):
        js = read_web("app.js")
        required_symbols = [
            "const PAGE_META",
            "function renderStatePanel",
            "function setSectionState",
            "function updateHeroMetrics",
            "function setGraphCallout",
        ]
        for symbol in required_symbols:
            self.assertIn(symbol, js, f"Missing JS helper: {symbol}")


if __name__ == "__main__":
    unittest.main()
