# SplitterNode

## Overview
Audio processing node that splits a single input signal to multiple outputs.

## Category
Math Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| numOutputs | uint32 | 1 - N | 2 | Number of output channels |
| outputGain[n] | float | 0.0 - ∞ | 1.0 | Gain for output n |

## Usage

```cpp
#include "nodes/math/SplitterNode.h"

auto splitter = nap::SplitterNodeFactory::create(4);
splitter->setOutputGain(0, 1.0f);
splitter->setOutputGain(1, 0.5f);
splitter->muteOutput(2, true);
```

## See Also
- MixerNode
- SummerNode
