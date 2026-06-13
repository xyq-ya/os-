#!/usr/bin/env bash
# 使用清华镜像安装 gh，并给出 apt 换源说明
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLS="$ROOT/.tools"
GH_DEB_URL="https://mirrors.tuna.tsinghua.edu.cn/debian/pool/main/g/gh/gh_2.46.0-4_amd64.deb"

mkdir -p "$TOOLS"
cd "$TOOLS"

echo "[1/3] 从清华镜像下载 gh ..."
curl -fL --connect-timeout 15 --retry 2 "$GH_DEB_URL" -o gh.deb

echo "[2/3] 解压 gh 到 $TOOLS/usr/bin/gh"
ar x gh.deb
tar xf data.tar.*
"$TOOLS/usr/bin/gh" --version

echo "[3/3] 完成"
echo
echo "使用方式:"
echo "  $TOOLS/usr/bin/gh auth login"
echo "  $TOOLS/usr/bin/gh repo create os实验 --public --source=$ROOT --remote=origin --push"
echo
echo "如需系统级 apt 换清华源，参考:"
echo "  $ROOT/scripts/tsinghua-apt-sources.list"
echo "  https://mirrors.tuna.tsinghua.edu.cn/help/debian/"
