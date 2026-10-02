#!/usr/bin/env bash
set -euo pipefail
echo "== NSL Greenhouse: Cloudflare + Supabase deploy =="
command -v git >/dev/null || { echo "Thiếu git"; exit 1; }
command -v node >/dev/null || { echo "Thiếu Node.js"; exit 1; }
command -v npm >/dev/null || { echo "Thiếu npm"; exit 1; }
command -v npx >/dev/null || { echo "Thiếu npx"; exit 1; }

echo "[1/4] Installing Cloudflare Wrangler..."
npm install --no-save wrangler@latest

echo "[2/4] Login Cloudflare (nếu chưa đăng nhập)..."
npx wrangler login

echo "[3/4] Deploying web/ to Cloudflare Pages..."
echo "Nếu project nsl-greenhouse chưa tồn tại, Wrangler sẽ yêu cầu tạo project."
npx wrangler pages deploy web --project-name nsl-greenhouse

echo "[4/4] Done."
echo "Mở URL Cloudflare Pages được in ở trên, sau đó vào /setup.html để cấu hình Supabase."
