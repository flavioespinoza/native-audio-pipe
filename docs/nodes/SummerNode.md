# SummerNode

## Overview
Audio processing node that sums multiple input signals with optional normalization.

## Category
Math Nodes

## Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| numInputs | uint32 | 2 | Number of inputs to sum |
| normalize | bool | false | Divide by number of inputs |
| clip | bool | true | Clip output to -1.0 to 1.0 |

## Usage

```cpp
#include "nodes/math/SummerNode.h"

auto summer = nap::SummerNodeFactory::createNormalized(4);
summer->setClip(true);
```

## See Also
- MixerNode
- GainNode
