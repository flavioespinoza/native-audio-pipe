# SineOscillator

## Overview
Audio source node that generates a pure sine wave signal.

## Category
Source Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| frequency | float | 0.0 - Nyquist | 440.0 | Frequency in Hz |
| amplitude | float | 0.0 - 1.0 | 1.0 | Output amplitude |
| phase | float | 0.0 - 2π | 0.0 | Phase offset in radians |

## Usage

```cpp
#include "nodes/source/SineOscillator.h"

auto osc = nap::SineOscillatorFactory::create(440.0f, 0.8f);
osc->start();
// ... process audio
osc->stop();
```

## See Also
- WhiteNoise
- ImpulseGenerator
