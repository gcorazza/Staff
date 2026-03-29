#!/bin/bash
# sendfile.sh - Send a file to ESP32 TCP server

set -euo pipefail

SERVICE_TYPE="${SERVICE_TYPE:-_arcane._tcp}"
TARGET="${1:-}"
SAVE_MODE_RAW="${2:-nosave}"
FILE="${3:-}"
PORT=888

usage() {
  cat <<EOF
Usage: $0 <service|ip> [save-mode] <png-file>
  service|ip : mDNS service instance name advertised via ${SERVICE_TYPE} or raw IPv4
  save-mode  : nosave | temp | persistent (default: nosave)
  png-file   : path to the PNG that should be streamed to the staff
Environment:
  SERVICE_TYPE  Override the mDNS service type to browse (default: _arcane._tcp)
EOF
}

if [[ -z "$TARGET" || -z "$FILE" ]]; then
  usage
  exit 1
fi

if [[ ! -f "$FILE" ]]; then
  echo "File not found: $FILE" >&2
  exit 1
fi

if ! command -v nc >/dev/null 2>&1; then
  echo "netcat (nc) is required" >&2
  exit 1
fi

normalize_mode() {
  local raw="${1:-}"
  raw="${raw,,}"
  case "$raw" in
    ""|"nosave"|"none") echo "nosave" ;;
    "temp"|"temporary") echo "temp" ;;
    "persist"|"persistent"|"permanent"|"effects") echo "persistent" ;;
    *) return 1 ;;
  esac
}

if ! SAVE_MODE=$(normalize_mode "$SAVE_MODE_RAW"); then
  echo "Unsupported save mode '$SAVE_MODE_RAW'. Use nosave, temp, persistent (aliases: none, temporary, persist, permanent)." >&2
  exit 1
fi

resolve_ip_from_service() {
  local instance="$1"
  local ip=""
  if command -v avahi-browse >/dev/null 2>&1; then
    ip=$(avahi-browse -rt "$SERVICE_TYPE" 2>/dev/null |
      awk -F';' -v name="$instance" '($1=="=" || $1=="+") && $4==name {print $8; exit}')
  elif command -v dns-sd >/dev/null 2>&1; then
    ip=$(dns-sd -L "$instance" "$SERVICE_TYPE" local 2>/dev/null |
      awk '/can address/ {print $NF; exit}')
  else
    return 1
  fi
  if [[ -n "$ip" ]]; then
    printf '%s\n' "$ip"
    return 0
  fi
  return 1
}

resolve_ip_or_host() {
  local target="$1"
  if [[ "$target" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "$target"
    return 0
  fi
  if ip=$(resolve_ip_from_service "$target" 2>/dev/null); then
    if [[ -n "$ip" ]]; then
      echo "$ip"
      return 0
    fi
  fi
  if ip=$(getent hosts "${target}.local" 2>/dev/null | awk '{print $1; exit}'); then
    echo "$ip"
    return 0
  fi
  return 1
}

ESP32_IP=$(resolve_ip_or_host "$TARGET") || {
  echo "Could not resolve IP for '$TARGET'. Ensure the mDNS service (${SERVICE_TYPE}) is visible or pass an IP address." >&2
  exit 1
}

FILESIZE=$(stat -c%s "$FILE")
FILENAME=$(basename "$FILE")

echo "Sending $FILENAME ($FILESIZE bytes) to $ESP32_IP:$PORT (mode=$SAVE_MODE)"
{
  printf "\nPLAYPNG %s %d %s\n" "$FILENAME" "$FILESIZE" "$SAVE_MODE"
  cat "$FILE"
} | nc -q 0 "$ESP32_IP" "$PORT"
echo "Done."

