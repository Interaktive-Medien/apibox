/* ------------------------------------------------------------------
   script.js — API Box WLAN Setup (Captive Portal)
   Wird per LittleFS hochgeladen, eingebunden in index.html.
------------------------------------------------------------------ */

function showTab(which) {
  document.getElementById('tab-home').classList.toggle('active', which === 'home');
  document.getElementById('tab-school').classList.toggle('active', which === 'school');
  document.getElementById('form-home').classList.toggle('active', which === 'home');
  document.getElementById('form-school').classList.toggle('active', which === 'school');
}
