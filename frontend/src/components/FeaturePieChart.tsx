// src/components/FeaturePieChart.tsx
import React from "react";
import {
  PieChart,
  Pie,
  Cell,
  Tooltip,
  ResponsiveContainer,
} from "recharts";

export type FeatureCategoryId =
  | "lifecycle"
  | "timeline"
  | "climate"
  | "finance"
  | "layout";

export interface FeatureCategory {
  id: FeatureCategoryId;
  label: string;
  value: number;
  accentColor: string;
}

interface FeaturePieChartProps {
  data: FeatureCategory[];
  selectedId: FeatureCategoryId | null;
  onSelect: (id: FeatureCategoryId) => void;
}

const FeaturePieChart: React.FC<FeaturePieChartProps> = ({
  data,
  selectedId,
  onSelect,
}) => {
  const handleClick = (entry: any) => {
    onSelect(entry.id as FeatureCategoryId);
  };

  return (
    <div style={{ width: "100%", height: 210 }}>
      <ResponsiveContainer>
        <PieChart>
          <Pie
            data={data}
            dataKey="value"
            nameKey="label"
            innerRadius={60}
            outerRadius={90}
            paddingAngle={3}
            onClick={handleClick}
          >
            {data.map((entry) => (
              <Cell
                key={entry.id}
                fill={entry.accentColor}
                stroke="#020617"
                strokeWidth={selectedId === entry.id ? 3 : 1}
                style={{ cursor: "pointer" }}
              />
            ))}
          </Pie>
          <Tooltip
            contentStyle={{
              backgroundColor: "#020617",
              border: "1px solid rgba(148, 163, 184, 0.6)",
              borderRadius: 10,
              fontSize: "0.8rem",
            }}
            formatter={(_, __, item) => [item?.payload?.label]}
          />
        </PieChart>
      </ResponsiveContainer>
    </div>
  );
};

export default FeaturePieChart;
