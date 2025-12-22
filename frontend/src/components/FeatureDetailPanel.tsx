// src/components/FeatureDetailPanel.tsx
import React from "react";
import { FeatureCategoryId } from "./FeaturePieChart";

interface FeatureDetailPanelProps {
  selectedId: FeatureCategoryId | null;
}

const FeatureDetailPanel: React.FC<FeatureDetailPanelProps> = ({
  selectedId,
}) => {
  if (!selectedId) {
    return (
      <div className="leafy-feature-side-panel">
        Click a segment in the chart to explore how LeafyLedger thinks about{" "}
        <strong>plant lifecycles</strong>, <strong>climate risk</strong>,{" "}
        <strong>layouts</strong>, and <strong>profitability</strong>.
      </div>
    );
  }

  const sections: Record<
    FeatureCategoryId,
    {
      title: string;
      bullets: string[];
    }
  > = {
    lifecycle: {
      title: "Lifecycle Intelligence",
      bullets: [
        "Database of plant lifetime averages, time to grow, bloom, and wither.",
        "Mark and visualize key dates: planting, peak bloom, and expected decline.",
        "Compare different species to see which ones align with your ideal garden window.",
      ],
    },
    timeline: {
      title: "Timeline & Simulation",
      bullets: [
        "Travel forward or backward in time to see how your garden evolves.",
        "Mark important events per plant (plant at this date, expected death date).",
        "Refresh simulations when conditions change to keep your plan realistic.",
      ],
    },
    climate: {
      title: "Climate & Resilience",
      bullets: [
        "Store tolerated temperatures, humidity ranges, and sun needs per plant.",
        "Use user-input climate info and expected temperatures and humidities.",
        "Adapt to unexpected events like floods and see how your garden responds.",
      ],
    },
    finance: {
      title: "Financial Insight",
      bullets: [
        "Estimate cost for the garden and yearly costs to maintain it.",
        "Calculate sell value at bloom (perfect for lumber or cash crops).",
        "Track yearly profit as sell value minus cost of seeds and upkeep.",
      ],
    },
    layout: {
      title: "Layout & Experience",
      bullets: [
        "Configure house and garden shapes to match real or planned spaces.",
        "Place plants inside the engine and see a visual representation over time.",
        "Combine layout with climate and lifecycle data to plan truly resilient designs.",
      ],
    },
  };

  const section = sections[selectedId];

  return (
    <div className="leafy-feature-side-panel">
      <strong>{section.title}</strong>
      <ul style={{ margin: "0.5rem 0 0 1rem", padding: 0 }}>
        {section.bullets.map((b) => (
          <li key={b} style={{ marginBottom: "0.25rem" }}>
            {b}
          </li>
        ))}
      </ul>
    </div>
  );
};

export default FeatureDetailPanel;
