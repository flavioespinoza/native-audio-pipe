# HardClipper

## Overview
Audio effect node that applies hard clipping distortion to the signal.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| threshold | float | 0.0 - 1.0 | 0.8 | Clipping threshold |
| inputGain | float | -∞ - +∞ | 0dB | Input gain in dB |
| outputGain | float | -∞ - +∞ | 0dB | Output gain in dB |

## Usage

```cpp
#include "nodes/effect/HardClipper.h"

auto clipper = nap::HardClipperFactory::create(0.5f);
clipper->setInputGain(6.0f);  // Drive into clipping
```

## See Also
- SoftClipper
- BitCrusher
