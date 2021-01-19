#ifndef FLAGS_H
#define FLAGS_H

template<class T = uint8_t>
class Flags {
 public:
  Flags() {
  }
  ~Flags() {
  }

  void ResetFlags() {
    flags_ = 0;
  }

  void SetFlagIf(T flag_mask, bool is_true) {
    flags_ |= (-static_cast<T>(is_true) & flag_mask);
  }
  void SetFlag(T flag_mask) {
    flags_ |= flag_mask;
  }
  void UnsetFlag(T flag_mask) {
    flags_ &= ~flag_mask;
  }
  bool GetFlag(T flag_mask) const {
    return flags_ & flag_mask;
  }

 private:
  T flags_;
};

#endif
