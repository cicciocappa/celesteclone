document.addEventListener('DOMContentLoaded', async () => {
    const hscroll = document.getElementById('hscroll');
    const vscroll = document.getElementById('vscroll');
    const hThumb = hscroll.querySelector('div');
    const vThumb = vscroll.querySelector('div');

    let isDraggingH = false;
    let isDraggingV = false;
    let startX, startY;
    let startLeft, startTop;
    let maxH, maxV;
    let valueH = 0;
    let valueV = 0;
    let app, texture;
    let viewportWidth, viewportHeight;

    function calculateMaxValues() {
        viewportWidth = window.innerWidth - 100 ;
        viewportHeight = window.innerHeight - 24;
        maxH = Math.max(16384 - viewportWidth, 0);
        maxV = Math.max(16384 - viewportHeight, 0);
    }

    function updateThumbs() {
        const hTrackWidth = hscroll.offsetWidth;
        const hThumbWidth = hThumb.offsetWidth;
        const vTrackHeight = vscroll.offsetHeight;
        const vThumbHeight = vThumb.offsetHeight;

        if (maxH > 0 && hTrackWidth > hThumbWidth) {
            const hAvailable = hTrackWidth - hThumbWidth;
            hThumb.style.left = (valueH / maxH) * hAvailable + 'px';
        }

        if (maxV > 0 && vTrackHeight > vThumbHeight) {
            const vAvailable = vTrackHeight - vThumbHeight;
            vThumb.style.top = (valueV / maxV) * vAvailable + 'px';
        }
    }

    // Inizializzazione
    calculateMaxValues();
    updateThumbs();
    await init();

    // Eventi per la scrollbar orizzontale
    hThumb.addEventListener('mousedown', (e) => {
        if (maxH === 0) return;
        isDraggingH = true;
        startX = e.clientX;
        startLeft = parseInt(hThumb.style.left || 0);
        e.preventDefault();
    });

    document.addEventListener('mousemove', (e) => {
        if (!isDraggingH) return;

        const trackWidth = hscroll.offsetWidth;
        const thumbWidth = hThumb.offsetWidth;
        const availableWidth = trackWidth - thumbWidth;

        if (availableWidth <= 0) return;

        const deltaX = e.clientX - startX;
        let newLeft = startLeft + deltaX;
        newLeft = Math.max(0, Math.min(newLeft, availableWidth));

        hThumb.style.left = newLeft + 'px';
        valueH = Math.round((newLeft / availableWidth) * maxH);
    });

    // Eventi per la scrollbar verticale
    vThumb.addEventListener('mousedown', (e) => {
        if (maxV === 0) return;
        isDraggingV = true;
        startY = e.clientY;
        startTop = parseInt(vThumb.style.top || 0);
        e.preventDefault();
    });

    document.addEventListener('mousemove', (e) => {
        if (!isDraggingV) return;

        const trackHeight = vscroll.offsetHeight;
        const thumbHeight = vThumb.offsetHeight;
        const availableHeight = trackHeight - thumbHeight;

        if (availableHeight <= 0) return;

        const deltaY = e.clientY - startY;
        let newTop = startTop + deltaY;
        newTop = Math.max(0, Math.min(newTop, availableHeight));

        vThumb.style.top = newTop + 'px';
        valueV = Math.round((newTop / availableHeight) * maxV);
    });

    document.addEventListener('mouseup', () => {
        isDraggingH = false;
        isDraggingV = false;
    });

    window.addEventListener('resize', () => {
        calculateMaxValues();
        valueH = Math.min(valueH, maxH);
        valueV = Math.min(valueV, maxV);
        updateThumbs();
    });

    async function init() {

        app = new PIXI.Application();

        await app.init({ background: '#1099bb', width: viewportWidth, height: viewportHeight });
        document.querySelector("#canvas").appendChild(app.canvas);
        app.canvas.addEventListener("click", aggiungi);



        const image = await PIXI.Assets.load('test.png');

        const rect = new PIXI.Rectangle(0, 0, 32, 32);

        texture = new PIXI.Texture({ source: image, frame: rect });
        const sprite = new PIXI.Sprite(texture);
        app.stage.addChild(sprite);

        /*
        const container = new Container();
    
        app.stage.addChild(container);
         */
    }
    function aggiungi(ev) {
        console.log(ev);
        const ns = new PIXI.Sprite(texture);
        ns.x = ev.offsetX;
        ns.y = ev.offsetY;
        app.stage.addChild(ns);
    }
});


