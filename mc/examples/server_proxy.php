<?php
/**
 * server_proxy.php — Beispiel: API Box server-seitig abfragen (PHP cURL)
 * ----------------------------------------------------------------------
 * Holt einen Sensorwert von der API Box ueber die PHP-cURL-Funktionen:
 *   curl_init, curl_setopt, curl_exec, curl_close.
 *
 * Nutzen:
 *  - als Proxy: umgeht CORS-Probleme im Browser (Server holt die Daten, nicht der Client)
 *  - zum Sammeln/Speichern von Messwerten in einer Datenbank ueber einen laengeren Zeitraum
 *
 * Aufruf im Browser:
 *   http://DEIN-SERVER/server_proxy.php?ip=192.168.0.42&sensor=temperatur
 *
 * Rueckgabe: der unveraenderte JSON-String der API Box (Content-Type: application/json).
 */

// ---- Parameter einlesen (mit Defaults) ----
$ip     = isset($_GET['ip'])     ? $_GET['ip']     : '192.168.0.42';
$sensor = isset($_GET['sensor']) ? $_GET['sensor'] : 'temperatur';

// ---- Eingaben absichern (nur erlaubte Endpunkte + IP-Format) ----
$erlaubt = [
  'temperatur', 'luftfeuchtigkeit', 'co2', 'bewegung', 'lautstaerke',
  'magnet', 'helligkeit', 'alkohol', 'lage', 'gewicht', 'rauch',
  'luftdruck', 'hoehe', 'distanz'
];
if (!in_array($sensor, $erlaubt, true)) {
  http_response_code(400);
  header('Content-Type: application/json');
  echo json_encode(['fehler' => 'Unbekannter Sensor', 'erlaubt' => $erlaubt]);
  exit;
}
if (!filter_var($ip, FILTER_VALIDATE_IP)) {
  http_response_code(400);
  header('Content-Type: application/json');
  echo json_encode(['fehler' => 'Ungueltige IP-Adresse']);
  exit;
}

// ---- URL der API Box zusammensetzen ----
$url = 'http://' . $ip . '/' . $sensor;

// ---- cURL-Request an die API Box ----
$ch = curl_init();                                  // cURL-Handle erzeugen
curl_setopt($ch, CURLOPT_URL, $url);                // Ziel-URL
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);     // Antwort als String zurueckgeben
curl_setopt($ch, CURLOPT_TIMEOUT, 5);               // Timeout 5 Sekunden
curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 3);        // Verbindungs-Timeout

$antwort  = curl_exec($ch);                         // Request ausfuehren
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);  // HTTP-Statuscode
$fehler   = curl_error($ch);                        // evtl. Fehlertext
curl_close($ch);                                    // Handle schliessen

// ---- Ergebnis an den Client ausgeben ----
header('Content-Type: application/json; charset=utf-8');

if ($antwort === false || $httpCode !== 200) {
  http_response_code(502);
  echo json_encode([
    'fehler'   => 'API Box nicht erreichbar',
    'url'      => $url,
    'httpCode' => $httpCode,
    'curl'     => $fehler
  ]);
  exit;
}

// JSON-String der API Box unveraendert durchreichen
echo $antwort;

/*
 * Beispiel: Wert in PHP weiterverarbeiten (z.B. fuer DB-Speicherung)
 * ------------------------------------------------------------------
 * $daten   = json_decode($antwort, true);
 * $wert    = $daten['wert'];
 * $einheit = $daten['einheit'];
 * // ... hier in Datenbank schreiben (PDO / mysqli) ...
 */
?>
