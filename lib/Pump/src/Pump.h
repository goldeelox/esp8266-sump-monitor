class Pump {
public:
  const char* name;
  int pin;
  int usageSeconds;
  int usageCount;
  void updateState(int state);
  Pump(const char* name, const int pin) {
    this->name = name;
    this->pin = pin;
    usageCount = 0;
    usageSeconds = 0;
  }

private:
  bool active;
  void inc(int sec);
};
