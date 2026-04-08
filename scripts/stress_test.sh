#!/bin/bash
# 压测脚本：后台启动服务器，并发运行多个 benchmark 客户端，汇总结果。
# 用法：./scripts/stress_test.sh [ip] [port] [clients] [requests_per_client]
# 示例：./scripts/stress_test.sh 127.0.0.1 5085 10 5000

set -u

IP=${1:-127.0.0.1}
PORT=${2:-5085}
CLIENTS=${3:-10}
REQ=${4:-5000}

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# 优先使用 CMake 构建目录
if [ -x "$DIR/build/benchmark" ]; then
  BIN="$DIR/build"
elif [ -x "$DIR/benchmark" ]; then
  BIN="$DIR"
else
  echo "build first: mkdir build && cd build && cmake .. && make"
  exit 1
fi
BENCH="$BIN/benchmark"
SERVER="$BIN/echoserver"

echo "=== stress test: $CLIENTS clients x $REQ requests to $IP:$PORT ==="

SRV_PID=""
TMP=""
cleanup() {
  if [ -n "${SRV_PID:-}" ]; then
    kill "$SRV_PID" 2>/dev/null || true
    wait "$SRV_PID" 2>/dev/null || true
  fi
  if [ -n "${TMP:-}" ] && [ -d "$TMP" ]; then
    rm -rf "$TMP"
  fi
}
trap cleanup EXIT

$SERVER "$IP" "$PORT" &
SRV_PID=$!
sleep 1
if ! kill -0 $SRV_PID 2>/dev/null; then
  echo "server failed to start"
  exit 1
fi

echo "server pid=$SRV_PID"

TMP=$(mktemp -d)
START_NS=$(date +%s%N)
PIDS=()
for i in $(seq 1 $CLIENTS); do
  $BENCH $IP $PORT $REQ > "$TMP/$i.log" 2>&1 &
  PIDS+=($!)
done
FAILED_CLIENTS=0
for pid in "${PIDS[@]}"; do
  if ! wait "$pid"; then
    FAILED_CLIENTS=$((FAILED_CLIENTS + 1))
  fi
done
END_NS=$(date +%s%N)

TOTAL_OK=0
TOTAL_REQ=$((CLIENTS * REQ))
PARSED_CLIENTS=0
PARSE_FAILED=0
for i in $(seq 1 $CLIENTS); do
  if [ -f "$TMP/$i.log" ]; then
    LINE=$(grep -E '^benchmark:' "$TMP/$i.log" | tail -n 1 || true)
    if [ -n "$LINE" ]; then
      OK=$(echo "$LINE" | sed -n 's/.*ok=\([0-9][0-9]*\).*/\1/p')
      if [ -z "$OK" ]; then
        OK=$(echo "$LINE" | sed -n 's/.*: \([0-9][0-9]*\)\/[0-9][0-9]*.*/\1/p')
      fi
      if [ -n "$OK" ]; then
        TOTAL_OK=$((TOTAL_OK + OK))
      else
        PARSE_FAILED=$((PARSE_FAILED + 1))
        echo "warn: failed to parse benchmark output from client $i: $LINE" >&2
      fi
      PARSED_CLIENTS=$((PARSED_CLIENTS + 1))
    fi
  fi
done

ELAPSED=$(awk -v start="$START_NS" -v end="$END_NS" 'BEGIN { printf "%.6f", (end - start) / 1000000000 }')
AGG_QPS=$(awk -v ok="$TOTAL_OK" -v elapsed="$ELAPSED" 'BEGIN { if (elapsed > 0) printf "%.2f", ok / elapsed; else print "0.00" }')
SUCCESS_RATE=$(awk -v ok="$TOTAL_OK" -v total="$TOTAL_REQ" 'BEGIN { if (total > 0) printf "%.2f", ok * 100 / total; else print "0.00" }')

echo "=== summary ==="
echo "clients_started=$CLIENTS parsed_clients=$PARSED_CLIENTS failed_clients=$FAILED_CLIENTS parse_failed=$PARSE_FAILED"
echo "requests_ok=$TOTAL_OK requests_total=$TOTAL_REQ success_rate=${SUCCESS_RATE}%"
echo "elapsed=${ELAPSED}s aggregate_qps=$AGG_QPS"

if [ "$TOTAL_OK" -eq "$TOTAL_REQ" ] && [ "$FAILED_CLIENTS" -eq 0 ] && [ "$PARSE_FAILED" -eq 0 ]; then
  echo "PASS"
  exit 0
else
  echo "FAIL"
  exit 1
fi
