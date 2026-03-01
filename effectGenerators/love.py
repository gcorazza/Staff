#!/usr/bin/env python3
"""
Water animation for a 1‑dimensional LED strip.

The script creates a PNG image of size 113 x 1000 pixels:
- Width  = 113 LED positions (0 … 112)
- Height = 1000 time steps (2 seconds, 500 steps per second)

Each row represents a frame in time, each column a LED.
A moving sine‑wave visualises water travelling along the strip.
Background is black, the wave is rendered in bright blue/cyan for high contrast.

Only the Pillow library from the standard Python ecosystem is required.
"""

from PIL import Image
import math

# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------
WIDTH = 113          # Number of LEDs (horizontal resolution)
HEIGHT = 1000        # Number of time frames (vertical resolution)

# Wave parameters
SPATIAL_CYCLES = 4.0     # Number of full sine periods across the strip
TEMPORAL_SPEED = 0.02   # Phase change per row (controls wave speed)

# Colour mapping (from wave amplitude to RGB)
def amplitude_to_color(ampl: float) -> tuple[int, int, int]:
    """
    Map a normalized amplitude (0.0 … 1.0) to a bright blue/cyan colour.
    Full amplitude → bright cyan, zero amplitude → black.
    """
    # Linear interpolation between black and cyan (0,255,255)
    intensity = int(255 * ampl)          # overall brightness
    return (0, intensity, intensity)    # (R, G, B)

# ----------------------------------------------------------------------
# Create image with black background
# ----------------------------------------------------------------------
img = Image.new("RGB", (WIDTH, HEIGHT), (0, 0, 0))
pixels = img.load()   # Pixel access object

# ----------------------------------------------------------------------
# Populate image row by row
# ----------------------------------------------------------------------
for y in range(HEIGHT):
    # Temporal phase shift – makes the wave move over time
    phase = TEMPORAL_SPEED * y * 2 * math.pi

    for x in range(WIDTH):
        # Spatial position as a fraction of the strip width
        pos = x / WIDTH

        # Sine wave value in range [-1, 1]
        wave = math.sin(2 * math.pi * SPATIAL_CYCLES * pos + phase)

        # Normalise to [0, 1] for colour mapping
        amplitude = (wave + 1) / 2.0

        # Assign colour based on amplitude
        pixels[x, y] = amplitude_to_color(amplitude)

# ----------------------------------------------------------------------
# Save the result
# ----------------------------------------------------------------------
output_path = "output.png"
img.save(output_path, "PNG")
print(f"Image saved as {output_path} ({WIDTH}×{HEIGHT} pixels)")


