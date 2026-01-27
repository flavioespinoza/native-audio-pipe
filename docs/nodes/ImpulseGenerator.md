# ImpulseGenerator

## Overview
Audio source node that generates impulse (click) signals at regular intervals or on trigger.

## Category
Source Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| amplitude | float | 0.0 - 1.0 | 1.0 | Impulse amplitude |
| intervalMs | float | 0 - ∞ | 1000 | Interval between impulses |
| oneShot | bool | - | false | Single impulse mode |

## Usage

```cpp
#include "nodes/source/ImpulseGenerator.h"

auto gen = nap::ImpulseGeneratorFactory::create(100.0f);  // 100ms interval
gen->start();

// Manual trigger
gen->trigger();
```

## See Also
- SineOscillator
- WhiteNoise
