document.addEventListener('DOMContentLoaded', () => {
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

    function calculateMaxValues() {
        const viewportWidth = window.innerWidth;
        const viewportHeight = window.innerHeight;
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
});