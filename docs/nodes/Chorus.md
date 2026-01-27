# Chorus

## Overview
Audio effect node that creates a rich chorus effect using multiple modulated voices.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| rate | float | 0.1 - 10 | 1.5 | LFO rate in Hz |
| depth | float | 0.0 - 1.0 | 0.5 | Modulation depth |
| delay | float | 1 - 50 | 25 | Base delay in ms |
| voices | uint32 | 1 - 8 | 3 | Number of chorus voices |
| mix | float | 0.0 - 1.0 | 0.5 | Dry/wet mix |

## Usage

```cpp
#include "nodes/effect/Chorus.h"

auto chorus = nap::ChorusFactory::create(2.0f, 0.6f, 4);
```

## See Also
- Flanger
- Phaser
- SimpleDelay
