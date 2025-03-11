document.addEventListener('DOMContentLoaded', async () => {
    const hscroll = document.getElementById('hscroll');
    const vscroll = document.getElementById('vscroll');
    const hThumb = hscroll.querySelector('div');
    const vThumb = vscroll.querySelector('div');
    const spritePicker = document.getElementById('currsprite');
    const tintPicker = document.getElementById('currtint');
    const toolboox = document.getElementById('toolbox');
    let tool = "";
    let oldtool = "";
    let scalaCorrenteX = 1;
    let scalaCorrenteY = 1;
    let rotazioneCorrente = 0;


    const NUM_ATLAS = 1;

    const atlas = [
        // atlas 0
        [[0, 0, 16, 16], [16, 0, 16, 16], [32, 0, 16, 16]]
    ];
    const grid = [];
    const images = [];
    let isDraggingH = false;
    let isDraggingV = false;
    let startX, startY;
    let startLeft, startTop;
    let maxH, maxV;
    let valueH = 0;
    let valueV = 0;
    let app, texture, container, preview, previewSprite;
    let viewportWidth, viewportHeight;
    let currAtlas = 0;


    function calculateMaxValues() {
        viewportWidth = window.innerWidth - 100;
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

    // Gestione Eventi
    toolboox.addEventListener('click', (e) => {
        const target = e.target.closest("[data-cmd]"); // Cerca l'elemento più vicino con data-cmd
        if (target) {
            const cmd = target.getAttribute("data-cmd");
            switch (cmd) {
                case "layers":
                    alert("todo: mostro selezione layers");
                    break;
                case "sprite":
                    alert("todo: mostro selezione sprite");
                    break;
                default:
                    activateTool(cmd);
                    break;
            }
        }

    });
    spritePicker.addEventListener('click', (e) => { });

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
        container.x = -valueH;
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
        container.y = -valueV;
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
        app.canvas.addEventListener("mousedown", canvasDown);
        app.canvas.addEventListener("mousemove", canvasMove);
        app.canvas.addEventListener("mouseup", canvasUp);
        app.canvas.addEventListener("wheel", mouseWheel);
        container = new PIXI.Container();
        preview = new PIXI.Container();
        app.stage.addChild(container);
        app.stage.addChild(preview);
        for (let i = 0; i < NUM_ATLAS; i++) {
            images[i] = await PIXI.Assets.load('test.png');
        }



        const rect = new PIXI.Rectangle(0, 0, 16, 16);

        texture = new PIXI.Texture({ source: images[currAtlas], frame: rect });
        previewSprite = new PIXI.Sprite(texture);
        preview.addChild(previewSprite);
        preview.visible = false;
        previewSprite.anchor.set(0.5);

        /*
        const container = new Container();
    
        app.stage.addChild(container);
         */
    }

    function canvasDown(ev) {
        switch (tool) {
            case "draw":
                const ns = new PIXI.Sprite(texture);
                ns.anchor.set(0.5);
                ns.x = ev.offsetX + valueH;
                ns.y = ev.offsetY + valueV;
                ns.rotation = rotazioneCorrente;
                ns.scale.x = scalaCorrenteX;
                ns.scale.y = scalaCorrenteY;
                container.addChild(ns);
                break;
            case "move":

                break;
            default:
                break;
        }


    }

    function mouseWheel(ev) {
        ev.preventDefault();
        const dir = ev.deltaY > 0;
        switch (tool) {
            case "draw":
                let alt = true;
                if (ev.shiftKey) {
                    scalaCorrenteX += dir ? 0.1 : -0.1;
                    previewSprite.scale.x = scalaCorrenteX;
                    alt = false;
                }
                if (ev.ctrlKey) {
                    scalaCorrenteY += dir ? 0.1 : -0.1;
                    previewSprite.scale.y = scalaCorrenteY;
                    alt = false;
                }
                if (alt) {
                    rotazioneCorrente += dir ? 0.1 : -0.1;
                    previewSprite.rotation = rotazioneCorrente;
                }
                break;
        }
    }

    function canvasUp(ev) {
        switch (tool) {
            case "draw":

                break;
            case "move":

                break;
            default:
                break;
        }


    }

    function canvasMove(ev) {
        switch (tool) {
            case "draw":
                previewSprite.x = ev.offsetX;
                previewSprite.y = ev.offsetY;

                break;
            case "move":

                break;
            default:
                break;
        }


    }

    function activateTool(cmd) {
        if (cmd !== oldtool) {
            if (oldtool) {
                toolboox.querySelector(`svg[data-cmd="${oldtool}"]`).classList.remove("active");
            }
            tool = cmd;
            oldtool = tool;
            toolboox.querySelector(`svg[data-cmd="${tool}"]`).classList.add("active");

            preview.visible = tool === "draw";

        }

    }
});


