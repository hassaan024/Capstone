import React, { useMemo, useState } from "react";
import { ResponsivePie } from "@nivo/pie";

type FeatureKey =
  | "Lifecycle"
  | "Placement"
  | "Timeline"
  | "Events"
  | "Climate"
  | "Finance"
  | "Visualization";

type Feature = {
  key: FeatureKey;
  label: string;
  value: number;
  colorVar: string;
  title: string;
  desc: string;
  bullets: string[];
};

const FEATURES: Feature[] = [
  {
    key: "Lifecycle",
    label: "Lifecycles",
    value: 16,
    colorVar: "--brand",
    title: "Plant lifecycles",
    desc: "Model how plants change from start to finish.",
    bullets: ["Time to grow", "Bloom time", "Time to wither"],
  },
  {
    key: "Placement",
    label: "Placement",
    value: 12,
    colorVar: "--mint",
    title: "3D placement",
    desc: "Build your layout and place plants in the engine.",
    bullets: ["Place plants in the scene", "House shape", "Garden shape"],
  },
  {
    key: "Timeline",
    label: "Timeline",
    value: 16,
    colorVar: "--sun",
    title: "Timeline travel",
    desc: "Move forward or backward and see the garden at any date.",
    bullets: ["Plant at this date", "Expected death date", "Time travel UI"],
  },
  {
    key: "Events",
    label: "Events",
    value: 10,
    colorVar: "--leaf-2",
    title: "Important events",
    desc: "Mark key moments per plant and track changes.",
    bullets: ["Milestones per plant", "Notes & markers", "Compare predicted vs actual"],
  },
  {
    key: "Climate",
    label: "Climate",
    value: 18,
    colorVar: "--brand",
    title: "Climate fit",
    desc: "Match plants to the environment you expect.",
    bullets: ["Temp tolerance", "Humidity tolerance", "Sun needed (stretch: soil nutrition)"],
  },
  {
    key: "Finance",
    label: "Finance",
    value: 14,
    colorVar: "--sun",
    title: "Financial insights",
    desc: "Estimate cost and value over time.",
    bullets: ["Cost for garden", "Cost per year", "Sell value at bloom & profit"],
  },
  {
    key: "Visualization",
    label: "Visuals",
    value: 14,
    colorVar: "--mint",
    title: "Visual simulation",
    desc: "See growth and withering as time changes.",
    bullets: ["Growth stages", "Withering stages", "Refresh after unexpected events"],
  },
];

function getCssVar(name: string): string {
  return getComputedStyle(document.documentElement).getPropertyValue(name).trim() || "#1bb673";
}

export default function FeaturePieChart(): JSX.Element {
  const [selected, setSelected] = useState<FeatureKey | null>(null);

  const active = useMemo(() => {
    if (!selected) return null;
    return FEATURES.find((f) => f.key === selected) ?? null;
  }, [selected]);

  const data = useMemo(
    () =>
      FEATURES.map((f) => ({
        id: f.key,
        label: f.label,
        value: f.value,
        color: getCssVar(f.colorVar),
      })),
    []
  );

  return (
    <div className={`fpShell ${selected ? "isSelected" : ""}`}>
      <div className="fpChartArea">
        <div className="fpChartCard">
          <div className="fpChartGlow" aria-hidden="true" />

          <div className="fpChartTop">
            <div className="fpChartTitle">Feature map</div>
            <div className="fpChartHint">
              <span className="fpDot" />
              Click a slice
            </div>
          </div>

          <div className="fpChartWrap">
            <div className="fpNivo">
              <ResponsivePie
                data={data}
                colors={(d) => (d.data as any).color}
                margin={{ top: 22, right: 22, bottom: 22, left: 22 }}
                innerRadius={0}          // pie (not donut)
                padAngle={1.3}
                cornerRadius={6}
                activeOuterRadiusOffset={10}
                borderWidth={1}
                borderColor={{ from: "color", modifiers: [["darker", 0.2]] }}
                enableArcLinkLabels={false}
                arcLabelsSkipAngle={6}
                arcLabelsTextColor="rgba(255,255,255,0.92)"
                arcLabel={(d) => d.label as string}
                theme={{
                  labels: { text: { fontSize: 12, fontWeight: 900 } },
                  tooltip: { container: { background: "rgba(10,12,14,0.92)", color: "white" } },
                }}
                onClick={(d) => setSelected(d.id as FeatureKey)}
              />
            </div>
          </div>

          <div className="fpBubbles">
            {FEATURES.map((f) => (
              <button
                key={f.key}
                type="button"
                className={`fpBubble ${selected === f.key ? "active" : ""}`}
                onClick={() => setSelected(f.key)}
              >
                <span className="fpBubbleDot" style={{ background: getCssVar(f.colorVar) }} />
                {f.label}
              </button>
            ))}
          </div>
        </div>
      </div>

      <div className={`fpPanel ${selected ? "open" : ""}`} aria-hidden={!selected}>
        {active && (
          <div className="fpPanelInner">
            <div className="fpPanelTop">
              <div>
                <div className="fpPanelKicker">Selected feature</div>
                <div className="fpPanelTitle">{active.title}</div>
              </div>

              <button className="fpClose" type="button" onClick={() => setSelected(null)} aria-label="Close">
                ✕
              </button>
            </div>

            <div className="fpPanelDesc">{active.desc}</div>

            <ul className="fpPanelList">
              {active.bullets.map((b) => (
                <li key={b}>{b}</li>
              ))}
            </ul>

            <div className="fpPanelNote">Close to re-center the chart.</div>
          </div>
        )}
      </div>
    </div>
  );
}
