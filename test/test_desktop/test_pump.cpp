#include <Pump.h>
#include <gtest/gtest.h>

TEST(PumpTest, ActiveForConsecutiveIntervals) {
  int expectedUsageCount = 1;
  int expectedUsageSeconds = 5;

  int intervals = 5;
  Pump p;
  for (int count = 0; count < intervals; count++) {
    p.updateState(1);
  }

  EXPECT_EQ(p.usageCount, expectedUsageCount);
  EXPECT_EQ(p.usageSeconds, expectedUsageSeconds);
}

TEST(PumpTest, ActiveForSomeIntervals) {
  int expectedUsageCount = 3;
  int expectedUsageSeconds = 6;

  Pump p;
  p.updateState(1);
  p.updateState(1);
  p.updateState(0);
  p.updateState(0);
  p.updateState(1);
  p.updateState(0);
  p.updateState(1);
  p.updateState(1);
  p.updateState(1);
  p.updateState(0);

  EXPECT_EQ(p.usageCount, expectedUsageCount);
  EXPECT_EQ(p.usageSeconds, expectedUsageSeconds);
}

TEST(PumpTest, NotActive) {
  int expectedUsageCount = 0;
  int expectedUsageSeconds = 0;

  int intervals = 10;
  Pump p;
  p.updateState(0);

  EXPECT_EQ(p.usageCount, expectedUsageCount);
  EXPECT_EQ(p.usageSeconds, expectedUsageSeconds);
}

#if defined(ARDUINO)
#include <Arduino.h>

void setup() {
  Serial.begin(115200);

  ::testing::InitGoogleTest();
}

void loop() {
  if (RUN_ALL_TESTS())
    ;
  delay(1000);
}

#else

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (RUN_ALL_TESTS())
    ;
  return 0;
}

#endif
