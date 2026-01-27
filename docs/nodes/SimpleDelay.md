# SimpleDelay

## Overview
Audio effect node that applies a basic delay with feedback and mix controls.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| delayTime | float | 0 - maxDelay | 250ms | Delay time in milliseconds |
| feedback | float | 0.0 - 0.99 | 0.3 | Feedback amount |
| mix | float | 0.0 - 1.0 | 0.5 | Dry/wet mix |
| maxDelayTime | float | - | 2000ms | Maximum delay time |

## Usage

```cpp
#include "nodes/effect/SimpleDelay.h"

auto delay = nap::SimpleDelayFactory::create(500.0f, 0.4f, 0.5f);
```

## See Also
- Chorus
- Flanger
