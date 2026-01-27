# WhiteNoise

## Overview
Audio source node that generates white noise with equal energy across all frequencies.

## Category
Source Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| amplitude | float | 0.0 - 1.0 | 1.0 | Output amplitude |

## Usage

```cpp
#include "nodes/source/WhiteNoise.h"

auto noise = nap::WhiteNoiseFactory::create(0.5f);
noise->start();
```

## See Also
- PinkNoise
- SineOscillator
