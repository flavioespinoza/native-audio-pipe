# ReverbConvolution

## Overview
Audio effect node that applies convolution reverb using impulse responses.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| dryWetMix | float | 0.0 - 1.0 | 0.5 | Dry/wet mix |
| preDelay | float | 0 - 500 | 0 | Pre-delay in ms |

## Usage

```cpp
#include "nodes/effect/ReverbConvolution.h"

auto reverb = nap::ReverbConvolutionFactory::create();
reverb->loadImpulseResponse("hall.wav");
reverb->setDryWetMix(0.3f);
```

## See Also
- SimpleDelay
