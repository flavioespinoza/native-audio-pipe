# MixerNode

## Overview
Audio processing node that mixes multiple input signals into a single stereo output.

## Category
Math Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| numInputs | uint32 | 1 - N | 2 | Number of input channels |
| inputGain[n] | float | 0.0 - ∞ | 1.0 | Gain for input n |
| masterGain | float | 0.0 - ∞ | 1.0 | Master output gain |

## Usage

```cpp
#include "nodes/math/MixerNode.h"

auto mixer = nap::MixerNodeFactory::create(4);  // 4-input mixer
mixer->setInputGain(0, 0.8f);
mixer->setInputGain(1, 0.6f);
mixer->muteInput(2, true);
mixer->setMasterGain(0.9f);
```

## See Also
- SplitterNode
- SummerNode
- GainNode
