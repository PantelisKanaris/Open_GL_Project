# EPL426 OpenGL Scene

## Overview
This project renders a small animated **solar-system vignette** with a textured **Earth and Moon**, a glowing **Sun**, a **prop-driven airplane** in orbit, and a **twinkling starfield**.  
It’s built with legacy **OpenGL/GLUT + GLU** and uses **stb_image** to load textures.

The scene demonstrates:
- Dual-pass texturing to blend **day** and **night** Earth maps  
- Time-driven animation (orbits and propellers)  
- Simple atmospheric **day/night cycle** using a sine curve  
- Multiple light sources (**Sun** and airplane “nav” light)  
- Additive, depth-friendly rendering for **stars**

---

##  Basic Controls

### Keyboard
| Key | Action |
|-----|---------|
| `1` | Quick side camera preset (looks along +X from the left). |
| `2` | Quick front camera preset (looks along +Z from the front near the center planet). |
| `p` | Increase/Decrease planet (Sun/Moon) orbit speed.<br>💡 *Hold Down Arrow first to decrease instead of increase.* |
| `a` | Increase/Decrease airplane orbit speed (*same rule as above*). |
| `s` | Increase/Decrease propeller rotation speed. |
| `r` | Reset all speeds to defaults. |
| `+` | Zoom out (camera Z increases). |
| `_` | Zoom in (camera Z decreases). |
| `Esc` | Exit the program. |

### Special Keys
| Key | Action |
|-----|---------|
| `F1` | Toggle fullscreen. |
| `↑` (Up Arrow) | Set *increment mode* (the `p`, `a`, `s` keys nudge values **up**). |
| `↓` (Down Arrow) | Set *decrement mode* (the `p`, `a`, `s` keys nudge values **down**). |

**Notes:**
- Zoom is implemented by moving the camera along its **Z-axis** (`m_camera.m_pos.z()`).

---

##  Day/Night Cycle (Sine Wave)

Two related systems create a realistic day/night transition:

### 1. Sky Color Transition
`ChangeColourOfBackground()` maps the planet’s orbital angle to a smooth sine curve:
- Normalize angle → radians  
- Compute `t = (-sin(radians) + 1)/2` so it cycles **night → day → night**  
- Linearly interpolate between:
  - **Night color:** `(0, 0, 0)`
  - **Day sky color:** `(0.25, 0.45, 0.7)`

 *Result:* The background gradually fades from black to blue and back, in sync with the Sun’s apparent motion.

### 2. Earth Day/Night Blending
`GetDayFactor()` uses the Sun’s **Y position** (derived from the same orbit) to compute a factor in `[0,1]`, then applies **smoothstep** to soften transitions.

In `CreatePlanetUsingTexture()`:
- **Pass 1 (Day):** Bind *day texture*, enable lighting, write depth.  
- **Pass 2 (Night):** Bind *night texture*, set **additive blending** (`glBlendFunc(GL_ONE, GL_ONE)`), don’t write depth, and use `glDepthFunc(GL_EQUAL)` so we only add light exactly where the day sphere rendered.

 *Result:* Smooth blending between day and night textures with glowing night lights, avoiding z-fighting or seams.

---

##  Textures & Assets
- Image loading via **stb_image.h** with `stbi_set_flip_vertically_on_load(1)` to match OpenGL’s UV orientation.  
- **Earth:** `Textures/earth_2k.jpg` (day), `Textures/earth_2k_night.jpg` (night)  
- **Moon:** `Textures/2k_moon.jpg`  
- **Sun:** `Textures/2k_sun.jpg` (for hot core; outer glow is drawn as translucent shells)

---

##  Lighting Model

### Global
- `glEnable(GL_LIGHTING)` and `glEnable(GL_COLOR_MATERIAL)` for ambient/diffuse tracking  
- `glEnable(GL_NORMALIZE)` ensures correct normals under scaling

###  Sun Light — `GL_LIGHT1`
- Position follows the Sun’s orbital path  
- Brightness scales with **Sun Y position**:
  - Higher = “noon” → brighter
  - Lower = “midnight” → dimmer
- A `brightnessMultiplier` remaps Y into `[0.05, ~5.0]` and modulates:
  - Ambient
  - Diffuse (warm tint)
  - Specular components

###  Airplane Light — `GL_LIGHT2`
- A cool white/blue **navigation light** following the airplane’s orbit  
- Low ambient, strong diffuse/specular → crisp highlight effect

###  Sun Visual
- The textured Sun (`CreateSunWithTexture`) is **emissive** (lighting disabled for its core)  
- Multiple translucent shells create a soft outer glow  
- Actual illumination is provided by **GL_LIGHT1**, so the Sun both *looks bright* and *lights the Earth/Moon realistically*

---

##  Stars & Atmospherics
- `InitializeStars()` seeds **1,000 stars** with:
  - Position
  - Base alpha
  - Per-star phase
  - Twinkle amplitude
  - Size
- `RenderStars()`:
  - Renders by size buckets as **GL_POINTS**  
  - Uses additive blending: `glBlendFunc(GL_SRC_ALPHA, GL_ONE)`  
  - Alpha modulates by `sin(phase + time * 2)`  
  - Scaled by *night factor*, so stars **fade out during the day**

---

##  Animation & Timing
- `Update()` uses GLUT’s elapsed time to compute `deltaTime`, updating:
  - `m_PlanetAngle` — orbit for Sun/Moon  
  - `m_AirplaneAngle` — orbit for airplane  
  - `m_ProperllerAngle` — propeller rotation  
  - `m_timeStars` — star twinkle clock  
- All wrap with `fmod(..., 360.f)` for continuous motion  
- Keyboard controls adjust speeds in real-time  
  - **Down Arrow** flips sign to decrease instead of increase

---

##  Summary
This OpenGL project demonstrates:
- Procedural animation and time-based control
- Lighting and texturing for realistic celestial motion
- Day/night atmospheric transitions
- Blended multi-pass rendering
- A mix of static and dynamic light sources for depth and realism
