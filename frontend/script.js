/* MovieMap front end. Compatible with exportGraphJson() in export.cpp. */
(() => {
  'use strict';
  const $ = id => document.getElementById(id);
  const SVG = 'http://www.w3.org/2000/svg';
  const VISIBLE_COUNT = 30;
  const ANIMATION_MS = 300;
  const state = {data:null, byId:new Map(), adjacency:new Map(), root:null, visible:[], positions:new Map(), visited:new Set(), timer:null, running:false, zoom:1, tx:0, ty:0};
  const nice = n => Number(n || 0).toLocaleString('en-US');
  const trunc = (s, n=18) => s.length > n ? s.slice(0,n-1)+'…' : s;
  const svgEl = (tag, attrs={}) => {const el=document.createElementNS(SVG,tag);Object.entries(attrs).forEach(([k,v])=>el.setAttribute(k,v));return el;};

  function load(raw) {
    if (!raw || !Array.isArray(raw.movies) || !Array.isArray(raw.edges) || !raw.movies.length) throw Error('Expected nonempty movies and edges arrays.');
    const ids = new Set();
    for (const m of raw.movies) {
      if (!Number.isInteger(m.id) || ids.has(m.id) || typeof m.title !== 'string') throw Error('Movies must have unique integer ids and titles.');
      ids.add(m.id);
    }
    for (const e of raw.edges) {
      if (!ids.has(e.from) || !ids.has(e.to) || !Number.isFinite(e.similarity)) throw Error('An edge has invalid endpoints or similarity.');
    }
    stop();
    state.data=raw;state.byId=new Map(raw.movies.map(m=>[m.id,m]));state.adjacency=new Map(raw.movies.map(m=>[m.id,[]]));
    for(const e of raw.edges) state.adjacency.get(e.from).push({to:e.to, similarity:e.similarity});
    for(const edges of state.adjacency.values()) edges.sort((a,b)=>b.similarity-a.similarity);
    $('dataset-badge').textContent=raw.sample?'SAMPLE DATA':'IMDB DATA';
    $('dataset-badge').className='badge'+(raw.sample?'':' real');
    $('count').textContent=nice(raw.movies.length)+' movies';
    $('search').value='';
    $('results').replaceChildren();
    choose(state.byId.has(raw.initialRoot)?raw.initialRoot:raw.movies[0].id);
  }

  function pickNeighborhood(root, cap) {
    const provided = state.data.traversals?.[root];
    if(provided) {
      const selected=[root], seen=new Set(selected);
      const bfs=provided.bfs||[], dfs=provided.dfs||[];
      for(let i=0; selected.length<cap && i<Math.max(bfs.length,dfs.length);i++) {
        for(const id of [bfs[i],dfs[i]]) {
          if(id!==undefined && !seen.has(id) && state.byId.has(id)) {seen.add(id);selected.push(id);if(selected.length>=cap)break;}
        }
      }
      return selected;
    }
    const seen=new Set([root]), list=[root];
    for(let i=0;i<list.length && list.length<cap;i++){
      for(const e of state.adjacency.get(list[i])||[]){
        if(!seen.has(e.to)){seen.add(e.to);list.push(e.to);if(list.length>=cap)break;}
      }
    }
    return list;
  }

  function choose(id) {
    stop();
    state.root=id;
    state.visited.clear();
    state.visible=pickNeighborhood(id, VISIBLE_COUNT);
    state.zoom=1;state.tx=0;state.ty=0;
    const m=state.byId.get(id);
    $('movie-title').textContent=m.title;
    $('movie-meta').textContent=[m.year||'Year unknown',m.rating?m.rating+' ★':'Unrated',m.runtime?m.runtime+' min':''].filter(Boolean).join('  ·  ');
    $('movie-genres').replaceChildren(...(m.genres||[]).map(g=>{const el=document.createElement('span');el.textContent=g;return el;}));
    $('movie-details').textContent=`IMDb: ${m.imdbId||'Not provided'}  ·  ${nice(m.votes)} votes`;
    $('stat-time').textContent='—';
    $('stat-sim').textContent='—';
    $('progress').textContent='Choose an algorithm to animate its traversal.';
    render();
  }

  function valueDifference(m, root, attr){
    const span={rating:5,year:50,runtime:120,votes:Math.max(1,root.votes||1)}[attr]||1;
    return Math.min(1,Math.abs((m[attr]||0)-(root[attr]||0))/span);
  }

  function colorFor(id) {
    if(id===state.root)return '#b7a1ff';
    const mode=$('color').value, root=state.byId.get(state.root), m=state.byId.get(id);
    let closeness;
    if(mode==='similarity') {
      const edge=(state.adjacency.get(state.root)||[]).find(e=>e.to===id);
      closeness=edge?edge.similarity:0.25;
    } else {
      closeness=1-valueDifference(m,root,mode);
    }
    const hue=205+Math.round(75*closeness);
    const light=43+Math.round(20*closeness);
    return `hsl(${hue}, 75%, ${light}%)`;
  }

  function layout() {
    const ids=state.visible;
    state.positions=new Map([[state.root,{x:450,y:270}]]);
    const direct=new Set((state.adjacency.get(state.root)||[]).map(e=>e.to));
    const near=ids.slice(1).filter(x=>direct.has(x));
    const far=ids.slice(1).filter(x=>!direct.has(x));
    near.forEach((id,i)=>{
      const a=i*2*Math.PI/Math.max(near.length,1)-Math.PI/2;
      state.positions.set(id,{x:450+155*Math.cos(a),y:270+155*Math.sin(a)});
    });
    far.forEach((id,i)=>{
      const a=i*2*Math.PI/Math.max(far.length,1)-Math.PI/2+.15;
      state.positions.set(id,{x:450+245*Math.cos(a),y:270+210*Math.sin(a)});
    });
  }

  function render() {
    layout();
    const visible=new Set(state.visible),links=$('links'),nodes=$('nodes');
    links.replaceChildren();
    nodes.replaceChildren();
    const linkSeen=new Set();
    for(const from of state.visible){
      for(const e of state.adjacency.get(from)||[]){
        if(!visible.has(e.to)||e.to===from)continue;
        const key=[from,e.to].sort((a,b)=>a-b).join(':');
        if(linkSeen.has(key))continue;
        linkSeen.add(key);
        const a=state.positions.get(from),b=state.positions.get(e.to);
        const line=svgEl('line',{x1:a.x,y1:a.y,x2:b.x,y2:b.y,class:'edge'+(from===state.root||e.to===state.root?' highlight':'')});
        const tip=svgEl('title');
        tip.textContent=`Similarity: ${(e.similarity*100).toFixed(1)}%`;
        line.append(tip);
        links.append(line);
      }
    }
    for(const id of state.visible){
      const m=state.byId.get(id),p=state.positions.get(id),root=id===state.root,visited=state.visited.has(id);
      const group=svgEl('g',{class:'node'+(root?' root':'')+(visited?' visited':''),transform:`translate(${p.x} ${p.y})`,tabindex:0,role:'button','aria-label':'Explore '+m.title});
      const circle=svgEl('circle',{r:root?23:visited?17:13,fill:visited&&!root?'#49d1b9':colorFor(id)});
      const label=svgEl('text',{y:root?39:28});
      label.textContent=trunc(m.title,root?24:17);
      const title=svgEl('title');
      title.textContent=`${m.title} (${m.year}), rating ${m.rating}`;
      group.append(circle,label,title);
      group.addEventListener('click',()=>choose(id));
      group.addEventListener('keydown',e=>{if(e.key==='Enter'||e.key===' '){e.preventDefault();choose(id);}});
      nodes.append(group);
    }
    $('viewport').setAttribute('transform',`translate(${state.tx} ${state.ty}) scale(${state.zoom})`);
  }

  function traverse(kind) {
    const exported=state.data.traversals?.[state.root];
    if(exported) return {order:[state.root,...(exported[kind.toLowerCase()]||[])],source:'C++'};
    const visible=new Set(state.visible), discovered=new Set([state.root]), frontier=[state.root], order=[];
    if(kind==='BFS') {
      let head=0;
      while(head<frontier.length && order.length<51) {
        const id=frontier[head++];
        order.push(id);
        for(const e of state.adjacency.get(id)||[]) {
          if(visible.has(e.to) && !discovered.has(e.to)) {discovered.add(e.to);frontier.push(e.to);}
        }
      }
    } else {
      const visited=new Set();
      while(frontier.length && order.length<51) {
        const id=frontier.pop();
        if(visited.has(id))continue;
        visited.add(id);
        order.push(id);
        const neighbors=state.adjacency.get(id)||[];
        for(let i=neighbors.length-1;i>=0;i--) {
          const to=neighbors[i].to;
          if(visible.has(to) && !visited.has(to))frontier.push(to);
        }
      }
    }
    return {order,source:'sample JS'};
  }

  
  function measureSampleSearch(kind) {
    let checksum=0;
    for(let i=0;i<100;i++) checksum+=traverse(kind).order.length;
    const start=performance.now();
    let runs=0;
    let elapsed=0;
    do {
      for(let i=0;i<200;i++) checksum+=traverse(kind).order.length;
      runs+=200;
      elapsed=performance.now()-start;
    } while(elapsed<80 && runs<100000);
    // Preserve observed work so optimized runtimes cannot discard the calls.
    if(checksum===-1) throw Error('Unreachable search checksum');
    return elapsed*1000/runs;
  }

  function formatMicroseconds(us, label) {
    if(!Number.isFinite(us) || us<0) return 'Not measured';
    if(us>0 && us<0.001) return '<0.001 µs '+label;
    return us.toFixed(3)+' µs '+label;
  }

  function stop(){
    if(state.timer!==null)clearTimeout(state.timer);
    state.timer=null;
    state.running=false;
    $('bfs').disabled=false;
    $('dfs').disabled=false;
  }

  function animate(kind){
    stop();
    state.visited.clear();
    render();
    const {order,source}=traverse(kind);
   
    const sampleAverage=order.length<=1?0:order.slice(1).reduce((sum,id)=>{
      const edge=(state.adjacency.get(state.root)||[]).find(e=>e.to===id);
      return sum+(edge?edge.similarity:0);
    },0)/(order.length-1);
   
    const benchmarks=state.data.benchmarks;
    const bench=Array.isArray(benchmarks)
      ? benchmarks[state.root]?.[kind.toLowerCase()]
      : (state.root===state.data.benchmarkRoot ? benchmarks?.[kind.toLowerCase()] : null);
    const visible=new Set(state.visible);
    const shown=order.filter(id=>visible.has(id));
    state.running=true;
    $('bfs').disabled=true;
    $('dfs').disabled=true;
    
    const us = Number(bench?.timeMicroseconds);
    $('stat-time').textContent = bench && Number.isFinite(us) && us>=0
      ? formatMicroseconds(us,'(C++)')
      : (state.data.sample ? formatMicroseconds(measureSampleSearch(kind),'(JS demo)') : 'Not available');
    $('stat-sim').textContent=bench
      ? (100*bench.averageSimilarity).toFixed(1)+'% (C++)'
      : (state.data.sample ? (100*sampleAverage).toFixed(1)+'% (sample)' : 'Not measured');
    let i=0;
    const step=()=>{
      if(!state.running)return;
      if(i>=shown.length){
        $('progress').textContent=`${kind} complete · ${shown.length} visible of ${order.length} traversal nodes shown`;
        stop();
        return;
      }
      const id=shown[i++];
      state.visited.add(id);
      $('progress').textContent=`${kind} (${source}) · ${state.byId.get(id).title} (${i}/${shown.length} displayed)`;
      render();
      state.timer=setTimeout(step, ANIMATION_MS);
    };
    step();
  }

  $('bfs').addEventListener('click',()=>animate('BFS'));
  $('dfs').addEventListener('click',()=>animate('DFS'));
  $('color').addEventListener('change',render);
  $('search').addEventListener('input',()=>{
    const q=$('search').value.trim().toLowerCase();
    $('results').replaceChildren();
    if(!q)return;
    const matches=state.data.movies.filter(m=>m.title.toLowerCase().includes(q)).sort((a,b)=>b.votes-a.votes).slice(0,8);
    for(const m of matches){
      const btn=document.createElement('button');
      btn.className='result';
      btn.type='button';
      btn.textContent=m.title;
      const y=document.createElement('small');
      y.textContent=`${m.year} · ${nice(m.votes)} votes`;
      btn.append(y);
      btn.addEventListener('click',()=>{$('search').value='';$('results').replaceChildren();choose(m.id);});
      $('results').append(btn);
    }
    if(!matches.length)$('results').textContent='No matching movies in this dataset.';
  });

  let drag=null;
  $('graph').addEventListener('pointerdown',e=>{if(e.target.closest('.node'))return;drag={x:e.clientX,y:e.clientY,tx:state.tx,ty:state.ty};$('graph').setPointerCapture(e.pointerId);});
  $('graph').addEventListener('pointermove',e=>{if(!drag)return;const box=$('graph').getBoundingClientRect();state.tx=drag.tx+(e.clientX-drag.x)*900/box.width;state.ty=drag.ty+(e.clientY-drag.y)*540/box.height;$('viewport').setAttribute('transform',`translate(${state.tx} ${state.ty}) scale(${state.zoom})`);});
  $('graph').addEventListener('pointerup',()=>drag=null);
  $('graph').addEventListener('pointercancel',()=>drag=null);
  $('graph').addEventListener('wheel',e=>{e.preventDefault();state.zoom=Math.max(.6,Math.min(2.7,state.zoom*(e.deltaY<0?1.1:.9)));$('viewport').setAttribute('transform',`translate(${state.tx} ${state.ty}) scale(${state.zoom})`);},{passive:false});

  async function init(){
    try {
      if (window.MOVIEMAP_DATA) {
        load(window.MOVIEMAP_DATA);
        return;
      }
      const response = await fetch('graph.json', {cache:'no-store'});
      if (!response.ok) throw Error('No exported graph');
      load(await response.json());
    } catch (error) {
      $('count').textContent = 'Data unavailable';
      $('progress').textContent = 'Could not load IMDb graph. Check graph-data.js.';
      console.error('MovieMap failed to load graph:', error);
    }
  }

  init();
})();
