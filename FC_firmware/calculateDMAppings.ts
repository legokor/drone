import mappings from './dmaMappings.json' with { type: 'json' };
import timers from './timers.json' with { type: 'json' };

const fixed = [
    // 'SPI1_RX', 'SPI1_TX', // LoRa
    'SPI2_RX', 'SPI2_TX', // IMU, barometer
    // 'ADC1', // VBAT, IBAT
    'UART5_RX', // SBUS (rc)
    // 'UART4_RX', 'UART4_TX', // GPS
    'USART1_RX', 'USART1_TX', // debug serial
    'I2C1_RX', 'I2C1_TX', // magnetometer
];

const colorPat = /^(SPI.*|US?ART.*|I2C.*|TIM\d+_CH\d+|ADC.*)$/;

const peripherials = new Set<string>();
const colors = new Map<string, [string, string]>();

const getPeripherialName = (p: string) => p.replaceAll(/\d+/g, 'x').replace(/_RX|_TX/, '_xX');

for (const m of [...mappings.dma1.flatMap(a => a), ...mappings.dma2.flatMap(a => a)]) {
    if (colorPat.test(m))
        peripherials.add(getPeripherialName(m));
}
{
    const ps = [...peripherials].sort();

    for (let i = 0; i < ps.length; i++) {
        const h = 360 * i / peripherials.size;
        colors.set(ps[i], [`hsl(${h} 50 50)`,
        `hsl(${h - 180} 100 50)`]);
        //'black']);
    }

    // console.log(ps);
    // console.log(ps.length);
}

const dmaMappingsToHtmlTable = (ms: typeof mappings) => {
    const inner = (m: typeof mappings[keyof typeof mappings]) => `
<div class="table">
${m.map((col, ci) => `
<div class="tcol">
<div class="tcol">
<div class="thead">
<span>Stream ${ci}</span>
</div>

${col.toSorted().map(p => {
        const [b, t] = colors.get(getPeripherialName(p)) ?? ['unset', 'unset'];
        return `<div class="item" style="background-color: ${b}; color: ${t};"><span>${p}</span></div>`;
    }).join('\n')}
</div>
${fixed.some(v => col.includes(v)) ? '<span class="fixed">FIXED</span>' : ''}
</div>
`).join('\n')}
</div>`;

    return `<html>
<head><title>Drone DMA mappings </title></head>
<body>
<style>

* {
    margin: 0;
    padding: 0;
    font-family: 'Roboto';
}

.thead {
    font-weight: bold;
    font-size: large;
}

h1 {
    position: absolute;
    width: max-content;
    text-align: center;
    padding-top: 1em;
    margin: 0 50%;
    left: 0;
}

body, main {
    width: 100vw;
    height: 100vh;
}

main {
    display: flex;
    flex-direction: column;
    row-gap: 1em;
    align-items: center;
    justify-content: center;
}

.table {
    display: flex;
    gap: 0.5em;
}

.tcol {
    display: flex;
    flex-direction: column;
    gap: 0.2em;
}

.fixed {
    color: #ff6961;
    font-weight: bold;
    font-size: large;
}

.item {
    padding: 0.25em;

    display: flex;
    align-content: center;
    justify-content: center;
}

.hide {
    display: none;
}
</style>

<h1>DMA Mapping</h1>

<main>

<div>
<h2>DMA1</h2>
${inner(ms.dma1)}
</div>

<div>
<h2>DMA2</h2>
${inner(ms.dma2)}
</div>

</main>
<input type="checkbox" id="irrbtn" checked/>
<label for="irrbtn">Show irrelevant</label>
<script>
const chk = document.getElementById('irrbtn');
chk.onclick = () => {
    for (const i of document.querySelectorAll('.item')) {
        if (!${colorPat}.test(i.innerText))
            i.classList.toggle('hide');
    }
};

chk.click();
</script>

</body>
</html>`;
};

console.log(dmaMappingsToHtmlTable(mappings));
