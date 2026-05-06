// src/components/FeaturePieChart.tsx
import * as React from "react";
import { PieChart, pieArcLabelClasses } from "@mui/x-charts/PieChart";
import { useDrawingArea } from "@mui/x-charts/hooks";
import { styled } from "@mui/material/styles";

export interface FeatureDatum {
  id: string;
  label: string;
  color: string;
}

const CenterTitle = styled("text")(() => ({
  fill: "rgba(255,255,255,0.95)",
  textAnchor: "middle",
  dominantBaseline: "central",
  fontSize: 13,
  fontWeight: 700,
  letterSpacing: "0.04em",
  textTransform: "uppercase",
}));

const CenterSub = styled("text")(() => ({
  fill: "rgba(148,163,184,0.8)",
  textAnchor: "middle",
  dominantBaseline: "central",
  fontSize: 10,
  letterSpacing: "0.06em",
}));

function PieCenterLabel(): React.ReactElement {
  const { width, height, left, top } = useDrawingArea();
  const cx = left + width / 2;
  const cy = top + height / 2;

  return (
    <g>
      <circle cx={cx} cy={cy} r={52} fill="none" stroke="rgba(148,163,184,0.15)" strokeWidth={1.5} />
      <CenterTitle x={cx} y={cy - 8}>FEATURES</CenterTitle>
      <CenterSub x={cx} y={cy + 12}>tap a slice</CenterSub>
    </g>
  );
}

interface FeaturePieChartProps {
  features: FeatureDatum[];
  selectedId: string;
  onSelect: (id: string) => void;
}

export default function FeaturePieChart(
  props: FeaturePieChartProps,
): React.ReactElement {
  const { features, selectedId, onSelect } = props;

  const innerRadius = 52;
  const outerRadius = 182;
  const selectedOuterRadius = outerRadius + 14;

  const data = features.map((f) => ({
    id: f.id,
    label: f.label,
    value: 1,
    color: f.color,
    outerRadius: f.id === selectedId ? selectedOuterRadius : outerRadius,
  }));

  const total = data.reduce((acc, d) => acc + d.value, 0);

  return (
    <div className="leafy-feature-chart-shell">
      <PieChart
        height={440}
        series={[
          {
            innerRadius,
            paddingAngle: 3,
            cornerRadius: 9,
            data,
            arcLabel: (item) => (item.label as string) ?? "",
            valueFormatter: ({ value }) =>
              `${value} of ${total} feature${total === 1 ? "" : "s"}`,
            highlightScope: { fade: "none", highlight: "none" },
          },
        ]}
        sx={{
          [`& .${pieArcLabelClasses.root}`]: {
            fontSize: "11px",
            fontWeight: "600",
            fill: "#ffffff",
            filter: "drop-shadow(0 1px 2px rgba(0,0,0,0.8))",
          },
          "& .MuiChartsPieArc-root": {
            cursor: "pointer",
            filter: "drop-shadow(0 2px 8px rgba(0,0,0,0.4))",
            transition: "filter 200ms ease",
          },
          "& .MuiChartsPieArc-root:hover": {
            filter: "drop-shadow(0 4px 16px rgba(0,0,0,0.6))",
          },
        }}
        hideLegend
        onItemClick={(_event, params: any) => {
          const index = params?.dataIndex as number;
          if (Number.isInteger(index) && index >= 0 && index < data.length) {
            onSelect(data[index].id);
          }
        }}
      >
        <PieCenterLabel />
      </PieChart>
    </div>
  );
}
