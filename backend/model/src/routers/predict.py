import logging
import math
import math
from fastapi import APIRouter
from ..schema import PredictionRequest, PredictionResponse, StressFactors

logger = logging.getLogger(__name__)
router = APIRouter()


# ── Helpers ──────────────────────────────────────────────────────────────────

def _temp_factor(weather, species) -> float:
    """
    Score 0–1 based on how often daily temperature falls within the species'
    tolerated range.  Outside the range the factor degrades linearly with a
    12 °F buffer before hitting the floor of 0.1.
    """
    if not weather:
        return 0.5

    lo = species.minTemp
    hi = species.maxTemp
    if lo is None and hi is None:
        return 1.0

    scores = []
    for day in weather:
        t = day.temperatureF
        if lo is not None and t < lo:
            deficit = lo - t
            scores.append(max(0.1, 1.0 - deficit / 12))
        elif hi is not None and t > hi:
            excess = t - hi
            scores.append(max(0.1, 1.0 - excess / 12))
        else:
            scores.append(1.0)

    return round(sum(scores) / len(scores), 4)


# When a species has no avgHoursSun from the API, use these type-based defaults
# so sunlight actually varies with seasonal weather data instead of always 1.0.
_DEFAULT_SUN_HOURS: dict[str, float] = {
    'flower':    6.0,
    'vegetable': 8.0,
    'tree':      5.0,
}

def _sunlight_factor(weather, species) -> float:
    """
    Compare average daily sunlight intensity (MJ/m²) against what the species
    needs.  We treat avgHoursSun * 2.5 MJ/m² as a rough target.
    Missing species data → use a type-based default so seasonal light variation
    is reflected rather than always returning 1.0.
    """
    if not weather:
        return 1.0

    avg_hours = species.avgHoursSun
    if avg_hours is None:
        cat = (species.modelCategory or 'flower').lower()
        avg_hours = _DEFAULT_SUN_HOURS.get(cat, 6.0)

    target_mj = avg_hours * 2.5
    avg_actual = sum(d.sunlightIntensity for d in weather) / len(weather)

    if target_mj == 0:
        return 1.0

    ratio = avg_actual / target_mj
    return round(min(ratio, 1.0), 4)


def _water_factor(weather, soil, species) -> float:
    """
    Water stress combines:
      • soil estimated moisture (0–1, optimal 0.35–0.70)
      • net precipitation minus evaporation across the forecast window
      • drought tolerance softens the penalty
    """
    moisture = soil.estimatedMoisture
    drought_buffer = 0.15 if species.droughtTolerant else 0.0

    # Moisture score
    if 0.35 - drought_buffer <= moisture <= 0.70:
        moisture_score = 1.0
    elif moisture < 0.35 - drought_buffer:
        moisture_score = max(0.1, moisture / (0.35 - drought_buffer))
    else:  # waterlogged
        excess = moisture - 0.70
        moisture_score = max(0.2, 1.0 - excess * 2)

    # Trend score: net water over the period — tighter threshold so dry spells register sooner
    if weather:
        net_water = sum(d.precipitation - d.dailyEvaporation for d in weather)
        if net_water < -20:
            trend_score = max(0.2, 1.0 + net_water / 40)
        else:
            trend_score = 1.0
    else:
        trend_score = 1.0

    return round((moisture_score + trend_score) / 2, 4)


def _soil_factor(soil) -> float:
    """
    pH and NPK availability.  Optimal pH 6.0–7.0.  NPK are stored as
    absolute readings; we just reward non-zero balanced nutrition.
    """
    ph = soil.pH
    if 6.0 <= ph <= 7.0:
        ph_score = 1.0
    elif ph < 6.0:
        ph_score = max(0.3, 1.0 - (6.0 - ph) * 0.15)
    else:
        ph_score = max(0.3, 1.0 - (ph - 7.0) * 0.15)

    # NPK: reward balanced non-zero values
    npk_sum = soil.nitrogen + soil.phosphorus + soil.potassium
    npk_score = min(1.0, npk_sum / 150) if npk_sum > 0 else 0.5

    # Organic matter bonus
    organic_bonus = min(0.1, (soil.organicPercentage or 0) * 0.01)

    return round(min(1.0, (ph_score + npk_score) / 2 + organic_bonus), 4)


