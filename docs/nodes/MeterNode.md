# MeterNode

## Overview
Audio utility node for measuring signal levels (peak and RMS).

## Category
Utility Nodes

## Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| getPeakLevel(ch) | float | Get peak level for channel |
| getRmsLevel(ch) | float | Get RMS level for channel |
| getPeakLevelDb(ch) | float | Get peak level in dB |
| getRmsLevelDb(ch) | float | Get RMS level in dB |
| resetPeaks() | void | Reset peak hold values |

## Usage

```cpp
#include "nodes/utility/MeterNode.h"

auto meter = nap::MeterNodeFactory::create();
// ... process audio ...
float peakL = meter->getPeakLevelDb(0);
float peakR = meter->getPeakLevelDb(1);
```

## See Also
- ScopeNode
