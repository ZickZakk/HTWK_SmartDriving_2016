# Noise Remover

## Input
* Grayscale_Video_Input (8 Bit)

## Output
* Grayscale_Video_Output (8 Bit)

## Properties
* Iterations
Dieser Wert gibt an, wie oft die logik zum Entfernen der Störungen angewendet werden soll. Je höher der Wert, desto größere Störungen werden entfernt, jedoch gehen mehr Informationen verloren.

## Beschreibung
Dieser Filter versucht mit Dilatation und Erosion Störungen zu beseitigen. Bei jeder Iteration wird 2x dilatiert und 1x erodiert.

## Benutzung
Dieser Filter funktioniert am besten auf Bildern, welche vorher von *[HTWK Grayscale](htwk_grayscale.md)* in ein Grauwert-Bild umgewandelt wurden.
