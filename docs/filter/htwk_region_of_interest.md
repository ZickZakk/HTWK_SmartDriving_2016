# Region of Interest

## Input
* Video_RGB_In (24 Bit)
* Grayscale_Video_Depth_In (8 Bit)

## Output
* Rect_RGB_Out (tRect)
* Rect_Depth_Out (tRect)
* Video_RGB_Out (24 Bit)
* Grayscale_Video_Depth_Out (8 Bit)

## Properties
* RGB video manipulation method (None, Crop, Rect)
Dieser Wert gibt an, ob und wie das RGB-Bild manipuliert werden soll.
    * **None** belässt das Bild im original Zustand. 
    * **Crop** beschneidet das Bild auf Basis der berechneten Region of Interest.
    * **Rect** zeichnet ein rotes Rechteck auf Basis der berechneten Region of Interest auf das Bild.


* Depth video manipulation method (None, Crop, Rect)
Dieser Wert gibt an, ob und wie das Tiefenbild manipuliert werden soll.
    * **None** belässt das Bild im original Zustand. 
    * **Crop** beschneidet das Bild auf Basis der berechneten Region of Interest.
    * **Rect** zeichnet ein rotes Rechteck auf Basis der berechneten Region of Interest auf das Bild.
    

* Hood scanline number
Dieser Wert gibt an, wie viele vertikale ScanLines für die Erkennung der Motorhaube genutzt werden sollen.


* Room scanline number
Dieser Wert gibt an, wie viele vertikale ScanLines für die Erkennung des Raums genutzt werden sollen.


* Room Height Manipulation
Dieser Wert gibt an, wie viel Prozent der ermittelten Raumhöhe tatsächlich aus Raum gewertet werden sollen. Damit soll umgangen werden, dass zu viel Strecke vom Bild abgeschnitten wird.


* Max hood detection count
Dieser Wert gibt an, wie oft die Motorhaube erkennt werden soll, bevor der ermittelte Wert festgesetzt wird.


* Detect hood
Dieser Wert gibt an, ob die Motorhaube erkannt werden soll.


* Detect room
Dieser Wert gibt an, ob der Raum erkannt werden soll.

## Beschreibung
Dieser Filter kann anhand des Tiefenbildes eine Region of Interest bilden, welche sich auf die Straße beschränkt und somit Raum und Motorhaube abscheidet.
Die Erkennung der Motorhaube geschieht durch das Auswerten des schwarzen Bereiches am unteren Bildrand.
Die Erkennung des Raumes geschieht durch das Auswerten des weißen Bereiches am oberen Bildrand.

Die Ausgabe der Region of Interest wurde eine Media-Description *tRect* angelegt.

## Benutzung
Das Tiefenbild erwiest sich in der Praxis als sehr fehleranfällig und voller Störungen. Deswegen wird folgende Filteranordnung empfohlen:

Depth_Image -> [Grayscale](filter/htwk_grayscale.md) -> [Noise Remover](filter/htwk_noise_remover.md) -> [Temporal Image]() -> [Region of Interest](filter/htwk_region_of_interest.md)
Video_RGB direkt an [Region of Interest](filter/htwk_region_of_interest.md) 


## tRect

1. Füge diese Methode hinzu
```cpp
tResult ROI::InitDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));

    tChar const *tRectDescription = pDescManager->GetMediaDescription("tRect");
    RETURN_IF_POINTER_NULL(tRectDescription);

    tRectType = new cMediaType(0, 0, 0, "tRect", tRectDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(tRectType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &tRectDescriptionSignal));

    RETURN_NOERROR;
}
```

2. Aufruf in StageFirst
```cpp
if (eStage == StageFirst)
{
    RETURN_IF_FAILED(InitDescriptions(__exception_ptr));
    RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
    RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
}
```

3. InputPin hinzufügen
```cpp
RETURN_IF_FAILED(roiRectInputPin.Create("roi", new cMediaType(0, 0, 0, "tRect"), static_cast<IPinEventSink *> (this)));
RETURN_IF_FAILED(RegisterPin(&roiRectInputPin));
```

4. in OnPinEvent() Werte auslesen
```cpp
if (source == &roiRectInputPin)
    {
        tRect roi;
        {
            __adtf_sample_read_lock_mediadescription(tRectDescriptionSignal, mediaSample, pCoderInput);

            pCoderInput->Get("tX", (tVoid *) &roi.tX);
            pCoderInput->Get("tY", (tVoid *) &roi.tY);
            pCoderInput->Get("tWidth", (tVoid *) &roi.tWidth);
            pCoderInput->Get("tHeight", (tVoid *) &roi.tHeight);
        }
    }
}
```

