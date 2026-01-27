# SoftClipper

## Overview
Audio effect node that applies soft clipping (saturation) distortion with multiple curve types.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| drive | float | 0.1 - ∞ | 1.0 | Input drive amount |
| curveType | CurveType | enum | Tanh | Saturation curve |
| mix | float | 0.0 - 1.0 | 1.0 | Dry/wet mix |

## Curve Types
- Tanh: Hyperbolic tangent (smooth)
- Atan: Arctangent (warm)
- Cubic: Polynomial (aggressive)
- Sine: Sinusoidal (musical)

## Usage

```cpp
#include "nodes/effect/SoftClipper.h"

auto clipper = nap::SoftClipperFactory::create(2.0f, nap::SoftClipper::CurveType::Tanh);
```

## See Also
- HardClipper
- BitCrusher
