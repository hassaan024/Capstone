NOTE: 
    The soil scoring uses standard agronomic optimal ranges since the plant species API we use doesn't expose per species soil preferences. That's a data limitation, not a model limitation if we had that data we could plug it right in.

# Prediction Model Documentation

Mathematical documentation for every model used in `src/routers/predict.py`.

---

## Overall Growth Formula

The core prediction is a multiplicative model:

```
daily_growth     = base_daily × overall_stress × stage_modifier × health_modifier
total_growth     = daily_growth × daysAhead
predicted_height = min(current_height + total_growth, maxHeightCm)
```

All stress and modifier factors are scalars in the range `[0.0, 1.0]`. Multiplying them
together means **any single severe stress compounds with others** — the model penalizes
multiple simultaneous stressors more aggressively than a single one.

---

## 1. Base Daily Growth — `_base_daily_growth_cm`

Establishes a species baseline cm/day before any stress is applied.

```
tier = { growthRate=1: 0.10, growthRate=2: 0.30, growthRate=3: 0.65 }  (cm/day)

height_scale = (maxHeightCm / 100) ^ 0.4

base_daily = tier × height_scale
```

- **Tier**: discrete lookup on the species' 1–3 growth rate rating.
- **Height scale**: sub-linear power function (exponent 0.4) so taller species grow faster
  in absolute cm/day but with diminishing returns.
  - 200 cm plant → `2^0.4 ≈ 1.32×`
  - 50 cm plant  → `0.5^0.4 ≈ 0.76×`

---

## 2. Temperature Stress — `_temp_factor`

Scores how well the forecast temperatures stay within the species' tolerated range
`[minTemp, maxTemp]` (°F).

For each forecast day:

```
if t < minTemp:
    score = max(0.1,  1.0 − (minTemp − t) / 20)

if t > maxTemp:
    score = max(0.1,  1.0 − (t − maxTemp) / 20)

else:
    score = 1.0

temp_factor = mean(scores)
```

- **Linear degradation**: factor drops 0.05 per °F outside the tolerance band (÷20 denominator).
- **Grace buffer**: approximately 10 °F outside the limit before reaching the floor of 0.1.
- **Floor**: 0.1 — extreme temperatures still allow trace growth.

---

## 3. Sunlight Stress — `_sunlight_factor`

Compares actual received radiation against the species' target using a simple ratio.

```
target_mj  = avgHoursSun × 2.5          (MJ/m²)
avg_actual = mean(sunlightIntensity)     (MJ/m²)

sun_factor = min(avg_actual / target_mj, 1.0)
```

- **Conversion**: 2.5 MJ/m² per hour of sun is a rough solar irradiance constant.
- **Capped at 1.0**: excess sunlight beyond the species target provides no additional benefit.
- **No over-irradiance penalty**: the model does not currently penalize excess sun.

---

## 4. Water Stress — `_water_factor`

Combines a **point-in-time soil moisture score** with a **precipitation trend score**.

### Moisture score

```
optimal range  = [0.35 − drought_buffer, 0.70]
drought_buffer = 0.15 if droughtTolerant else 0.0

if moisture in optimal range:
    moisture_score = 1.0

if moisture < lower_bound:
    moisture_score = max(0.1,  moisture / lower_bound)      # linear dry penalty

if moisture > 0.70:
    excess = moisture − 0.70
    moisture_score = max(0.2,  1.0 − excess × 2)            # steeper waterlogged penalty
```

### Precipitation trend score

```
net_water = Σ (precipitation − dailyEvaporation)  over forecast window  (mm)

if net_water < −30:
    trend_score = max(0.3,  1.0 + net_water / 60)   # linear deficit penalty
else:
    trend_score = 1.0
```

A net deficit > 30 mm triggers the penalty; the factor hits its floor of 0.3 at −90 mm.

### Combined

```
water_factor = (moisture_score + trend_score) / 2
```

- Drought-tolerant species shift the lower optimal boundary from 0.35 → 0.20, widening the safe zone.

---

## 5. Soil Quality — `_soil_factor`

Scores pH and NPK nutrition with a small organic matter bonus.

### pH score

```
optimal range = [6.0, 7.0]

if pH in optimal:
    ph_score = 1.0

if pH < 6.0:
    ph_score = max(0.3,  1.0 − (6.0 − pH) × 0.15)

if pH > 7.0:
    ph_score = max(0.3,  1.0 − (pH − 7.0) × 0.15)
```

Factor drops 0.15 per pH unit outside the optimal window; floor is 0.3.

### NPK score

```
npk_sum   = nitrogen + phosphorus + potassium
npk_score = min(1.0, npk_sum / 150)   if npk_sum > 0
            0.5                         otherwise
```

Reaches 1.0 at a combined NPK reading of 150.

### Organic matter bonus

```
organic_bonus = min(0.1, organicPercentage × 0.01)
```

### Combined

```
soil_factor = min(1.0, (ph_score + npk_score) / 2 + organic_bonus)
```

---

## 6. Overall Stress

```
overall = temp_factor × sun_factor × water_factor × soil_factor
```

A pure product with no weighting. Each factor is an independent multiplier, so the model
degrades the growth rate in proportion to the **product** of all active stresses.

---

## 7. Growth Stage & Health Modifiers

Discrete multipliers applied after stress:

### Growth Stage

| Stage      | Modifier |
|------------|----------|
| Seedling   | 0.70     |
| Vegetative | 1.00     |
| Flowering  | 0.65     |
| Fruiting   | 0.45     |
| Dormant    | 0.05     |

### Health Status

| Status     | Modifier |
|------------|----------|
| Healthy    | 1.00     |
| NeedsWater | 0.50     |
| Wilting    | 0.30     |
| Sick       | 0.20     |
| Dead       | 0.00     |

---

## 8. Confidence Score

Starts at 1.0 and subtracts penalties for sparse or uncertain inputs:

```
score = 1.0
  − 0.20   if fewer than 3 weather days provided
  − 0.10   if ageDays == 0 (just planted)
  − 0.05   if healthStatus is unknown
  − 0.05   if growthStage is unknown
  − 0.10   if daysAhead > 14 (long forecast horizon)

confidence = max(0.30, score)
```

Floor is 0.30 — the model always returns at least minimal confidence.
