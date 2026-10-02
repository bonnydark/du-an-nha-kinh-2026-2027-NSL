#!/usr/bin/env bash
set -euo pipefail

WORKER_NAME="nsl-greenhouse"

echo "=============================================="
echo " NSL GREENHOUSE • CLOUDFLARE ONE-COMMAND SETUP"
echo "=============================================="

command -v node >/dev/null || { echo "❌ Cần Node.js"; exit 1; }
command -v npm >/dev/null || { echo "❌ Cần npm"; exit 1; }
command -v npx >/dev/null || { echo "❌ Cần npx"; exit 1; }

echo "→ Cài Wrangler..."
npm install --no-save wrangler@latest

echo "→ Kiểm tra Cloudflare login..."
if ! npx wrangler whoami >/dev/null 2>&1; then
  echo "→ Chưa đăng nhập. Mở trình duyệt để đăng nhập Cloudflare..."
  npx wrangler login
fi

echo "→ Deploy Worker: $WORKER_NAME"
cd worker
npx wrangler deploy

echo
echo "=============================================="
echo " ✅ DEPLOY XONG"
echo " 🌐 https://$WORKER_NAME.<your-subdomain>.workers.dev"
echo " ❤️  https://$WORKER_NAME.<your-subdomain>.workers.dev/health"
echo "=============================================="
