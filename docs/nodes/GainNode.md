# GainNode

## Overview
Audio processing node that applies gain (volume) adjustment to the signal with smooth parameter changes.

## Category
Math Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| gain | float | 0.0 - ∞ | 1.0 | Linear gain multiplier |
| gainDb | float | -∞ - +∞ | 0.0 | Gain in decibels |

## Usage

```cpp
#include "nodes/math/GainNode.h"
#include "nodes/math/GainNodeFactory.h"

// Using factory
auto gain = nap::GainNodeFactory::create();
gain->setGain(0.5f);  // 50% volume

// Or with dB
gain->setGainDb(-6.0f);  // -6dB
```

## Technical Details
- Implements smooth gain transitions to avoid clicks
- Supports both linear and dB gain settings
- Pass-through when bypassed

## See Also
- MixerNode
- SummerNode
