// Aseprite
// Copyright (C) 2001-2017  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifndef APP_UI_COLOR_SLIDERS_H_INCLUDED
#define APP_UI_COLOR_SLIDERS_H_INCLUDED
#pragma once

#include "app/color.h"
#include "obs/signal.h"
#include "ui/event.h"
#include "ui/grid.h"
#include "ui/widget.h"

#include <vector>

namespace ui {
  class Label;
  class Slider;
  class Entry;
}

namespace app {

  class ColorSlidersChangeEvent;

  class ColorSliders : public ui::Widget {
  public:
    enum Channel { Red, Green, Blue,
                   HsvHue, HsvSaturation, HsvValue,
                   HslHue, HslSaturation, HslLightness,
                   Gray, Alpha };
    enum Mode { Absolute, Relative };

    ColorSliders();
    ~ColorSliders();

    void setColor(const app::Color& color);
    void setMode(Mode mode);
    void resetRelativeSliders();

    int getAbsSliderValue(int sliderIndex) const;
    int getRelSliderValue(int sliderIndex) const;

    // Signals
    obs::signal<void(ColorSlidersChangeEvent&)> ColorChange;

  protected:
    void onSizeHint(ui::SizeHintEvent& ev) override;

    // For derived classes
    void addSlider(const Channel channel,
                   const char* labelText,
                   const int absMin, const int absMax,
                   const int relMin, const int relMax);
    void setAbsSliderValue(int sliderIndex, int value);

    virtual void onSetColor(const app::Color& color) = 0;
    virtual app::Color getColorFromSliders() = 0;

  private:
    void onSliderChange(int i);
    void onEntryChange(int i);
    void onControlChange(int i);

    void updateEntryText(int entryIndex);
    void updateSlidersBgColor(const app::Color& color);
    void updateSliderBgColor(ui::Slider* slider, const app::Color& color);

    std::vector<ui::Label*> m_label;
    std::vector<ui::Slider*> m_absSlider;
    std::vector<ui::Slider*> m_relSlider;
    std::vector<ui::Entry*> m_entry;
    std::vector<Channel> m_channel;
    ui::Grid m_grid;
    Mode m_mode;
    int m_lockEntry;
  };

  //////////////////////////////////////////////////////////////////////
  // Derived-classes

  class RgbSliders : public ColorSliders {
  public:
    RgbSliders();

  private:
    virtual void onSetColor(const app::Color& color) override;
    virtual app::Color getColorFromSliders() override;
  };

  class HsvSliders : public ColorSliders {
  public:
    HsvSliders();

  private:
    virtual void onSetColor(const app::Color& color) override;
    virtual app::Color getColorFromSliders() override;
  };

  class HslSliders : public ColorSliders {
  public:
    HslSliders();

  private:
    virtual void onSetColor(const app::Color& color) override;
    virtual app::Color getColorFromSliders() override;
  };

  class GraySlider : public ColorSliders {
  public:
    GraySlider();

  private:
    virtual void onSetColor(const app::Color& color) override;
    virtual app::Color getColorFromSliders() override;
  };

  //////////////////////////////////////////////////////////////////////
  // Events

  class ColorSlidersChangeEvent : public ui::Event {
  public:
    ColorSlidersChangeEvent(ColorSliders::Channel channel,
                            ColorSliders::Mode mode,
                            const app::Color& color,
                            int delta,
                            ui::Component* source)
      : Event(source)
      , m_channel(channel)
      , m_mode(mode)
      , m_color(color)
      , m_delta(delta) {
    }

    ColorSliders::Channel channel() const { return m_channel; }
    ColorSliders::Mode mode() const { return m_mode; }
    app::Color color() const { return m_color; }
    int delta() const { return m_delta; }

  private:
    ColorSliders::Channel m_channel;
    ColorSliders::Mode m_mode;
    app::Color m_color;
    int m_delta;
  };

} // namespace app

#endif
