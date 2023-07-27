#pragma once

namespace mcview {

class VisibleRegions {
public:
  void add(int rx, int rz) {
    if (fMinRx && fMinRz && fMaxRx && fMaxRz) {
      fMinRx = std::min(*fMinRx, rx);
      fMaxRx = std::max(*fMaxRx, rx);
      fMinRz = std::min(*fMinRz, rz);
      fMaxRz = std::max(*fMaxRz, rz);
    } else {
      fMinRx = rx;
      fMaxRx = rx;
      fMinRz = rz;
      fMaxRz = rz;
    }
  }

  int getX() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMinRx;
    }
  }

  int getY() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMinRz;
    }
  }

  int getRight() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMaxRx;
    }
  }

  int getBottom() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMaxRz;
    }
  }

  int getWidth() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMaxRx - *fMinRx + 1;
    }
  }

  int getHeight() const {
    if (isEmpty()) {
      return 0;
    } else {
      return *fMaxRz - *fMinRz + 1;
    }
  }

private:
  bool isEmpty() const {
    return !fMinRx || !fMinRz || !fMaxRx || !fMaxRz;
  }

private:
  std::optional<int> fMinRx, fMinRz, fMaxRx, fMaxRz;
};

} // namespace mcview
