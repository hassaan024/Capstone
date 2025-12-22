// src/components/FeatureCarousel.tsx
import React, { useState } from "react";
import {
  Carousel,
  CarouselItem,
  CarouselControl,
  CarouselIndicators,
  CarouselCaption,
} from "reactstrap";

interface FeatureSlide {
  key: string;
  title: string;
  caption: string;
}

const items: FeatureSlide[] = [
  {
    key: "slide-1",
    title: "Plan Around Bloom Windows",
    caption:
      "See which plants need to be seeded when so your entire garden peaks together.",
  },
  {
    key: "slide-2",
    title: "Stress-Test with Climate Events",
    caption:
      "Add floods or heatwaves to watch how a garden responds before you spend a dollar.",
  },
  {
    key: "slide-3",
    title: "Treat Your Garden Like a Portfolio",
    caption:
      "Combine yield, cost, and risk to understand which plantings actually pay off.",
  },
];

const FeatureCarousel: React.FC = () => {
  const [activeIndex, setActiveIndex] = useState(0);
  const [animating, setAnimating] = useState(false);

  const next = () => {
    if (animating) return;
    const nextIndex = activeIndex === items.length - 1 ? 0 : activeIndex + 1;
    setActiveIndex(nextIndex);
  };

  const previous = () => {
    if (animating) return;
    const nextIndex = activeIndex === 0 ? items.length - 1 : activeIndex - 1;
    setActiveIndex(nextIndex);
  };

  const goToIndex = (newIndex: number) => {
    if (animating) return;
    setActiveIndex(newIndex);
  };

  return (
    <Carousel activeIndex={activeIndex} next={next} previous={previous}>
      <CarouselIndicators
        items={items}
        activeIndex={activeIndex}
        onClickHandler={goToIndex}
      />
      {items.map((item) => (
        <CarouselItem
          onExiting={() => setAnimating(true)}
          onExited={() => setAnimating(false)}
          key={item.key}
        >
          <div style={{ padding: "1.5rem 0.75rem" }}>
            <CarouselCaption
              captionText={item.caption}
              captionHeader={item.title}
            />
          </div>
        </CarouselItem>
      ))}
      <CarouselControl
        direction="prev"
        directionText="Previous"
        onClickHandler={previous}
      />
      <CarouselControl
        direction="next"
        directionText="Next"
        onClickHandler={next}
      />
    </Carousel>
  );
};

export default FeatureCarousel;
