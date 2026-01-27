# DCBlockerNode

## Overview
Audio utility node that removes DC offset from the signal using a high-pass filter.

## Category
Utility Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| cutoffFrequency | float | 1 - 100 | 10 | Cutoff frequency in Hz |

## Technical Details
Implements a simple one-pole high-pass filter:
`y[n] = x[n] - x[n-1] + R * y[n-1]`

## Usage

```cpp
#include "nodes/utility/DCBlockerNode.h"

auto blocker = nap::DCBlockerNodeFactory::create(20.0f);
```

## See Also
- BiQuadFilter
- MeterNode
