# BitCrusher

## Overview
Audio effect node that reduces bit depth and sample rate for lo-fi effects.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| bitDepth | uint32 | 1 - 16 | 8 | Bit resolution |
| downsampleFactor | uint32 | 1 - N | 1 | Sample rate reduction factor |
| mix | float | 0.0 - 1.0 | 1.0 | Dry/wet mix |

## Usage

```cpp
#include "nodes/effect/BitCrusher.h"

auto crusher = nap::BitCrusherFactory::create(4, 4);  // 4-bit, 4x downsample
```

## See Also
- HardClipper
- SoftClipper
