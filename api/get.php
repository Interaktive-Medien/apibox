<?php
// https://apibox.dorfkneipe.ch/api/get.php?id=3&sensor=co2
// CORS Header (Wichtig, damit die Studierenden per JS (fetch) von ihren eigenen Hostpoint-Seiten zugreifen können!)
header("Access-Control-Allow-Origin: *");
header('Content-Type: application/json; charset=UTF-8');

require_once 'config.php';

// 1. Statische Metadaten definieren (Allowlist & Anreicherung)
// Hier trägst du alle Hardware-Namen und Einheiten ein, passend zu deinen Spaltennamen.
$sensorMeta = [
    'co2'              => ['einheit' => 'ppm',  'datentyp' => 'int',   'sensor' => 'SCD41'],
    'temperatur'       => ['einheit' => 'C',   'datentyp' => 'float', 'sensor' => 'SCD41'],
    'luftfeuchtigkeit' => ['einheit' => '%',    'datentyp' => 'float', 'sensor' => 'SCD41'],
    'luftdruck'        => ['einheit' => 'hPa',  'datentyp' => 'float', 'sensor' => 'BMP280'],
    'distanz'          => ['einheit' => 'mm',   'datentyp' => 'int',   'sensor' => 'VL6180X'],
    'bewegung'         => ['einheit' => '',     'datentyp' => 'bool',  'sensor' => 'PIR'],
    'lautstaerke'      => ['einheit' => '%',    'datentyp' => 'int',   'sensor' => 'Mic'],
    'magnet'           => ['einheit' => '',     'datentyp' => 'bool',  'sensor' => 'Hall'],
    'helligkeit'       => ['einheit' => '%',    'datentyp' => 'int',   'sensor' => 'LDR'],
    'alkohol'          => ['einheit' => '',     'datentyp' => 'int',   'sensor' => 'MQ3'],
    'lage_x'           => ['einheit' => 'Grad',    'datentyp' => 'float', 'sensor' => 'MPU6050'],
    'lage_y'           => ['einheit' => 'Grad',    'datentyp' => 'float', 'sensor' => 'MPU6050'],
    'gewicht'          => ['einheit' => 'kg',   'datentyp' => 'float', 'sensor' => 'LoadCell']
];

// 2. Den gewünschten Sensor aus der URL auslesen
// Das Skript unterstützt nun saubere REST-Aufrufe wie ?sensor=co2, 
// toleriert aber auch deine gewünschte Kurzform ?co2
$requestedSensor = null;
if (isset($_GET['sensor'])) {
    $requestedSensor = strtolower($_GET['sensor']);
} 

// 3. Sicherheits-Check: Wurde ein gültiger Sensor angefragt?
if (!$requestedSensor || !array_key_exists($requestedSensor, $sensorMeta)) {
    http_response_code(400); // 400 Bad Request
    echo json_encode([
        "error" => "Ungültiger oder fehlender Sensor.", 
        "erlaubte_parameter" => array_keys($sensorMeta) // Hilft Studis beim Debuggen
    ]);
    exit;
}

// 3b. Welches Device (Tabelle) soll abgefragt werden? ?id=1 -> Tabelle box1
$requestedId = null;
if (isset($_GET['id'])) {
    // nur positive ganze Zahlen erlauben
    if (preg_match('/^[0-9]+$/', $_GET['id'])) {
        $requestedId = intval($_GET['id']);
    }
}

if ($requestedId === null) {
    http_response_code(400);
    echo json_encode(["error" => "Ungültige oder fehlende id. Erwartet: positive ganze Zahl (z.B. id=1)"]);
    exit;
}

// 4. SQL Abfrage: Nur den aktuellsten Wert der einen spezifischen Spalte holen
// ORDER BY id DESC LIMIT 1 ist die performanteste Methode für den neusten Wert.
// Parametrisierte Abfrage mit Placeholder für zusätzliche Sicherheit.
$table = 'box' . $requestedId; // safe because $requestedId ist bereits als int validiert
$sql = "SELECT `$requestedSensor` AS wert, zeit FROM `$table` ORDER BY id DESC LIMIT 1";

try {
    $stmt = $pdo->prepare($sql);
    $stmt->execute();
    $dbResult = $stmt->fetch(PDO::FETCH_ASSOC); // Nutze fetch() statt fetchAll() für einen einzelnen Datensatz

    if ($dbResult) {
        // 5. Finalen JSON-String zusammenbauen (DB-Werte + PHP-Metadaten)
        $response = [
            "wert"     => $dbResult['wert'],
            "einheit"  => $sensorMeta[$requestedSensor]['einheit'],
            "datentyp" => $sensorMeta[$requestedSensor]['datentyp'],
            "sensor"   => $sensorMeta[$requestedSensor]['sensor'],
            "zeit"     => $dbResult['zeit']
        ];
        
        // JSON_NUMERIC_CHECK sorgt dafür, dass Integer und Floats im JSON nicht als Strings ("500") 
        // ausgegeben werden, sondern als echte Zahlen (500). Das macht das Plotten mit JS später leichter.
        echo json_encode($response, JSON_NUMERIC_CHECK);
        exit;
        
    } else {
        http_response_code(404);
        echo json_encode(["error" => "Keine Datensätze gefunden"]);
        exit;
    }
} catch (PDOException $e) {
    http_response_code(500);
    $msg = $e->getMessage();
    echo json_encode(["error" => "Database error", "message" => $msg]);
    error_log("Database error in get.php: " . $msg);
    exit;
}
?>