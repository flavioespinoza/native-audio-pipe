# BiQuadFilter

## Overview
Audio effect node implementing a versatile biquad filter with multiple filter types.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| filterType | FilterType | enum | LowPass | Filter type |
| frequency | float | 20 - 20000 | 1000 | Cutoff/center frequency |
| Q | float | 0.1 - 30 | 0.707 | Resonance/Q factor |
| gain | float | -24 - +24 | 0 | Gain for shelving/peaking (dB) |

## Filter Types
- LowPass, HighPass, BandPass
- Notch, AllPass
- PeakingEQ, LowShelf, HighShelf

## Usage

```cpp
#include "nodes/effect/BiQuadFilter.h"

auto filter = nap::BiQuadFilterFactory::createLowPass(2000.0f, 1.0f);
```

## See Also
- DCBlockerNode
