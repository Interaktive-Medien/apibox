<?php
// https://apibox.dorfkneipe.ch/api/set.php?id=1

// CORS Header 
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");
// header('Content-Type: application/json; charset=UTF-8');


require_once 'config.php';

// Box ID auslesen und validieren
if (!isset($_GET['id'])) {
    http_response_code(400);
    echo json_encode(["error" => "Parameter 'id' fehlt in der URL (z.B. ?id=1)."]);
    exit;
}

$box_id = intval($_GET['id']);
// echo $box_id;

// Validierung: Nur Box 1 bis 10 erlauben
if ($box_id < 1 || $box_id > 10) {
    http_response_code(400);
    echo json_encode(["error" => "Ungültige Box-ID."]);
    exit;
}

$tableName = "box" . $box_id; 
// echo $tableName;

// ###################################### Empfangen der JSON-Daten

$inputJSON = file_get_contents('php://input');

// nur zum Test ohne ESP32
$inputJSON = '{"temperatur": 22.5,"luftfeuchtigkeit": 45.2,"bewegung": 1,"lautstaerke": 65.4,"magnet": 0,"helligkeit": 400.5,"alkohol": 0.0,"lage_x": 1.2,"lage_y": -0.5,"gewicht": 150.0,"co2": 450,"luftdruck": 1013.25,"distanz": 120,"latitude": 46.8503,"longitude": 9.5310,"altitude": 593.0,"gps_time": "11:11:25","gps_num_satellites": 8}';

$input = json_decode($inputJSON, true); 

if (!$input) {
    http_response_code(400);
    echo json_encode(["error" => "Ungültiges oder leeres JSON empfangen."]);
    exit;
}

// ###################################### Werte extrahieren (mit Fallbacks)

// Null-Koaleszenz-Operator (??) setzt den Wert auf 0, falls der Key im JSON fehlt.
// Typ-Casting (floatval, intval, strval) sorgt für zusätzliche Sicherheit.
$temperatur         = floatval($input["temperatur"] ?? 0);
$luftfeuchtigkeit   = floatval($input["luftfeuchtigkeit"] ?? 0);
$bewegung           = intval($input["bewegung"] ?? 0); // Boolean wird in DB als TinyInt (0/1) gespeichert
$lautstaerke        = floatval($input["lautstaerke"] ?? 0);
$magnet             = intval($input["magnet"] ?? 0);
$helligkeit         = floatval($input["helligkeit"] ?? 0);
$alkohol            = floatval($input["alkohol"] ?? 0);
$lage_x             = floatval($input["lage_x"] ?? 0);
$lage_y             = floatval($input["lage_y"] ?? 0);
$gewicht            = floatval($input["gewicht"] ?? 0);
$co2                = intval($input["co2"] ?? 0);
$luftdruck          = floatval($input["luftdruck"] ?? 0);
$distanz            = intval($input["distanz"] ?? 0);
$latitude           = floatval($input["latitude"] ?? 0);
$longitude          = floatval($input["longitude"] ?? 0);
$altitude           = floatval($input["altitude"] ?? 0);
$gps_time           = strval($input["gps_time"] ?? "");
$gps_num_satellites = intval($input["gps_num_satellites"] ?? 0);

// ###################################### Eintragen in die Datenbank

// SQL Prepare Statement mit benannten Platzhaltern (Named Parameters)
$sql = "INSERT INTO `" . $tableName . "` 
        (temperatur, luftfeuchtigkeit, bewegung, lautstaerke, magnet, helligkeit, alkohol, lage_x, lage_y, gewicht, co2, luftdruck, distanz, latitude, longitude, altitude, gps_time, gps_num_satellites) 
        VALUES 
        (:temperatur, :luftfeuchtigkeit, :bewegung, :lautstaerke, :magnet, :helligkeit, :alkohol, :lage_x, :lage_y, :gewicht, :co2, :luftdruck, :distanz, :latitude, :longitude, :altitude, :gps_time, :gps_num_satellites)";

try {
    $stmt = $pdo->prepare($sql);
    
    // Assoziatives Array für die execute-Methode
    $stmt->execute([
        ':temperatur'         => $temperatur, 
        ':luftfeuchtigkeit'   => $luftfeuchtigkeit, 
        ':bewegung'           => $bewegung, 
        ':lautstaerke'        => $lautstaerke, 
        ':magnet'             => $magnet, 
        ':helligkeit'         => $helligkeit, 
        ':alkohol'            => $alkohol, 
        ':lage_x'             => $lage_x, 
        ':lage_y'             => $lage_y, 
        ':gewicht'            => $gewicht, 
        ':co2'                => $co2, 
        ':luftdruck'          => $luftdruck, 
        ':distanz'            => $distanz, 
        ':latitude'           => $latitude, 
        ':longitude'          => $longitude, 
        ':altitude'           => $altitude, 
        ':gps_time'           => $gps_time, 
        ':gps_num_satellites' => $gps_num_satellites
    ]);
    
    // Erfolgsmeldung als JSON zurückgeben
    echo json_encode([
        "status" => "success", 
        "message" => "Daten erfolgreich in $tableName gespeichert."
    ]);

} catch (PDOException $e) {
    // Fehler abfangen, z.B. wenn die Tabelle nicht existiert oder ein DB-Fehler auftritt
    http_response_code(500);
    echo json_encode([
        "status" => "error", 
        "message" => "Datenbankfehler: " . $e->getMessage()
    ]);
}
?>