class Pump {
public:
  int usageSeconds;
  int usageCount;
  void updateState(int state);
  Pump() {
    usageCount = 0;
    usageSeconds = 0;
  }

private:
  bool active;
  void inc(int sec);
};