def _growth_stage_modifier(stage: str | None) -> float:
    return {
        "Seedling":   0.70,
        "Vegetative": 1.00,
        "Flowering":  0.65,
        "Fruiting":   0.45,
        "Dormant":    0.05,
    }.get(stage or "", 1.0)


def _health_modifier(status: str | None) -> float:
    return {
        "Healthy":    1.00,
        "NeedsWater": 0.50,
        "Wilting":    0.30,
        "Sick":       0.20,
        "Dead":       0.00,
    }.get(status or "", 1.0)


# Expected days to reach mature height — mirrors utils/plant-growth.ts, keep in sync.
_EXPECTED_DAYS: dict[tuple[str, str], list[int]] = {
    ('flower',    'annual'):    [105,  80,  65],
    ('flower',    'biennial'):  [600, 450, 300],
    ('flower',    'perennial'): [500, 210, 120],
    ('vegetable', 'annual'):    [120,  80,  55],
    ('vegetable', 'perennial'): [270, 180, 110],
    ('tree',      'annual'):    [130,  95,  65],
    ('tree',      'perennial'): [730, 730, 600],
}

_REFERENCE_HEIGHT_CM: dict[str, float] = {
    'flower':    60.0,
    'vegetable': 90.0,
    'tree':     300.0,
}

# Calibrated to match the typical overall stress score produced by the weighted
# formula below (weather * clamped-soil).  Lowered from 0.6 because sunlight
# now uses type defaults (no longer 1.0 for species missing avgHoursSun) and
# soil is clamped so it contributes less to the product.
_AVG_STRESS = 0.48


def _cycle_key(cycle: str | None) -> str:
    c = (cycle or '').lower()
    if 'annual' in c and 'biennial' not in c:
        return 'annual'
    if 'biennial' in c:
        return 'biennial'
    return 'perennial'


def _base_daily_growth_cm(species) -> float:
    """
    Baseline cm/day derived from a type+cycle-aware expected-days table so that
    different plant categories have biologically realistic growth timelines:
      • Annual flowers  → 80–200 days to mature
      • Vegetables      → 65–130 days
      • Trees           → 600–730 days (capped timeline)
    The base rate is calibrated so that at average stress (0.6) the plant
    reaches maxHeightCm in approximately the expected days.
    Baseline cm/day derived from a type+cycle-aware expected-days table so that
    different plant categories have biologically realistic growth timelines:
      • Annual flowers  → 80–200 days to mature
      • Vegetables      → 65–130 days
      • Trees           → 600–730 days (capped timeline)
    The base rate is calibrated so that at average stress (0.6) the plant
    reaches maxHeightCm in approximately the expected days.
    """
    cat = (species.modelCategory or 'flower').lower()
    ck  = _cycle_key(species.cycle)

    key   = (cat, ck)
    tiers = _EXPECTED_DAYS.get(key, _EXPECTED_DAYS[('flower', 'perennial')])

    tier_idx  = max(0, min(2, int(round(species.growthRate)) - 1))
    base_days = tiers[tier_idx]

    ref_h      = _REFERENCE_HEIGHT_CM.get(cat, 60.0)
    height_adj = math.pow(max(species.maxHeightCm, 1.0) / ref_h, 0.3)
    expected_days = base_days * height_adj

    # base_daily × AVG_STRESS × expected_days = maxHeightCm  →  solve for base_daily
    return species.maxHeightCm / (expected_days * _AVG_STRESS)
    cat = (species.modelCategory or 'flower').lower()
    ck  = _cycle_key(species.cycle)

    key   = (cat, ck)
    tiers = _EXPECTED_DAYS.get(key, _EXPECTED_DAYS[('flower', 'perennial')])

    tier_idx  = max(0, min(2, int(round(species.growthRate)) - 1))
    base_days = tiers[tier_idx]

    ref_h      = _REFERENCE_HEIGHT_CM.get(cat, 60.0)
    height_adj = math.pow(max(species.maxHeightCm, 1.0) / ref_h, 0.3)
    expected_days = base_days * height_adj

    # base_daily × AVG_STRESS × expected_days = maxHeightCm  →  solve for base_daily
    return species.maxHeightCm / (expected_days * _AVG_STRESS)


