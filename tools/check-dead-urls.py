#!/usr/bin/env python3

# Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, Iterable, Set, Tuple

import httpx

URL_RE = re.compile(r"https?://[^\s'\"\)\],<>]+", re.IGNORECASE)


def find_urls_in_tree(
    root: Path,
    exts: Iterable[str] = (".py", ".js", ".ts", ".html", ".md", ".rst"),
) -> Set[str]:
    EXCLUDED_DIRS = {".venv", ".git", "node_modules"}

    urls: Set[str] = set()
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        # Skip anything inside excluded directories
        if any(part in EXCLUDED_DIRS for part in path.parts):
            continue
        if exts and path.suffix not in exts:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        urls.update(map(lambda x: x.strip("."), URL_RE.findall(text)))
    return urls


def make_client(timeout: float) -> httpx.Client:
    # Configure a shared client with sane defaults.
    # You can tweak headers, proxies, HTTP/2, etc. here.
    return httpx.Client(
        timeout=httpx.Timeout(timeout),
        follow_redirects=True,
        http2=True,
        headers={"User-Agent": "url-checker-httpx/1.0"},
    )


def check_url_once(client: httpx.Client, url: str) -> tuple[int, str]:
    try:
        # Try HEAD first; some servers don't support it well.
        r = client.head(url)
        status = r.status_code
        if status >= 400 or status == 405:
            r = client.get(url)
            status = r.status_code
        return status, ""
    except (httpx.RequestError, UnicodeError, ValueError) as exc:
        # UnicodeError catches IDNA issues like "label empty or too long"
        return 0, f"{type(exc).__name__}: {exc}"


def is_alive_with_retries(
    client: httpx.Client,
    url: str,
    retries: int = 2,
) -> Tuple[bool, int, str]:
    last_status = 0
    last_error = ""
    for _ in range(retries + 1):
        status, err = check_url_once(client, url)
        last_status, last_error = status, err
        # Treat < 500 as final answer; retry only on network / 5xx / 0.
        if status and status < 500:
            break
    ok = (last_status != 0) and (last_status < 400)
    return ok, last_status, last_error


def check_urls(
    urls: Iterable[str],
    timeout: float = 5.0,
    retries: int = 2,
) -> Dict[str, Tuple[bool, int, str]]:
    urls = sorted(set(urls))
    results: Dict[str, Tuple[bool, int, str]] = {}
    with make_client(timeout) as client:
        for url in urls:
            ok, status, err = is_alive_with_retries(client, url, retries=retries)
            results[url] = (ok, status, err)
    return results


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description="Scan source tree for URLs and check if they are alive using httpx."
    )
    parser.add_argument(
        "root",
        nargs="?",
        default=".",
        help="Root directory to scan (default: current directory).",
    )
    parser.add_argument(
        "--ext",
        action="append",
        dest="exts",
        default=None,
        help="File extension to include (e.g. --ext .py). "
        "Can be given multiple times. Default: .py,.js,.ts,.html,.md,.rst",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="Per-request timeout in seconds (default: 5.0).",
    )
    parser.add_argument(
        "--retries",
        type=int,
        default=2,
        help="Number of retries on network/5xx errors (default: 2).",
    )
    args = parser.parse_args(argv)

    root = Path(args.root).resolve()
    if args.exts:
        exts = tuple(args.exts)
    else:
        exts = (".py", ".js", ".ts", ".html", ".md", ".rst")

    urls = find_urls_in_tree(root, exts=exts)
    if not urls:
        print("No URLs found.", file=sys.stderr)
        return 0

    whitelisted_urls = [
        # Private
        "https://gitlab.com/AAshtari/simcamp.git",
        "https://gitlab.com/AAshtari/simcamp/-/settings/integrations",
        "https://gitlab.cttc.es/ns3-new-radio/nr",
        "https://gitlab.cttc.es/ns3-new-radio/sim-campaigns.git",
        # Blocked by server, but works
        "https://medium.com/humans-create-software/composition-over-inheritance-cb6f88070205",
        "https://doi.org/10.1145/3592149.3592159",
        # Kept for historical reasons
        "https://docs.gitlab.com/ee/gitlab-basics/add-file.html#add-a-file-using-the-command-line",
        "https://docs.gitlab.com/ee/user/project/merge_requests/merge_when_pipeline_succeeds.html",
        # Conditionally built
        "https://cttc-lena.gitlab.io/nr/html/gsoc-nr-rl-based-sched_8cc.html",
    ]
    urls = urls.difference(set(whitelisted_urls))

    print(f"Found {len(urls)} unique URLs. Checking...\n")
    results = check_urls(urls, timeout=args.timeout, retries=args.retries)

    alive = []
    dead = []

    for url, (ok, status, err) in results.items():
        if ok:
            alive.append((url, status))
        else:
            dead.append((url, status, err))

    print("Alive URLs:")
    for url, status in alive:
        print(f"  [OK] {status} {url}")

    print("\nDead/Problematic URLs:")
    for url, status, err in dead:
        status_str = status or "-"
        if err:
            print(f"  [XX] {status_str} {url}  ({err})")
        else:
            print(f"  [XX] {status_str} {url}")

    print(f"\nSummary: {len(alive)} OK, {len(dead)} dead/problematic.")

    # Non-zero exit if any URL failed, so you can use it in CI.
    return 0 if not dead else 1


if __name__ == "__main__":
    raise SystemExit(main())
