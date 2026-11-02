#!/usr/bin/env python3
"""Рендер иллюстраций курса из TikZ в SVG.

Исходники лежат в figures/<раздел>/<имя>.tikz и содержат только тело
tikzpicture; преамбула figures/preamble.tex подставляется автоматически.

Каждая картинка собирается в двух вариантах:

    docs/images/<имя>.svg        - для светлой темы
    docs/images/<имя>.dark.svg   - для темной

В markdown они подключаются парой (Material for MkDocs):

    ![Подпись](images/имя.svg#only-light)
    ![Подпись](images/имя.dark.svg#only-dark)

Требуется: lualatex и pdftocairo (poppler-utils).

    python3 build_figures.py            # собрать все, что изменилось
    python3 build_figures.py --force    # пересобрать все
    python3 build_figures.py 03-assembly
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FIGURES = ROOT / "figures"
DOCS = ROOT / "docs" / "images"
PREAMBLE = FIGURES / "preamble.tex"
STAMPS = ROOT / ".cache" / "figures"

PALETTES = {
    "light": r"""
\definecolor{fg}{HTML}{1A1A1A}
\definecolor{muted}{HTML}{6E6E6E}
\definecolor{boxfill}{HTML}{F2F2F2}
\definecolor{accent}{HTML}{1E6FD9}
\definecolor{accentb}{HTML}{C2410C}
\definecolor{mixbg}{HTML}{FFFFFF}
""",
    "dark": r"""
\definecolor{fg}{HTML}{DCDDDE}
\definecolor{muted}{HTML}{9A9A9A}
\definecolor{boxfill}{HTML}{3A3D44}
\definecolor{accent}{HTML}{6BA6F5}
\definecolor{accentb}{HTML}{F59E6B}
\definecolor{mixbg}{HTML}{22252C}
""",
}


def check_tools():
    missing = [tool for tool in ("lualatex", "pdftocairo") if not shutil.which(tool)]
    if missing:
        sys.exit(
            "Не найдены: " + ", ".join(missing) + "\n"
            "Нужны TeX Live (lualatex) и poppler-utils (pdftocairo).\n"
            "Готовые SVG лежат в репозитории --- пересборка нужна, только если вы "
            "правите .tikz-исходники."
        )


def source_fingerprint(tikz_path: Path) -> str:
    h = hashlib.sha256()
    h.update(PREAMBLE.read_bytes())
    h.update(tikz_path.read_bytes())
    return h.hexdigest()


def render(tikz_path: Path, out_svg: Path, variant: str) -> None:
    template = PREAMBLE.read_text(encoding="utf-8")
    body = tikz_path.read_text(encoding="utf-8")
    document = template.replace("%%PALETTE%%", PALETTES[variant]).replace("%%BODY%%", body)

    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        tex_file = tmp_path / "figure.tex"
        tex_file.write_text(document, encoding="utf-8")

        texcache = ROOT / ".cache" / "texmf"
        texcache.mkdir(parents=True, exist_ok=True)
        env = os.environ.copy()
        env["TEXMFVAR"] = str(texcache)
        env["TEXMFCACHE"] = str(texcache)

        result = subprocess.run(
            ["lualatex", "-interaction=nonstopmode", "-halt-on-error", "figure.tex"],
            cwd=tmp_path,
            env=env,
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            log = tmp_path / "figure.log"
            errors = ""
            if log.exists():
                errors = "\n".join(
                    line for line in log.read_text(encoding="utf-8", errors="replace").splitlines()
                    if line.startswith("!") or line.startswith("l.")
                )
            raise SystemExit(f"\nОшибка LaTeX в {tikz_path.relative_to(ROOT)}:\n{errors}\n")

        out_svg.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(
            ["pdftocairo", "-svg", str(tmp_path / "figure.pdf"), str(out_svg)],
            check=True,
            capture_output=True,
        )


def build(only_section: str | None = None, force: bool = False) -> int:
    check_tools()
    STAMPS.mkdir(parents=True, exist_ok=True)
    built = skipped = 0
    sections = sorted(p.name for p in FIGURES.iterdir() if p.is_dir())

    for section in sections:
        if only_section and only_section != section:
            continue
        for tikz_path in sorted((FIGURES / section).glob("*.tikz")):
            name = tikz_path.stem
            targets = {
                "light": DOCS / f"{name}.svg",
                "dark": DOCS / f"{name}.dark.svg",
            }

            fingerprint = source_fingerprint(tikz_path)
            stamp = STAMPS / f"{section}__{name}"
            up_to_date = (
                not force
                and stamp.exists()
                and stamp.read_text() == fingerprint
                and all(target.exists() for target in targets.values())
            )
            if up_to_date:
                skipped += 1
                continue

            print(f"  + {name}.svg")
            for variant, target in targets.items():
                render(tikz_path, target, variant)
            stamp.write_text(fingerprint)
            built += 1

    print(f"\nСобрано: {built}, без изменений: {skipped}")
    return built


def main():
    parser = argparse.ArgumentParser(description="Рендер TikZ-иллюстраций курса в SVG")
    parser.add_argument("section", nargs="?", help="собрать только один раздел")
    parser.add_argument("--force", action="store_true", help="пересобрать все")
    args = parser.parse_args()

    print("==> Рендер иллюстраций (TikZ -> SVG)...")
    build(args.section, args.force)


if __name__ == "__main__":
    main()
