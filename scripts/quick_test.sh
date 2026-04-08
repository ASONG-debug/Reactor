#!/bin/bash
# 快速功能测试：启动服务器，运行若干客户端，校验回显正确性。
# 用法：./scripts/quick_test.sh [ip] [port]

set -e

IP=${1:-127.0.0.1}
PORT=${2:-5085}

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ -x "$DIR/build/echoserver" ]; then
  BIN="$DIR/build"
elif [ -x "$DIR/echoserver" ]; then
  BIN="$DIR"
else
  echo "build first: mkdir build && cd build && cmake .. && make"
  exit 1
fi
CLIENT="$BIN/client"
SERVER="$BIN/echoserver"

echo "=== quick test $IP:$PORT ==="
# 使用 ip port 启动，与传入参数一致
$SERVER "$IP" "$PORT" &
SRV_PID=$!
sleep 1

$CLIENT $IP $PORT &
C1=$!
$CLIENT $IP $PORT &
C2=$!
wait $C1 $C2

kill $SRV_PID 2>/dev/null || true
wait $SRV_PID 2>/dev/null || true
echo "=== done ==="
