(function(){const t=document.createElement("link").relList;if(t&&t.supports&&t.supports("modulepreload"))return;for(const s of document.querySelectorAll('link[rel="modulepreload"]'))r(s);new MutationObserver(s=>{for(const o of s)if(o.type==="childList")for(const i of o.addedNodes)i.tagName==="LINK"&&i.rel==="modulepreload"&&r(i)}).observe(document,{childList:!0,subtree:!0});function n(s){const o={};return s.integrity&&(o.integrity=s.integrity),s.referrerPolicy&&(o.referrerPolicy=s.referrerPolicy),s.crossOrigin==="use-credentials"?o.credentials="include":s.crossOrigin==="anonymous"?o.credentials="omit":o.credentials="same-origin",o}function r(s){if(s.ep)return;s.ep=!0;const o=n(s);fetch(s.href,o)}})();const Ve="modulepreload",We=function(e){return"/"+e},be={},Ge=function(t,n,r){if(!n||n.length===0)return t();const s=document.getElementsByTagName("link");return Promise.all(n.map(o=>{if(o=We(o),o in be)return;be[o]=!0;const i=o.endsWith(".css"),l=i?'[rel="stylesheet"]':"";if(!!r)for(let a=s.length-1;a>=0;a--){const f=s[a];if(f.href===o&&(!i||f.rel==="stylesheet"))return}else if(document.querySelector(`link[href="${o}"]${l}`))return;const c=document.createElement("link");if(c.rel=i?"stylesheet":Ve,i||(c.as="script",c.crossOrigin=""),c.href=o,document.head.appendChild(c),i)return new Promise((a,f)=>{c.addEventListener("load",a),c.addEventListener("error",()=>f(new Error(`Unable to preload CSS for ${o}`)))})})).then(()=>t()).catch(o=>{const i=new Event("vite:preloadError",{cancelable:!0});if(i.payload=o,window.dispatchEvent(i),!i.defaultPrevented)throw o})},Xe=(e,t)=>e===t,fe=Symbol("solid-proxy"),z={equals:Xe};let Je=$e;const B=1,ee=2,Se={owned:null,cleanups:null,context:null,owner:null},ie={};var p=null;let le=null,y=null,P=null,I=null,re=0;function Pe(e,t){const n=y,r=p,s=e.length===0,o=t===void 0?r:t,i=s?Se:{owned:null,cleanups:null,context:o?o.context:null,owner:o},l=s?e:()=>e(()=>k(()=>se(i)));p=i,y=null;try{return O(l,!0)}finally{y=n,p=r}}function R(e,t){t=t?Object.assign({},z,t):z;const n={value:e,observers:null,observerSlots:null,comparator:t.equals||void 0},r=s=>(typeof s=="function"&&(s=s(n.value)),Re(n,s));return[Le.bind(n),r]}function ve(e,t,n){const r=me(e,t,!0,B);X(r)}function W(e,t,n){const r=me(e,t,!1,B);X(r)}function S(e,t,n){n=n?Object.assign({},z,n):z;const r=me(e,t,!0,0);return r.observers=null,r.observerSlots=null,r.comparator=n.equals||void 0,X(r),Le.bind(r)}function Ye(e){return e&&typeof e=="object"&&"then"in e}function Qe(e,t,n){let r,s,o;arguments.length===2&&typeof t=="object"||arguments.length===1?(r=!0,s=e,o=t||{}):(r=e,s=t,o=n||{});let i=null,l=ie,u=!1,c="initialValue"in o,a=typeof r=="function"&&S(r);const f=new Set,[d,w]=(o.storage||R)(o.initialValue),[b,m]=R(void 0),[L,$]=R(void 0,{equals:!1}),[N,q]=R(c?"ready":"unresolved");function E(v,x,h,g){return i===v&&(i=null,g!==void 0&&(c=!0),(v===l||x===l)&&o.onHydrated&&queueMicrotask(()=>o.onHydrated(g,{value:x})),l=ie,_(x,h)),x}function _(v,x){O(()=>{x===void 0&&w(()=>v),q(x!==void 0?"errored":c?"ready":"unresolved"),m(x);for(const h of f.keys())h.decrement();f.clear()},!1)}function M(){const v=tt,x=d(),h=b();if(h!==void 0&&!i)throw h;return y&&!y.user&&v&&ve(()=>{L(),i&&(v.resolved||f.has(v)||(v.increment(),f.add(v)))}),x}function U(v=!0){if(v!==!1&&u)return;u=!1;const x=a?a():r;if(x==null||x===!1){E(i,k(d));return}const h=l!==ie?l:k(()=>s(x,{value:d(),refetching:v}));return Ye(h)?(i=h,"value"in h?(h.status==="success"?E(i,h.value,void 0,x):E(i,void 0,void 0,x),h):(u=!0,queueMicrotask(()=>u=!1),O(()=>{q(c?"refreshing":"pending"),$()},!1),h.then(g=>E(h,g,void 0,x),g=>E(h,void 0,je(g),x)))):(E(i,h,void 0,x),h)}return Object.defineProperties(M,{state:{get:()=>N()},error:{get:()=>b()},loading:{get(){const v=N();return v==="pending"||v==="refreshing"}},latest:{get(){if(!c)return M();const v=b();if(v&&!i)throw v;return d()}}}),a?ve(()=>U(!1)):U(!1),[M,{refetch:U,mutate:w}]}function k(e){if(y===null)return e();const t=y;y=null;try{return e()}finally{y=t}}function Ae(e,t,n){const r=Array.isArray(e);let s,o=n&&n.defer;return i=>{let l;if(r){l=Array(e.length);for(let c=0;c<e.length;c++)l[c]=e[c]()}else l=e();if(o){o=!1;return}const u=k(()=>t(l,s,i));return s=l,u}}function Ee(e){return p===null||(p.cleanups===null?p.cleanups=[e]:p.cleanups.push(e)),e}function Ze(){return p}function ze(e,t){const n=p,r=y;p=e,y=null;try{return O(t,!0)}catch(s){we(s)}finally{p=n,y=r}}function et(e){const t=y,n=p;return Promise.resolve().then(()=>{y=t,p=n;let r;return O(e,!1),y=p=null,r?r.done:void 0})}function ke(e,t){const n=Symbol("context");return{id:n,Provider:st(n),defaultValue:e}}function pe(e){return p&&p.context&&p.context[e.id]!==void 0?p.context[e.id]:e.defaultValue}function ye(e){const t=S(e),n=S(()=>de(t()));return n.toArray=()=>{const r=n();return Array.isArray(r)?r:r!=null?[r]:[]},n}let tt;function Le(){if(this.sources&&this.state)if(this.state===B)X(this);else{const e=P;P=null,O(()=>te(this),!1),P=e}if(y){const e=this.observers?this.observers.length:0;y.sources?(y.sources.push(this),y.sourceSlots.push(e)):(y.sources=[this],y.sourceSlots=[e]),this.observers?(this.observers.push(y),this.observerSlots.push(y.sources.length-1)):(this.observers=[y],this.observerSlots=[y.sources.length-1])}return this.value}function Re(e,t,n){let r=e.value;return(!e.comparator||!e.comparator(r,t))&&(e.value=t,e.observers&&e.observers.length&&O(()=>{for(let s=0;s<e.observers.length;s+=1){const o=e.observers[s],i=le&&le.running;i&&le.disposed.has(o),(i?!o.tState:!o.state)&&(o.pure?P.push(o):I.push(o),o.observers&&_e(o)),i||(o.state=B)}if(P.length>1e6)throw P=[],new Error},!1)),t}function X(e){if(!e.fn)return;se(e);const t=p,n=y,r=re;y=p=e,nt(e,e.value,r),y=n,p=t}function nt(e,t,n){let r;try{r=e.fn(t)}catch(s){return e.pure&&(e.state=B,e.owned&&e.owned.forEach(se),e.owned=null),e.updatedAt=n+1,we(s)}(!e.updatedAt||e.updatedAt<=n)&&(e.updatedAt!=null&&"observers"in e?Re(e,r):e.value=r,e.updatedAt=n)}function me(e,t,n,r=B,s){const o={fn:e,state:r,updatedAt:null,owned:null,sources:null,sourceSlots:null,cleanups:null,value:t,owner:p,context:p?p.context:null,pure:n};return p===null||p!==Se&&(p.owned?p.owned.push(o):p.owned=[o]),o}function Oe(e){if(e.state===0)return;if(e.state===ee)return te(e);if(e.suspense&&k(e.suspense.inFallback))return e.suspense.effects.push(e);const t=[e];for(;(e=e.owner)&&(!e.updatedAt||e.updatedAt<re);)e.state&&t.push(e);for(let n=t.length-1;n>=0;n--)if(e=t[n],e.state===B)X(e);else if(e.state===ee){const r=P;P=null,O(()=>te(e,t[0]),!1),P=r}}function O(e,t){if(P)return e();let n=!1;t||(P=[]),I?n=!0:I=[],re++;try{const r=e();return rt(n),r}catch(r){n||(I=null),P=null,we(r)}}function rt(e){if(P&&($e(P),P=null),e)return;const t=I;I=null,t.length&&O(()=>Je(t),!1)}function $e(e){for(let t=0;t<e.length;t++)Oe(e[t])}function te(e,t){e.state=0;for(let n=0;n<e.sources.length;n+=1){const r=e.sources[n];if(r.sources){const s=r.state;s===B?r!==t&&(!r.updatedAt||r.updatedAt<re)&&Oe(r):s===ee&&te(r,t)}}}function _e(e){for(let t=0;t<e.observers.length;t+=1){const n=e.observers[t];n.state||(n.state=ee,n.pure?P.push(n):I.push(n),n.observers&&_e(n))}}function se(e){let t;if(e.sources)for(;e.sources.length;){const n=e.sources.pop(),r=e.sourceSlots.pop(),s=n.observers;if(s&&s.length){const o=s.pop(),i=n.observerSlots.pop();r<s.length&&(o.sourceSlots[i]=r,s[r]=o,n.observerSlots[r]=i)}}if(e.owned){for(t=e.owned.length-1;t>=0;t--)se(e.owned[t]);e.owned=null}if(e.cleanups){for(t=e.cleanups.length-1;t>=0;t--)e.cleanups[t]();e.cleanups=null}e.state=0}function je(e){return e instanceof Error?e:new Error(typeof e=="string"?e:"Unknown error",{cause:e})}function we(e,t=p){throw je(e)}function de(e){if(typeof e=="function"&&!e.length)return de(e());if(Array.isArray(e)){const t=[];for(let n=0;n<e.length;n++){const r=de(e[n]);Array.isArray(r)?t.push.apply(t,r):t.push(r)}return t}return e}function st(e,t){return function(r){let s;return W(()=>s=k(()=>(p.context={...p.context,[e]:r.value},ye(()=>r.children))),void 0),s}}function A(e,t){return k(()=>e(t||{}))}function Q(){return!0}const ot={get(e,t,n){return t===fe?n:e.get(t)},has(e,t){return t===fe?!0:e.has(t)},set:Q,deleteProperty:Q,getOwnPropertyDescriptor(e,t){return{configurable:!0,enumerable:!0,get(){return e.get(t)},set:Q,deleteProperty:Q}},ownKeys(e){return e.keys()}};function ae(e){return(e=typeof e=="function"?e():e)?e:{}}function it(){for(let e=0,t=this.length;e<t;++e){const n=this[e]();if(n!==void 0)return n}}function lt(...e){let t=!1;for(let o=0;o<e.length;o++){const i=e[o];t=t||!!i&&fe in i,e[o]=typeof i=="function"?(t=!0,S(i)):i}if(t)return new Proxy({get(o){for(let i=e.length-1;i>=0;i--){const l=ae(e[i])[o];if(l!==void 0)return l}},has(o){for(let i=e.length-1;i>=0;i--)if(o in ae(e[i]))return!0;return!1},keys(){const o=[];for(let i=0;i<e.length;i++)o.push(...Object.keys(ae(e[i])));return[...new Set(o)]}},ot);const n={},r={},s=new Set;for(let o=e.length-1;o>=0;o--){const i=e[o];if(!i)continue;const l=Object.getOwnPropertyNames(i);for(let u=0,c=l.length;u<c;u++){const a=l[u];if(a==="__proto__"||a==="constructor")continue;const f=Object.getOwnPropertyDescriptor(i,a);if(!s.has(a))f.get?(s.add(a),Object.defineProperty(n,a,{enumerable:!0,configurable:!0,get:it.bind(r[a]=[f.get.bind(i)])})):(f.value!==void 0&&s.add(a),n[a]=f.value);else{const d=r[a];d?f.get?d.push(f.get.bind(i)):f.value!==void 0&&d.push(()=>f.value):n[a]===void 0&&(n[a]=f.value)}}}return n}const at=e=>`Stale read from <${e}>.`;function Te(e){const t=e.keyed,n=S(()=>e.when,void 0,{equals:(r,s)=>t?r===s:!r==!s});return S(()=>{const r=n();if(r){const s=e.children;return typeof s=="function"&&s.length>0?k(()=>s(t?r:()=>{if(!k(n))throw at("Show");return e.when})):s}return e.fallback},void 0,void 0)}function ut(e,t,n){let r=n.length,s=t.length,o=r,i=0,l=0,u=t[s-1].nextSibling,c=null;for(;i<s||l<o;){if(t[i]===n[l]){i++,l++;continue}for(;t[s-1]===n[o-1];)s--,o--;if(s===i){const a=o<r?l?n[l-1].nextSibling:n[o-l]:u;for(;l<o;)e.insertBefore(n[l++],a)}else if(o===l)for(;i<s;)(!c||!c.has(t[i]))&&t[i].remove(),i++;else if(t[i]===n[o-1]&&n[l]===t[s-1]){const a=t[--s].nextSibling;e.insertBefore(n[l++],t[i++].nextSibling),e.insertBefore(n[--o],a),t[s]=n[o]}else{if(!c){c=new Map;let f=l;for(;f<o;)c.set(n[f],f++)}const a=c.get(t[i]);if(a!=null)if(l<a&&a<o){let f=i,d=1,w;for(;++f<s&&f<o&&!((w=c.get(t[f]))==null||w!==a+d);)d++;if(d>a-l){const b=t[i];for(;l<a;)e.insertBefore(n[l++],b)}else e.replaceChild(n[l++],t[i++])}else i++;else t[i++].remove()}}}const xe="_$DX_DELEGATE";function ct(e,t,n,r={}){let s;return Pe(o=>{s=o,t===document?e():he(t,e(),t.firstChild?null:void 0,n)},r.owner),()=>{s(),t.textContent=""}}function D(e,t,n){let r;const s=()=>{const i=document.createElement("template");return i.innerHTML=e,n?i.content.firstChild.firstChild:i.content.firstChild},o=t?()=>k(()=>document.importNode(r||(r=s()),!0)):()=>(r||(r=s())).cloneNode(!0);return o.cloneNode=o,o}function Be(e,t=window.document){const n=t[xe]||(t[xe]=new Set);for(let r=0,s=e.length;r<s;r++){const o=e[r];n.has(o)||(n.add(o),t.addEventListener(o,ft))}}function he(e,t,n,r){if(n!==void 0&&!r&&(r=[]),typeof t!="function")return ne(e,t,r,n);W(s=>ne(e,t(),s,n),r)}function ft(e){const t=`$$${e.type}`;let n=e.composedPath&&e.composedPath()[0]||e.target;for(e.target!==n&&Object.defineProperty(e,"target",{configurable:!0,value:n}),Object.defineProperty(e,"currentTarget",{configurable:!0,get(){return n||document}});n;){const r=n[t];if(r&&!n.disabled){const s=n[`${t}Data`];if(s!==void 0?r.call(n,s,e):r.call(n,e),e.cancelBubble)return}n=n._$host||n.parentNode||n.host}}function ne(e,t,n,r,s){for(;typeof n=="function";)n=n();if(t===n)return n;const o=typeof t,i=r!==void 0;if(e=i&&n[0]&&n[0].parentNode||e,o==="string"||o==="number")if(o==="number"&&(t=t.toString()),i){let l=n[0];l&&l.nodeType===3?l.data=t:l=document.createTextNode(t),n=H(e,n,r,l)}else n!==""&&typeof n=="string"?n=e.firstChild.data=t:n=e.textContent=t;else if(t==null||o==="boolean")n=H(e,n,r);else{if(o==="function")return W(()=>{let l=t();for(;typeof l=="function";)l=l();n=ne(e,l,n,r)}),()=>n;if(Array.isArray(t)){const l=[],u=n&&Array.isArray(n);if(ge(l,t,n,s))return W(()=>n=ne(e,l,n,r,!0)),()=>n;if(l.length===0){if(n=H(e,n,r),i)return n}else u?n.length===0?Ce(e,l,r):ut(e,n,l):(n&&H(e),Ce(e,l));n=l}else if(t.nodeType){if(Array.isArray(n)){if(i)return n=H(e,n,r,t);H(e,n,null,t)}else n==null||n===""||!e.firstChild?e.appendChild(t):e.replaceChild(t,e.firstChild);n=t}}return n}function ge(e,t,n,r){let s=!1;for(let o=0,i=t.length;o<i;o++){let l=t[o],u=n&&n[o],c;if(!(l==null||l===!0||l===!1))if((c=typeof l)=="object"&&l.nodeType)e.push(l);else if(Array.isArray(l))s=ge(e,l,u)||s;else if(c==="function")if(r){for(;typeof l=="function";)l=l();s=ge(e,Array.isArray(l)?l:[l],Array.isArray(u)?u:[u])||s}else e.push(l),s=!0;else{const a=String(l);u&&u.nodeType===3&&u.data===a?e.push(u):e.push(document.createTextNode(a))}}return s}function Ce(e,t,n=null){for(let r=0,s=t.length;r<s;r++)e.insertBefore(t[r],n)}function H(e,t,n,r){if(n===void 0)return e.textContent="";const s=r||document.createTextNode("");if(t.length){let o=!1;for(let i=t.length-1;i>=0;i--){const l=t[i];if(s!==l){const u=l.parentNode===e;!o&&!i?u?e.replaceChild(s,l):e.insertBefore(s,n):u&&l.remove()}else o=!0}}else e.insertBefore(s,n);return[s]}const dt=!1;function ht(e,t,n){return e.addEventListener(t,n),()=>e.removeEventListener(t,n)}function gt([e,t],n,r){return[n?()=>n(e()):e,r?s=>t(r(s)):t]}function pt(e){if(e==="#")return null;try{return document.querySelector(e)}catch{return null}}function yt(e,t){const n=pt(`#${e}`);n?n.scrollIntoView():t&&window.scrollTo(0,0)}function mt(e,t,n,r){let s=!1;const o=l=>typeof l=="string"?{value:l}:l,i=gt(R(o(e()),{equals:(l,u)=>l.value===u.value}),void 0,l=>(!s&&t(l),l));return n&&Ee(n((l=e())=>{s=!0,i[1](o(l)),s=!1})),{signal:i,utils:r}}function wt(e){if(e){if(Array.isArray(e))return{signal:e}}else return{signal:R({value:""})};return e}function bt(){return mt(()=>({value:window.location.pathname+window.location.search+window.location.hash,state:history.state}),({value:e,replace:t,scroll:n,state:r})=>{t?window.history.replaceState(r,"",e):window.history.pushState(r,"",e),yt(window.location.hash.slice(1),n)},e=>ht(window,"popstate",()=>e()),{go:e=>window.history.go(e)})}function vt(){let e=new Set;function t(s){return e.add(s),()=>e.delete(s)}let n=!1;function r(s,o){if(n)return!(n=!1);const i={to:s,options:o,defaultPrevented:!1,preventDefault:()=>i.defaultPrevented=!0};for(const l of e)l.listener({...i,from:l.location,retry:u=>{u&&(n=!0),l.navigate(s,o)}});return!i.defaultPrevented}return{subscribe:t,confirm:r}}const xt=/^(?:[a-z0-9]+:)?\/\//i,Ct=/^\/+|(\/)\/+$/g;function V(e,t=!1){const n=e.replace(Ct,"$1");return n?t||/^[?#]/.test(n)?n:"/"+n:""}function Z(e,t,n){if(xt.test(t))return;const r=V(e),s=n&&V(n);let o="";return!s||t.startsWith("/")?o=r:s.toLowerCase().indexOf(r.toLowerCase())!==0?o=r+s:o=s,(o||"/")+V(t,!o)}function St(e,t){if(e==null)throw new Error(t);return e}function Ne(e,t){return V(e).replace(/\/*(\*.*)?$/g,"")+V(t)}function Pt(e){const t={};return e.searchParams.forEach((n,r)=>{t[r]=n}),t}function At(e,t,n){const[r,s]=e.split("/*",2),o=r.split("/").filter(Boolean),i=o.length;return l=>{const u=l.split("/").filter(Boolean),c=u.length-i;if(c<0||c>0&&s===void 0&&!t)return null;const a={path:i?"":"/",params:{}},f=d=>n===void 0?void 0:n[d];for(let d=0;d<i;d++){const w=o[d],b=u[d],m=w[0]===":",L=m?w.slice(1):w;if(m&&ue(b,f(L)))a.params[L]=b;else if(m||!ue(b,w))return null;a.path+=`/${b}`}if(s){const d=c?u.slice(-c).join("/"):"";if(ue(d,f(s)))a.params[s]=d;else return null}return a}}function ue(e,t){const n=r=>r.localeCompare(e,void 0,{sensitivity:"base"})===0;return t===void 0?!0:typeof t=="string"?n(t):typeof t=="function"?t(e):Array.isArray(t)?t.some(n):t instanceof RegExp?t.test(e):!1}function Et(e){const[t,n]=e.pattern.split("/*",2),r=t.split("/").filter(Boolean);return r.reduce((s,o)=>s+(o.startsWith(":")?2:3),r.length-(n===void 0?0:1))}function qe(e){const t=new Map,n=Ze();return new Proxy({},{get(r,s){return t.has(s)||ze(n,()=>t.set(s,S(()=>e()[s]))),t.get(s)()},getOwnPropertyDescriptor(){return{enumerable:!0,configurable:!0}},ownKeys(){return Reflect.ownKeys(e())}})}function Ie(e){let t=/(\/?\:[^\/]+)\?/.exec(e);if(!t)return[e];let n=e.slice(0,t.index),r=e.slice(t.index+t[0].length);const s=[n,n+=t[1]];for(;t=/^(\/\:[^\/]+)\?/.exec(r);)s.push(n+=t[1]),r=r.slice(t[0].length);return Ie(r).reduce((o,i)=>[...o,...s.map(l=>l+i)],[])}const kt=100,De=ke(),oe=ke(),Me=()=>St(pe(De),"Make sure your app is wrapped in a <Router />");let G;const Ue=()=>G||pe(oe)||Me().base;function Lt(e,t="",n){const{component:r,data:s,children:o}=e,i=!o||Array.isArray(o)&&!o.length,l={key:e,element:r?()=>A(r,{}):()=>{const{element:u}=e;return u===void 0&&n?A(n,{}):u},preload:e.component?r.preload:e.preload,data:s};return He(e.path).reduce((u,c)=>{for(const a of Ie(c)){const f=Ne(t,a),d=i?f:f.split("/*",1)[0];u.push({...l,originalPath:a,pattern:d,matcher:At(d,!i,e.matchFilters)})}return u},[])}function Rt(e,t=0){return{routes:e,score:Et(e[e.length-1])*1e4-t,matcher(n){const r=[];for(let s=e.length-1;s>=0;s--){const o=e[s],i=o.matcher(n);if(!i)return null;r.unshift({...i,route:o})}return r}}}function He(e){return Array.isArray(e)?e:[e]}function Ke(e,t="",n,r=[],s=[]){const o=He(e);for(let i=0,l=o.length;i<l;i++){const u=o[i];if(u&&typeof u=="object"&&u.hasOwnProperty("path")){const c=Lt(u,t,n);for(const a of c){r.push(a);const f=Array.isArray(u.children)&&u.children.length===0;if(u.children&&!f)Ke(u.children,a.pattern,n,r,s);else{const d=Rt([...r],s.length);s.push(d)}r.pop()}}}return r.length?s:s.sort((i,l)=>l.score-i.score)}function Ot(e,t){for(let n=0,r=e.length;n<r;n++){const s=e[n].matcher(t);if(s)return s}return[]}function $t(e,t){const n=new URL("http://sar"),r=S(u=>{const c=e();try{return new URL(c,n)}catch{return console.error(`Invalid path ${c}`),u}},n,{equals:(u,c)=>u.href===c.href}),s=S(()=>r().pathname),o=S(()=>r().search,!0),i=S(()=>r().hash),l=S(()=>"");return{get pathname(){return s()},get search(){return o()},get hash(){return i()},get state(){return t()},get key(){return l()},query:qe(Ae(o,()=>Pt(r())))}}function _t(e,t="",n,r){const{signal:[s,o],utils:i={}}=wt(e),l=i.parsePath||(h=>h),u=i.renderPath||(h=>h),c=i.beforeLeave||vt(),a=Z("",t),f=void 0;if(a===void 0)throw new Error(`${a} is not a valid base path`);a&&!s().value&&o({value:a,replace:!0,scroll:!1});const[d,w]=R(!1),b=async h=>{w(!0);try{await et(h)}finally{w(!1)}},[m,L]=R(s().value),[$,N]=R(s().state),q=$t(m,$),E=[],_={pattern:a,params:{},path:()=>a,outlet:()=>null,resolvePath(h){return Z(a,h)}};if(n)try{G=_,_.data=n({data:void 0,params:{},location:q,navigate:U(_)})}finally{G=void 0}function M(h,g,C){k(()=>{if(typeof g=="number"){g&&(i.go?c.confirm(g,C)&&i.go(g):console.warn("Router integration does not support relative routing"));return}const{replace:J,resolve:Y,scroll:j,state:K}={replace:!1,resolve:!0,scroll:!0,...C},T=Y?h.resolvePath(g):Z("",g);if(T===void 0)throw new Error(`Path '${g}' is not a routable path`);if(E.length>=kt)throw new Error("Too many redirects");const F=m();if((T!==F||K!==$())&&!dt){if(c.confirm(T,C)){const Fe=E.push({value:F,replace:J,scroll:j,state:$()});b(()=>{L(T),N(K)}).then(()=>{E.length===Fe&&v({value:T,state:K})})}}})}function U(h){return h=h||pe(oe)||_,(g,C)=>M(h,g,C)}function v(h){const g=E[0];g&&((h.value!==g.value||h.state!==g.state)&&o({...h,replace:g.replace,scroll:g.scroll}),E.length=0)}W(()=>{const{value:h,state:g}=s();k(()=>{h!==m()&&b(()=>{L(h),N(g)})})});{let h=function(g){if(g.defaultPrevented||g.button!==0||g.metaKey||g.altKey||g.ctrlKey||g.shiftKey)return;const C=g.composedPath().find(F=>F instanceof Node&&F.nodeName.toUpperCase()==="A");if(!C||!C.hasAttribute("link"))return;const J=C.href;if(C.target||!J&&!C.hasAttribute("state"))return;const Y=(C.getAttribute("rel")||"").split(/\s+/);if(C.hasAttribute("download")||Y&&Y.includes("external"))return;const j=new URL(J);if(j.origin!==window.location.origin||a&&j.pathname&&!j.pathname.toLowerCase().startsWith(a.toLowerCase()))return;const K=l(j.pathname+j.search+j.hash),T=C.getAttribute("state");g.preventDefault(),M(_,K,{resolve:!1,replace:C.hasAttribute("replace"),scroll:!C.hasAttribute("noscroll"),state:T&&JSON.parse(T)})};var x=h;Be(["click"]),document.addEventListener("click",h),Ee(()=>document.removeEventListener("click",h))}return{base:_,out:f,location:q,isRouting:d,renderPath:u,parsePath:l,navigatorFactory:U,beforeLeave:c}}function jt(e,t,n,r,s){const{base:o,location:i,navigatorFactory:l}=e,{pattern:u,element:c,preload:a,data:f}=r().route,d=S(()=>r().path);a&&a();const w={parent:t,pattern:u,get child(){return n()},path:d,params:s,data:t.data,outlet:c,resolvePath(b){return Z(o.path(),b,d())}};if(f)try{G=w,w.data=f({data:t.data,params:s,location:i,navigate:l(w)})}finally{G=void 0}return w}const Tt=e=>{const{source:t,url:n,base:r,data:s,out:o}=e,i=t||bt(),l=_t(i,r,s);return A(De.Provider,{value:l,get children(){return e.children}})},Bt=e=>{const t=Me(),n=Ue(),r=ye(()=>e.children),s=S(()=>Ke(r(),Ne(n.pattern,e.base||""),Nt)),o=S(()=>Ot(s(),t.location.pathname)),i=qe(()=>{const a=o(),f={};for(let d=0;d<a.length;d++)Object.assign(f,a[d].params);return f});t.out&&t.out.matches.push(o().map(({route:a,path:f,params:d})=>({originalPath:a.originalPath,pattern:a.pattern,path:f,params:d})));const l=[];let u;const c=S(Ae(o,(a,f,d)=>{let w=f&&a.length===f.length;const b=[];for(let m=0,L=a.length;m<L;m++){const $=f&&f[m],N=a[m];d&&$&&N.route.key===$.route.key?b[m]=d[m]:(w=!1,l[m]&&l[m](),Pe(q=>{l[m]=q,b[m]=jt(t,b[m-1]||n,()=>c()[m+1],()=>o()[m],i)}))}return l.splice(a.length).forEach(m=>m()),d&&w?d:(u=b[0],b)}));return A(Te,{get when(){return c()&&u},keyed:!0,children:a=>A(oe.Provider,{value:a,get children(){return a.outlet()}})})},ce=e=>{const t=ye(()=>e.children);return lt(e,{get children(){return t()}})},Nt=()=>{const e=Ue();return A(Te,{get when(){return e.child},keyed:!0,children:t=>A(oe.Provider,{value:t,get children(){return t.outlet()}})})},qt=D("<h1>Home"),It=D(`<div class="max-w-sm flex flex-col bg-white border shadow-sm rounded-xl dark:bg-slate-900 dark:border-gray-700 dark:shadow-slate-700/[.7]"><img class="w-full h-auto rounded-t-xl"src="https://images.unsplash.com/photo-1680868543815-b8666dba60f7?ixlib=rb-4.0.3&amp;ixid=MnwxMjA3fDB8MHxwaG90by1wYWdlfHx8fGVufDB8fHx8&amp;auto=format&amp;fit=crop&amp;w=2532&amp;q=80"alt="Image Description"><div class="p-4 md:p-5"><h3 class="text-lg font-bold text-gray-800 dark:text-white">Card title</h3><p class="mt-1 text-gray-500 dark:text-gray-400">Some quick example text to build on the card title and make up the bulk of the card's content.</p><a class="mt-2 py-2 px-3 inline-flex justify-center items-center gap-x-2 text-sm font-semibold rounded-lg border border-transparent bg-blue-600 text-white hover:bg-blue-700 disabled:opacity-50 disabled:pointer-events-none dark:focus:outline-none dark:focus:ring-1 dark:focus:ring-gray-600"href=#>Go somewhere`),Dt=D("<span>"),Mt=D("<div><pre>"),Ut=D('<button class="">Refetch'),Ht=async()=>(await fetch("http://8.8.8.8/get-system-info")).json(),Kt=()=>{const[e,{refetch:t}]=Qe(Ht);return[qt(),It(),(()=>{const n=Dt();return he(n,()=>e.loading&&"Loading..."),n})(),(()=>{const n=Mt(),r=n.firstChild;return he(r,()=>JSON.stringify(e(),null,2)),n})(),(()=>{const n=Ut();return n.$$click=()=>t(),n})()]};Be(["click"]);const Ft=D('<div><h1>Settings</h1><button type=button class="py-3 px-4 inline-flex justify-center items-center gap-2 rounded-lg border font-semibold bg-white text-slate-800 hover:bg-slate-50 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2 transition-all text-sm dark:bg-slate-800 dark:text-white dark:focus:ring-offset-gray-800"data-hs-overlay=#hs-basic-modal>Open modal</button><div id=hs-basic-modal class="hs-overlay hidden w-full h-full fixed top-0 left-0 z-[60] overflow-x-hidden overflow-y-auto"><div class="hs-overlay-open:opacity-100 hs-overlay-open:duration-500 opacity-0 transition-all sm:max-w-lg sm:w-full m-3 sm:mx-auto"><div class="flex flex-col bg-white border shadow-sm rounded-xl dark:bg-gray-800 dark:border-gray-700 dark:shadow-slate-700/[.7]"><div class="flex justify-between items-center py-3 px-4 border-b dark:border-gray-700"><h3 class="font-bold text-gray-800 dark:text-white">Modal title</h3><button type=button class="inline-flex flex-shrink-0 justify-center items-center h-8 w-8 rounded-md text-gray-500 hover:text-gray-400 focus:outline-none focus:ring-2 focus:ring-gray-400 focus:ring-offset-2 focus:ring-offset-white transition-all text-sm dark:focus:ring-gray-700 dark:focus:ring-offset-gray-800"data-hs-overlay=#hs-basic-modal><span class=sr-only>Close</span><svg class="w-3.5 h-3.5"width=8 height=8 viewBox="0 0 8 8"fill=none xmlns=http://www.w3.org/2000/svg><path d="M0.258206 1.00652C0.351976 0.912791 0.479126 0.860131 0.611706 0.860131C0.744296 0.860131 0.871447 0.912791 0.965207 1.00652L3.61171 3.65302L6.25822 1.00652C6.30432 0.958771 6.35952 0.920671 6.42052 0.894471C6.48152 0.868271 6.54712 0.854471 6.61352 0.853901C6.67992 0.853321 6.74572 0.865971 6.80722 0.891111C6.86862 0.916251 6.92442 0.953381 6.97142 1.00032C7.01832 1.04727 7.05552 1.1031 7.08062 1.16454C7.10572 1.22599 7.11842 1.29183 7.11782 1.35822C7.11722 1.42461 7.10342 1.49022 7.07722 1.55122C7.05102 1.61222 7.01292 1.6674 6.96522 1.71352L4.31871 4.36002L6.96522 7.00648C7.05632 7.10078 7.10672 7.22708 7.10552 7.35818C7.10442 7.48928 7.05182 7.61468 6.95912 7.70738C6.86642 7.80018 6.74102 7.85268 6.60992 7.85388C6.47882 7.85498 6.35252 7.80458 6.25822 7.71348L3.61171 5.06702L0.965207 7.71348C0.870907 7.80458 0.744606 7.85498 0.613506 7.85388C0.482406 7.85268 0.357007 7.80018 0.264297 7.70738C0.171597 7.61468 0.119017 7.48928 0.117877 7.35818C0.116737 7.22708 0.167126 7.10078 0.258206 7.00648L2.90471 4.36002L0.258206 1.71352C0.164476 1.61976 0.111816 1.4926 0.111816 1.36002C0.111816 1.22744 0.164476 1.10028 0.258206 1.00652Z"fill=currentColor></path></svg></button></div><div class="p-4 overflow-y-auto"><p class="mt-1 text-gray-800 dark:text-gray-400">This is a wider card with supporting text below as a natural lead-in to additional content.</p></div><div class="flex justify-end items-center gap-x-2 py-3 px-4 border-t dark:border-gray-700"><button type=button class="py-3 px-4 inline-flex justify-center items-center gap-2 rounded-md border font-medium bg-white text-gray-700 shadow-sm align-middle hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-offset-white focus:ring-blue-600 transition-all text-sm dark:bg-slate-900 dark:hover:bg-slate-800 dark:border-gray-700 dark:text-gray-400 dark:hover:text-white dark:focus:ring-offset-gray-800"data-hs-overlay=#hs-basic-modal>Close</button><a class="py-3 px-4 inline-flex justify-center items-center gap-2 rounded-md border border-transparent font-semibold bg-blue-500 text-white hover:bg-blue-600 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2 transition-all text-sm dark:focus:ring-offset-gray-800"href=#>Save changes'),Vt=()=>Ft(),Wt=D("<h1>Codes List"),Gt=()=>Wt(),Xt=()=>A(Tt,{get children(){return A(Bt,{get children(){return[A(ce,{path:"/",component:Kt}),A(ce,{path:"/settings",component:Vt})," ",A(ce,{path:"/codes",component:Gt})," "]}})}});Ge(()=>import("./preline-GcweDT_Q.js").then(e=>e.p),__vite__mapDeps([]));const Jt=document.getElementById("root");ct(()=>A(Xt,{}),Jt);
function __vite__mapDeps(indexes) {
  if (!__vite__mapDeps.viteFileDeps) {
    __vite__mapDeps.viteFileDeps = []
  }
  return indexes.map((i) => __vite__mapDeps.viteFileDeps[i])
}