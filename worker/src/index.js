const HTML = String.raw`<!doctype html><html lang="vi"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>NSL Greenhouse</title><style>body{margin:0;background:#0b1020;color:#edf2f7;font:15px system-ui}main{max-width:1100px;margin:auto;padding:24px}.top{display:flex;justify-content:space-between}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(170px,1fr));gap:12px;margin:18px 0}.card,.panel{background:#141c2f;border:1px solid #2b3750;border-radius:16px;padding:16px}.v{font-size:28px;font-weight:800;margin-top:6px}.ok{color:#5ee58b}.bad{color:#ff7777}button{padding:10px 14px;border:0;border-radius:9px;margin:4px;background:#4ade80;font-weight:800}button.off{background:#344054;color:white}input{padding:9px;background:#0d1425;color:white;border:1px solid #35415d;border-radius:8px;max-width:130px}.controls{display:flex;flex-wrap:wrap;gap:12px}canvas{width:100%;max-height:350px}</style></head><body><main><div class="top"><div><h1>🌱 NSL Greenhouse</h1><span>Cloudflare Worker + Supabase</span></div><button onclick="logout()">Đăng xuất</button></div><div class="grid"><div class="card">🌱 Đất A<div id="a" class="v">--</div></div><div class="card">🌱 Đất B<div id="b" class="v">--</div></div><div class="card">🌡️ Nhiệt độ<div id="t" class="v">--</div></div><div class="card">💧 Độ ẩm<div id="h" class="v">--</div></div><div class="card">💨 Khói<div id="s" class="v">--</div></div><div class="card">🌧️ Mưa<div id="r" class="v">--</div></div><div class="card">💦 Bơm<div id="p" class="v">--</div></div><div class="card">🌀 Quạt<div id="f" class="v">--</div></div><div class="card">📡 ESP32<div id="esp" class="v">--</div></div><div class="card">🚨 Cảnh báo<div id="al" class="v">--</div></div></div><section class="panel"><h2>📈 Lịch sử</h2><canvas id="chart"></canvas></section><section class="panel"><h2>🎛️ Điều khiển</h2><button onclick="cmd('pump',true)">💦 Bật bơm</button><button class="off" onclick="cmd('pump',false)">Tắt bơm</button><button onclick="cmd('fan',true)">🌀 Bật quạt</button><button class="off" onclick="cmd('fan',false)">Tắt quạt</button></section><section class="panel"><h2>⚙️ Ngưỡng</h2><div class="controls"><label>Đất khô <input id="dry" type="number">%</label><label>Nhiệt độ <input id="ht" type="number" step=".1">°C</label><label>Khói <input id="st" type="number"></label><button onclick="save()">Lưu</button></div><p id="msg"></p></section></main><script src="https://cdn.jsdelivr.net/npm/chart.js"></script><script>
async function api(path,opt={}){const r=await fetch('/api'+path,{credentials:'include',...opt});if(r.status===401){location.href='/login'}if(!r.ok)throw Error(await r.text());return r.json()}
async function load(){try{const d=await api('/latest');if(d){a.textContent=d.soil_a+'%';b.textContent=d.soil_b+'%';t.textContent=(d.temperature??'--')+' °C';h.textContent=(d.humidity??'--')+' %';s.textContent=d.smoke;r.textContent=d.raining?'ĐANG MƯA':'KHÔ';p.textContent=d.pump?'BẬT':'TẮT';f.textContent=d.fan?'BẬT':'TẮT';al.textContent=d.alarm?'NGUY HIỂM':'BÌNH THƯỜNG';al.className='v '+(d.alarm?'bad':'ok');esp.textContent=(Date.now()-new Date(d.created_at).getTime()<30000)?'ONLINE':'OFFLINE'}}catch(e){}}async function history(){const rows=await api('/history');new Chart(chart,{type:'line',data:{labels:rows.map(x=>new Date(x.created_at).toLocaleTimeString()),datasets:[{label:'Đất A %',data:rows.map(x=>x.soil_a)},{label:'Đất B %',data:rows.map(x=>x.soil_b)},{label:'Nhiệt độ °C',data:rows.map(x=>x.temperature)}]}})}async function settings(){const s=await api('/settings');dry.value=s.soil_dry_threshold;ht.value=s.high_temperature;st.value=s.smoke_threshold}async function save(){await api('/settings',{method:'PUT',headers:{'content-type':'application/json'},body:JSON.stringify({soil_dry_threshold:+dry.value,high_temperature:+ht.value,smoke_threshold:+st.value})});msg.textContent='✓ Đã lưu'}async function cmd(device,on){await api('/command',{method:'POST',headers:{'content-type':'application/json'},body:JSON.stringify({device,on})});load()}async function logout(){await api('/logout',{method:'POST'});location.href='/login'}load();history();settings();setInterval(load,5000);
</script></body></html>`;

async function supa(env,path,method="GET",body){
  const r=await fetch(env.SUPABASE_URL+"/rest/v1/"+path,{method,headers:{"apikey":env.SUPABASE_ANON_KEY,"Authorization":"Bearer "+env.SUPABASE_ANON_KEY,"Content-Type":"application/json","Prefer":"return=representation"},body:body?JSON.stringify(body):undefined});
  const text=await r.text(); if(!r.ok) throw new Error(text); return text?JSON.parse(text):null;
}
function cors(r){const h=new Headers(r.headers);h.set("Access-Control-Allow-Origin","*");h.set("Access-Control-Allow-Headers","content-type, authorization");return new Response(r.body,{status:r.status,headers:h})}
async function api(request,env,path){
  try{
    if(path==="latest") return Response.json((await supa(env,"greenhouse_readings?select=*&order=created_at.desc&limit=1"))[0]||null);
    if(path==="history") return Response.json(await supa(env,"greenhouse_readings?select=created_at,soil_a,soil_b,temperature&order=created_at.desc&limit=60"));
    if(path==="settings"){
      if(request.method==="GET") return Response.json((await supa(env,"greenhouse_settings?select=*&id=eq.1"))[0]);
      const b=await request.json();return Response.json((await supa(env,"greenhouse_settings?id=eq.1","PATCH",b))[0]);
    }
    if(path==="command"&&request.method==="POST"){
      const b=await request.json();if(!["pump","fan"].includes(b.device)||typeof b.on!=="boolean")return Response.json({error:"invalid command"},{status:400});
      const patch=b.device==="pump"?{auto_mode:false,force_pump:b.on}:{auto_mode:false,force_fan:b.on};
      return Response.json((await supa(env,"greenhouse_settings?id=eq.1","PATCH",patch))[0]);
    }
    if(path==="logout")return Response.json({ok:true});
    return Response.json({error:"not found"},{status:404});
  }catch(e){return Response.json({error:e.message},{status:500})}
}
export default {async fetch(request,env){
 const u=new URL(request.url);
 if(u.pathname==="/health")return Response.json({ok:true,service:"NSL Greenhouse",time:new Date().toISOString()});
 if(u.pathname==="/api/"||u.pathname.startsWith("/api/"))return api(request,env,u.pathname.slice(5));
 if(u.pathname==="/")return new Response(HTML,{headers:{"content-type":"text/html;charset=UTF-8"}});
 if(u.pathname==="/login")return new Response(HTML,{headers:{"content-type":"text/html;charset=UTF-8"}});
 return new Response("NSL Greenhouse Worker",{status:404});
}};