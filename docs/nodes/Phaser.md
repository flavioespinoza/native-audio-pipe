# Phaser

## Overview
Audio effect node that creates a sweeping notch filter effect using all-pass filters.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| rate | float | 0.01 - 20 | 0.5 | LFO rate in Hz |
| depth | float | 0.0 - 1.0 | 0.7 | Modulation depth |
| feedback | float | -0.99 - 0.99 | 0.5 | Feedback amount |
| stages | uint32 | 2 - 12 | 4 | Number of all-pass stages |
| mix | float | 0.0 - 1.0 | 0.5 | Dry/wet mix |

## Usage

```cpp
#include "nodes/effect/Phaser.h"

auto phaser = nap::PhaserFactory::create(0.3f, 0.8f);
phaser->setStages(6);
```

## See Also
- Flanger
- Chorus
