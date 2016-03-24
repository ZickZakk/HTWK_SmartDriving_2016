# HTWK Grayscale

## Input
* Video_Input

## Output
* Grayscale_Video_Output

## Beschreibung
Dieser Filter verwandelt einen Video Input der Modi

* 8UC3 (RGB)
* 8UC1 (Grauwert)
* 16UC1 (Depth)

in ein 8 Bit (8UC1) Grauwert Bild um.

Das Ausgabeformat ist:
```cpp
videoOutputFormat.nWidth = videoInputFormat.nWidth;
videoOutputFormat.nHeight = videoInputFormat.nHeight;
videoOutputFormat.nBitsPerPixel = 8;
videoOutputFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
videoOutputFormat.nBytesPerLine = videoInputFormat.nWidth;
videoOutputFormat.nSize = videoOutputFormat.nBytesPerLine * videoInputFormat.nHeight;
videoOutputFormat.nPaletteSize = 0;
```



