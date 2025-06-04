#ifndef COMMAND_TEST_H
#define COMMAND_TEST_H

class CommandTest : public Gauge {
public:
  CommandTest(TFT_eSPI* display) : 
    Gauge(display),
    rpm(display)
    {}

  void initialize() override {
    display->fillScreen(DISPLAY_BG_COLOR);

    // Calculate gauge center and outline position
    rpm.setColorDepth(1);
    rpm.setTextFont(1);
    rpm.setTextSize(4);
    rpm.createSprite(rpm.textWidth("7777"), rpm.fontHeight());
    rpm.setCursor((rpm.width() - rpm.textWidth("0")) / 2, (rpm.height() - rpm.fontHeight()) / 2);
    rpm.println("0");
    rpm.pushSprite((DISPLAY_WIDTH - rpm.width()) / 2, 100);
  }

  void render(double val) override {
    renderTest(val);
  }

  void displayStats(float fps, double frameAvg, double queryAvg) override {
    display->setTextColor(TFT_WHITE, DISPLAY_BG_COLOR);
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->printf("FPS: %.1f\nFrame: %.1f ms\nQuery: %.1f ms", fps, frameAvg, queryAvg);
  }

private:
  TFT_eSprite rpm;

  void renderTest(double val) {
    String value = String((int) val);
    rpm.fillSprite(TFT_BLACK);
    rpm.setCursor((rpm.width() - rpm.textWidth(value)) / 2, (rpm.height() - rpm.fontHeight()) / 2);
    rpm.println(value);
    rpm.pushSprite((DISPLAY_WIDTH - rpm.width()) / 2, 100);
  }
};





#endif