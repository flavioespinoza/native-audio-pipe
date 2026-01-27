# ScopeNode

## Overview
Audio utility node for capturing waveform data for visualization.

## Category
Utility Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| bufferSize | size_t | 64 - 8192 | 1024 | Capture buffer size |
| triggerLevel | float | -1.0 - 1.0 | 0.0 | Trigger threshold |
| triggerEnabled | bool | - | false | Enable triggered capture |

## Usage

```cpp
#include "nodes/utility/ScopeNode.h"

auto scope = nap::ScopeNodeFactory::create(2048);
scope->setTriggerEnabled(true);
scope->setTriggerLevel(0.1f);

// Get waveform for display
auto waveform = scope->getWaveformData(0);
```

## See Also
- MeterNode
