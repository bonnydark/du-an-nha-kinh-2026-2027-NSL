#!/usr/bin/env bash
set -e
command -v node >/dev/null || { echo "Can cai Node.js truoc."; exit 1; }
command -v npm >/dev/null || { echo "Can npm."; exit 1; }
npm exec --yes wrangler@latest login
cd worker
npm exec --yes wrangler@latest deploy
