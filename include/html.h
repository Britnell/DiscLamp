const char * html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <script defer src="https://cdn.jsdelivr.net/npm/alpinejs@3.x.x/dist/cdn.min.js"></script>
    <style>
      body {
        color: white;
        background-color: black;
      }
      main {
        height: 100vh;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
      }
      main div {
        margin-bottom: 20px;
      }
      #b, #h {
        width: 80vw;
        max-width: 400px;
      }
    </style>
  <script>
      document.addEventListener('alpine:init', () => {
        Alpine.data('lamp', () => ({
          modes: ['full','half','square','triangle','cross','lines','hatch','hatch2','arrow','line','scroll_lines','waves','stripes','rain','hues','trif','autom','clock'],
          send(name, val) {
            fetch(`/set?${name}=${val}`).then(r => r.text()).then(console.log);
          },
          handle(ev) {
            const el = ev.target;
            if (!el.name || el.name === 'cip') return;
            const v = el.type === 'checkbox' ? (el.checked ? 1 : 0) : el.value;
            this.send(el.name, v);
          },
          next() {
            const sel = this.$refs.mode;
            sel.selectedIndex = (sel.selectedIndex + 1) % sel.options.length;
            this.send('m', sel.value);
          },
          cmd(ev) {
            const v = ev.target.elements.cip.value;
            this.send(v[0], v.slice(1));
          },
        }));
      });
    </script>
  </head>
  <body x-data="lamp" @input="handle($event)">
    <main>
      <div><h2>PIXEL LAMP</h2></div>
      <div>
        <label>MODE</label>
        <select name="m" x-ref="mode">
          <template x-for="m in modes" :key="m">
            <option :value="m" x-text="m"></option>
          </template>
        </select>
        <button @click="next()">Next</button>
      </div>
      <div>
        <label>DIM </label>
        <input id="b" name="b" type="range" min="0" max="100" value="50" step="5" />
      </div>
      <div>
        <label>Color </label>
        <input id="h" name="h" type="range" min="0" max="255" value="0" step="5" />
      </div>
      <div>
        <label>
          <input name="i" type="checkbox" />
          Invert
        </label>
      </div>
      <div>
        <form @submit.prevent="cmd($event)">
          <label>CMD</label>
          <input name="cip" type="text" />
          <button>>></button>
        </form>
      </div>
    </main>
  </body>
</html>

)rawliteral";