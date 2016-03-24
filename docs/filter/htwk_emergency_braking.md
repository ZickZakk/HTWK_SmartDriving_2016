# HTWK Emergency Braking

## Input
* current_speed (in m/s)
* speed_to_set (in m/s)
* UFC (Ultraschall vorn mitte)
* UFCL (Ultraschall vorn mitte links)
* UFCR (Ultraschall vorn mitte rechts)

## Output
* set_Speed (in m/s)

## Properties
* Brake Speed
Während einer Notbremsung gibt dieser Wert an, ab welcher Geschwindigkeit (in m/s) kein rückwärts Signal mehr auf den Motor gegeben werden soll. Ist dieser Wert zu klein, fährt das Auto rückwärts und somit evtl. aus der Notbremsdistanz heraus.

* Stopping Distance
Dieser Wert gibt den Abstand in Metern an, ab wann eine Notbremsung ausgeführt werden soll.

## Beschreibung
Dieser Filter reagiert auf Hindernisse vor dem Auto und führt eine Notbremsung durch. Aufgrund der technischen Gegebenheiten reicht es nicht aus eine Geschwindigkeit von 0 auf den Motor zu geben, da sich dieser dann im Leerlauf befindet. Um eine Bremswirkung zu erzielen, muss der Leerlauf blockiert werden, was durch rückwärts fahren realisiert wird.

## Benutzung
An den Output muss der *AADC Calibration Xml* Filter angehängt werden um die Ansteuerung in Servo-Winkel zu übersetzen.

