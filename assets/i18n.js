/**
 * Brick Blaster - Web Portal Internationalization
 */

const translations = {
    es: {
        title: "Brick Blaster",
        subtitle: "VESPA RECOVERY MISSION",
        controls: "CONTROLES",
        moveLeft: "MOVER IZQ",
        moveRight: "MOVER DER",
        fire: "DISPARAR",
        pause: "PAUSA",
        music: "MUSICA",
        storyTitle: "HISTORIA",
        story1: "Era una mañana cualquiera. Ibas a comprar el pan, pero <strong>Mariano el Marciano</strong> te robó la Vespa. Ahora estás atrapado en una <strong>prisión espacial de ladrillos</strong>.",
        story2: "Tu arma: una tabla de planchar y una pelota de tenis nuclear. ¡Recupera la moto y vuelve a tiempo para comer!",
        about: "SOBRE EL JUEGO",
        aboutText: "Bienvenido a <strong>Brick Blaster</strong>, un clon de Arkanoid de alto octanaje para Amstrad CPC. Navega por tu nave, recoge potenciadores y destruye niveles de ladrillos geométricos.",
        creditsCode: "CÓDIGO Y GFX: ISSALIG",
        creditsMusic: "MÚSICA: ULTRASYD",
        creditsPowered: "POTENCIADO POR CPCTELERA",
        creditsYear: "2026",
        downloadTitle: "DESCARGAR DSK",
        downloadText: "Llévate el juego a tu emulador favorito.",
        downloadBtn: "DESCARGAR .DSK"
    },
    en: {
        title: "Brick Blaster",
        subtitle: "VESPA RECOVERY MISSION",
        controls: "CONTROLS",
        moveLeft: "MOVE LEFT",
        moveRight: "MOVE RIGHT",
        fire: "FIRE / ACTION",
        pause: "PAUSE",
        music: "MUSIC TOGGLE",
        storyTitle: "STORY",
        story1: "It was a normal morning. You went for bread, but <strong>Martin the Martian</strong> stole your scooter. Now you are trapped in a <strong>space prison of bricks</strong>.",
        story2: "Your weapon: an ironing board and a nuclear tennis ball. Get your scooter back and return in time for dinner!",
        about: "ABOUT",
        aboutText: "Welcome to <strong>Brick Blaster</strong>, a high-octane Arkanoid-style clone for the Amstrad CPC. Navigate your craft, collect power-ups, and blast through levels of geometric bricks.",
        creditsCode: "CODE & GFX: ISSALIG",
        creditsMusic: "MUSIC: ULTRASYD",
        creditsPowered: "POWERED BY CPCTELERA",
        creditsYear: "2026",
        downloadTitle: "DOWNLOAD DSK",
        downloadText: "Play on your favorite emulator.",
        downloadBtn: "DOWNLOAD .DSK"
    },
    fr: {
        title: "Brick Blaster",
        subtitle: "VESPA RECOVERY MISSION",
        controls: "CONTROLES",
        moveLeft: "VERS LA GAUCHE",
        moveRight: "VERS LA DROITE",
        fire: "TIRER",
        pause: "PAUSE",
        music: "MUSIQUE",
        storyTitle: "HISTOIRE",
        story1: "C'était un matin normal. Tu allais au pain, mais <strong>Lucien le Martien</strong> a volé ton scooter. Tu es maintenant coincé dans une <strong>prison de briques</strong>.",
        story2: "Ton arme: une planche à repasser et une balle de tennis nucléaire. Récupère ton scooter et rentre pour manger!",
        about: "À PROPOS",
        aboutText: "Bienvenue sur <strong>Brick Blaster</strong>, un clone de style Arkanoid pour l'Amstrad CPC. Pilotez votre vaisseau, ramassez des bonus et détruisez des niveaux de briques géométriques.",
        creditsCode: "CODE ET GFX : ISSALIG",
        creditsMusic: "MUSIQUE : ULTRASYD",
        creditsPowered: "PRODUIT PAR CPCTELERA",
        creditsYear: "2026",
        downloadTitle: "TÉLÉCHARGER DSK",
        downloadText: "Jouez sur votre émulateur préféré.",
        downloadBtn: "TÉLÉCHARGER .DSK"
    },
    gr: {
        title: "Brick Blaster",
        subtitle: "VESPA RECOVERY MISSION",
        controls: "ΧΕΙΡΙΣΤΗΡΙΑ",
        moveLeft: "ΚΙΝΗΣΗ ΑΡΙΣΤΕΡΑ",
        moveRight: "ΚΙΝΗΣΗ ΔΕΞΙΑ",
        fire: "ΒΟΛΗ / ΔΡΑΣΗ",
        pause: "ΠΑΥΣΗ",
        music: "ΜΟΥΣΙΚΗ",
        storyTitle: "ΙΣΤΟΡΙΑ",
        story1: "Ήταν ένα κανονικό πρωί. Πηγαίνεις για ψωμί, αλλά ο <strong>Λουκιανός ο Αρειανός</strong> έκλεψε το σκούτερ. Τώρα είσαι παγιδευμένος σε μια <strong>φυλακή από τούβλα</strong>.",
        story2: "Το όπλο σου: μια σιδερώστρα και μια πυρηνική μπάλα τένις. Πάρε πίσω το σκούτερ σου και γύρνα για φαγητό!",
        about: "ΣΧΕΤΙΚΑ",
        aboutText: "Καλώς ήρθατε στο <strong>Brick Blaster</strong>, ένα κλώνο του Arkanoid για τον Amstrad CPC. Οδηγήστε το σκάφος σας, συλλέξτε αναβαθμίσεις και καταστρέψτε τα τούβλα.",
        creditsCode: "ΚΩΔΙΚΑΣ & GFX: ISSALIG",
        creditsMusic: "ΜΟΥΣΙΚΗ: ULTRASYD",
        creditsPowered: "ΜΕ ΤΗΝ ΥΠΟΣΤΗΡΙΞΗ ΤΟΥ CPCTELERA",
        creditsYear: "2026",
        downloadTitle: "ΛΗΨΗ DSK",
        downloadText: "Παίξτε στον αγαπημένο σας εξομοιωτή.",
        downloadBtn: "ΛΗΨΗ .DSK"
    }
};

function setLanguage(lang) {
    if (!translations[lang]) lang = 'en';

    document.documentElement.lang = lang;

    // Update elements with data-i18n attribute
    document.querySelectorAll('[data-i18n]').forEach(el => {
        const key = el.getAttribute('data-i18n');
        if (translations[lang] && translations[lang][key]) {
            el.innerHTML = translations[lang][key];
        }
    });

    // Save preference
    localStorage.setItem('preferred-language', lang);

    // Update active state in switcher
    document.querySelectorAll('.lang-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.lang === lang);
    });

    // Update download link
    const downloadLink = document.getElementById('download-link');
    if (downloadLink) {
        downloadLink.href = `assets/brickblaster_${lang}.dsk`;
    }

    // Update iframe source to reload the emulator with the new language
    const frame = document.getElementById('emulator-frame');
    if (frame) {
        frame.src = `emulator.html?lang=${lang}`;
    }
}

// Initial language detection
window.addEventListener('DOMContentLoaded', () => {
    const savedLang = localStorage.getItem('preferred-language');
    const browserLang = navigator.language.split('-')[0];
    const initialLang = savedLang || (translations[browserLang] ? browserLang : 'en');

    setLanguage(initialLang);
});
