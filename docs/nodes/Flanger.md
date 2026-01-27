# Flanger

## Overview
Audio effect node that creates a flanging effect using modulated delay.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| rate | float | 0.01 - 20 | 0.25 | LFO rate in Hz |
| depth | float | 0.0 - 1.0 | 0.5 | Modulation depth |
| feedback | float | -0.99 - 0.99 | 0.5 | Feedback amount |
| delay | float | 0.1 - 20 | 5.0 | Base delay in ms |
| mix | float | 0.0 - 1.0 | 0.5 | Dry/wet mix |

## Usage

```cpp
#include "nodes/effect/Flanger.h"

auto flanger = nap::FlangerFactory::create(0.5f, 0.7f);
flanger->setFeedback(0.6f);
```

## See Also
- Phaser
- Chorus
