export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    if (url.pathname === "/health") {
      return Response.json({ ok: true, service: "NSL Greenhouse", time: new Date().toISOString() });
    }
    return new Response("NSL Greenhouse Worker is running.", {
      headers: { "content-type": "text/plain; charset=UTF-8" }
    });
  }
};
