# PanNode

## Overview
Audio processing node that pans a stereo signal between left and right channels.

## Category
Math Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| pan | float | -1.0 to 1.0 | 0.0 | Pan position (-1=left, 0=center, 1=right) |
| panLaw | PanLaw | enum | ConstantPower | Pan law algorithm |

## Pan Laws
- `Linear`: Simple linear panning
- `ConstantPower`: Maintains consistent perceived loudness
- `MinusFourPointFive`: -4.5dB center attenuation

## Usage

```cpp
#include "nodes/math/PanNode.h"

auto pan = nap::PanNodeFactory::create();
pan->setPan(-0.5f);  // Pan slightly left
pan->setPanLaw(nap::PanNode::PanLaw::ConstantPower);
```

## See Also
- GainNode
- MixerNode
