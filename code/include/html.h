
const char * html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1" />
<style>
body {
color: white;
background-color: black;
}
container {
height: 100vh;
display: flex;
flex-direction: column;
justify-content: center;
align-items: center;
}
container div {
margin-bottom: 20px;
}
#dimmer {
width: 80vw;
max-width: 400px;
}
#hue {
width: 80vw;
max-width: 400px;
}
</style>
</head>
<body>
<container>
<div>
<h2>PIXEL LAMP</h2>
</div>
<div>
<label>MODE</label>
<select name="mode" id="mode" onchange="setmode(event)"></select>
<button onclick="nextmode()">Next</button>
</div>
<div>
<label>DIM </label>
<input id="dimmer" type="range" min="0" max="100" value="50" step="5" />
</div>
<div>
<label>Color </label>
<input id="hue" type="range" min="0" max="255" value="0" step="5" />
</div>
<div>
<label>Invert </label>
<input id="invert" type="checkbox" />
</div>
</container>
<script>
const set = (m, v) =>
fetch(`/set?${m}=${v}`)
.then((res) => res.text())
.then(console.log);
const modes = [
"full",
"half",
"square",
"triangle",
"cross",
"lines",
"hatch",
"hatch2",
"arrow",
"line",
"scroll_lines",
"waves",
"stripes",
"rain",
"autom",
"count",
];
const mode = document.getElementById("mode");
const dimmer = document.getElementById("dimmer");
const hue = document.getElementById("hue");
const invert = document.getElementById("invert");
mode.onchange = (ev) => set("mode", ev.target.value);
dimmer.oninput = (ev) => set("bright", ev.target.value);
hue.oninput = (ev) => set("hue", ev.target.value);
invert.onchange = (ev) => set("invert", ev.target.checked ? 1 : 0);
modes.forEach((m) => {
const el = document.createElement("option");
el.value = m;
el.textContent = m;
mode.appendChild(el);
});
function nextmode() {
let s = mode.selectedIndex + 1;
if (s >= mode.options.length) s = 0;
mode.selectedIndex = s;
set("mode", mode.value);
}
</script>
</body>
</html>
)rawliteral";