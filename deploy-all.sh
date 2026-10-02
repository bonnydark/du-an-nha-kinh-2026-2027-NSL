#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
command -v node >/dev/null || { echo "❌ Chưa có Node.js"; exit 1; }
command -v npm >/dev/null || { echo "❌ Chưa có npm"; exit 1; }
echo "🌱 NSL Greenhouse Cloud Deploy"
echo "1/4 Cloudflare login..."
cd "$ROOT/worker"
npm exec --yes wrangler@latest login
echo "2/4 Nhập Supabase URL (vd https://xxxx.supabase.co)"
read -r -p "SUPABASE_URL: " SUPABASE_URL
echo "3/4 Nhập Supabase anon/publishable key"
read -r -s -p "SUPABASE_ANON_KEY: " SUPABASE_ANON_KEY
echo
test -n "$SUPABASE_URL" && test -n "$SUPABASE_ANON_KEY" || { echo "❌ Thiếu Supabase config"; exit 1; }
echo "4/4 Cấu hình secrets + deploy Worker..."
printf '%s' "$SUPABASE_URL" | npm exec --yes wrangler@latest secret put SUPABASE_URL
printf '%s' "$SUPABASE_ANON_KEY" | npm exec --yes wrangler@latest secret put SUPABASE_ANON_KEY
npm exec --yes wrangler@latest deploy
echo
echo "✅ Deploy xong. URL:"
npm exec --yes wrangler@latest deployments list 2>/dev/null | head -5 || true
echo "👉 https://nsl-greenhouse.xzort.workers.dev"
echo "⚠️ Chạy database/schema.sql trong Supabase SQL Editor một lần trước khi test."
