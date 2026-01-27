# FileStreamReader

## Overview
Audio source node that streams audio from a file with seeking and looping support.

## Category
Source Nodes

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| filePath | string | "" | Path to audio file |
| looping | bool | false | Enable loop playback |

## Supported Formats
- WAV
- AIFF
- FLAC
- OGG (with appropriate codecs)

## Usage

```cpp
#include "nodes/source/FileStreamReader.h"

auto reader = nap::FileStreamReaderFactory::create("audio.wav", true);
reader->start();
reader->seek(44100);  // Seek to 1 second
```

## See Also
- SineOscillator
- ImpulseGenerator
