# InverterNode

## Overview
Audio processing node that inverts (phase flips) the audio signal.

## Category
Math Nodes

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| invertLeft | bool | true | Invert left channel |
| invertRight | bool | true | Invert right channel |

## Usage

```cpp
#include "nodes/math/InverterNode.h"

auto inverter = nap::InverterNodeFactory::create();
inverter->setInvertLeft(true);
inverter->setInvertRight(false);  // Only invert left
```

## Use Cases
- Phase correction
- Creating stereo width effects
- Canceling signals

## See Also
- GainNode
