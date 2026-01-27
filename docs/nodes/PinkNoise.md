# PinkNoise

## Overview
Audio source node that generates pink (1/f) noise with equal energy per octave.

## Category
Source Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| amplitude | float | 0.0 - 1.0 | 1.0 | Output amplitude |

## Technical Details
Uses the Voss-McCartney algorithm for efficient pink noise generation.

## Usage

```cpp
#include "nodes/source/PinkNoise.h"

auto noise = nap::PinkNoiseFactory::create(0.5f);
noise->start();
```

## See Also
- WhiteNoise
- SineOscillator
