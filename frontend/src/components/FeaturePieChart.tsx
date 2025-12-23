// src/components/FeaturePieChart.tsx
import * as React from "react";
import { PieChart, pieArcLabelClasses } from "@mui/x-charts/PieChart";
import { useDrawingArea } from "@mui/x-charts/hooks";
import { styled } from "@mui/material/styles";
import type { Theme } from "@mui/material/styles";

export interface FeatureDatum {
  id: string;
  label: string;
  color: string;
}

const StyledText = styled("text")(() => ({
  fill: "#ffffff",
  textAnchor: "middle",
  dominantBaseline: "central",
  fontSize: 18,
}));

interface PieCenterLabelProps {
  children: React.ReactNode;
}

function PieCenterLabel({ children }: PieCenterLabelProps): React.ReactElement {
  const { width, height, left, top } = useDrawingArea();
  return (
    <StyledText x={left + width / 2} y={top + height / 2}>
      {children}
    </StyledText>
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

  // Equal slice values for now
  const data = features.map((f) => ({
    id: f.id,
    label: f.label,
    value: 1,
    color: f.color,
  }));

  const total = data.reduce((acc, d) => acc + d.value, 0);

  const innerRadius = 70;
  const outerRadius = 170;

  return (
    <div className="leafy-feature-chart-shell">
      <PieChart
        height={400}
        series={[
          {
            innerRadius,
            outerRadius,
            data,
            arcLabel: (item) => (item.label as string) ?? "",
            valueFormatter: ({ value }) =>
              `${value} of ${total} feature${total === 1 ? "" : "s"}`,
            highlightScope: { fade: "global", highlight: "item" },
            highlighted: { additionalRadius: 3 },
            cornerRadius: 3,
          },
        ]}
        sx={{
          [`& .${pieArcLabelClasses.root}`]: {
            fontSize: "11px",
            fill: "#ffffff",
          },
          "& .MuiChartsPieArc-root": {
            cursor: "pointer",
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
        {/* Center label: “Features” */}
        <PieCenterLabel>Features</PieCenterLabel>
      </PieChart>
    </div>
  );
}
