# RingModulator

## Overview
Audio effect node that applies ring modulation by multiplying the signal with an oscillator.

## Category
Effect Nodes

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| frequency | float | 0 - Nyquist | 440 | Modulator frequency in Hz |
| waveform | WaveformType | enum | Sine | Modulator waveform |
| mix | float | 0.0 - 1.0 | 1.0 | Dry/wet mix |

## Waveform Types
- Sine, Square, Triangle, Sawtooth

## Usage

```cpp
#include "nodes/effect/RingModulator.h"

auto mod = nap::RingModulatorFactory::create(220.0f);
mod->setWaveform(nap::RingModulator::WaveformType::Square);
```

## See Also
- Phaser
- Chorus