def _confidence(weather, plant, species) -> float:
    """Penalise confidence when inputs are sparse or at extremes."""
    score = 1.0

    if len(weather) < 3:
        score -= 0.20

    if plant.ageDays == 0:
        score -= 0.10

    # Unknown health / stage → slightly less certain
    if plant.healthStatus is None:
        score -= 0.05
    if plant.growthStage is None:
        score -= 0.05

    # Very long forecast horizon
    if plant.daysAhead > 14:
        score -= 0.10

    return round(max(0.30, score), 2)


def _build_notes(temp_f, sun_f, water_f, soil_f, plant, species) -> list[str]:
    notes = []

    if temp_f < 0.7:
        notes.append("Temperature outside comfortable range for this species.")

    if sun_f < 0.6:
        notes.append(
            f"Receiving less sunlight than the ~{species.avgHoursSun:.0f} hrs/day this species prefers."
            if species.avgHoursSun else "Sunlight may be limiting growth."
        )

    if water_f < 0.6:
        notes.append("Water stress detected — check irrigation and soil drainage.")

    if soil_f < 0.6:
        notes.append("Soil conditions are sub-optimal (check pH or nutrient levels).")

    if plant.growthStage == "Dormant":
        notes.append("Plant is dormant — minimal growth expected.")

    if plant.healthStatus in ("Wilting", "Sick"):
        notes.append(f"Plant health ({plant.healthStatus}) is reducing predicted growth significantly.")

    if plant.healthStatus == "Dead":
        notes.append("Plant is marked as Dead — no growth predicted.")

    overall = temp_f * sun_f * water_f * max(0.6, soil_f)
    if not notes and overall < 0.6:
        notes.append("Combined stress factors are limiting growth significantly.")
    elif not notes:
        notes.append("Conditions look good for healthy growth.")

    return notes


# ── Route ────────────────────────────────────────────────────────────────────

@router.post("/predict", response_model=PredictionResponse)
def predict(req: PredictionRequest) -> PredictionResponse:
    plant   = req.plant
    species = req.species
    soil    = req.soil
    weather = req.weather

    logger.info(
        "Prediction request — plant_id=%d  ageDays=%d  daysAhead=%d  weatherDays=%d",
        plant.id, plant.ageDays, plant.daysAhead, len(weather),
    )

    # ── Stress factors ───────────────────────────────────────────────────────
    temp_f  = _temp_factor(weather, species)
    sun_f   = _sunlight_factor(weather, species)
    water_f = _water_factor(weather, soil, species)
    soil_f  = _soil_factor(soil)

    # Soil is static (doesn't change with bloom date); clamp it so weather
    # factors drive meaningful variation when the user picks different dates.
    # Soil can reduce growth by at most 40% — weather does the rest.
    soil_clamped = max(0.6, soil_f)
    overall = round(temp_f * sun_f * water_f * soil_clamped, 4)

    # ── Growth calculation ───────────────────────────────────────────────────
    base_daily   = _base_daily_growth_cm(species)
    stage_mod    = _growth_stage_modifier(plant.growthStage)
    health_mod   = _health_modifier(plant.healthStatus)

    daily_growth = base_daily * overall * stage_mod * health_mod

    total_growth = daily_growth * plant.daysAhead

    # Cap at species maximum height
    current_height   = plant.heightCm
    predicted_height = min(current_height + total_growth, species.maxHeightCm)
    actual_growth    = predicted_height - current_height

    # ── Confidence & notes ───────────────────────────────────────────────────
    confidence = _confidence(weather, plant, species)
    notes      = _build_notes(temp_f, sun_f, water_f, soil_f, plant, species)

    response = PredictionResponse(
        plantInstanceId    = plant.id,
        currentHeightCm    = round(current_height, 2),
        predictedHeightCm  = round(predicted_height, 2),
        growthCm           = round(actual_growth, 2),
        daysAhead          = plant.daysAhead,
        dailyGrowthCm      = round(daily_growth, 4),
        stressFactors      = StressFactors(
            temperature = temp_f,
            sunlight    = sun_f,
            water       = water_f,
            soil        = soil_f,
            overall     = overall,
        ),
        confidence = confidence,
        notes      = notes,
    )

    logger.info(
        "Prediction result — plant_id=%d  current=%.1f cm  predicted=%.1f cm  "
        "growth=%.2f cm  overall_stress=%.2f  confidence=%.2f",
        plant.id, current_height, predicted_height,
        actual_growth, overall, confidence,
    )

    return response
