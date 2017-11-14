#ifndef __CORESTATS_H__
#define __CORESTATS_H__

class Stats {
public:
  virtual ~Stats() {}

  /// Resets the statistics.
  virtual void reset(float startMean=0, float startVar=0) = 0;

  /// Adds a value to the statistics (returns the mean).
  virtual float update(float value) = 0;

  /// The statistics.
  virtual float mean() const = 0;
  virtual float var() const  = 0;
  virtual float stddev() const;

  /// Returns the normalized value according to the computed statistics (mean and variance).
  float normalize(float value) const;
};

class SimpleStats : public Stats {
public:
  float _mean;
  float _mean2; // mean of squared values

  unsigned long _nSamples;

  /// ctor
  SimpleStats(float startMean=0, float startVar=0);
  virtual ~SimpleStats() {}

  /// Resets the statistics.
  virtual void reset(float startMean=0, float startVar=0);

  /// Adds a value to the statistics (returns the mean).
  virtual float update(float value);

  /// The statistics.
  virtual float mean() const { return _mean; }
  // The var() and stddev() are the population (ie. not the sample) variance and standard dev, so technically
  // they should be readjusted by multiplying it by _nSamples / (_nSamples-1). But with a lot of samples the
  // difference vanishes and we priviledged less floating points computations over precision.
  virtual float var() const;
};

/// An exponential moving average class.
class MovingAverage {
public:
  // The alpha (mixing) variable (in [0,1]).
  float _alpha;

  // The current value of the exponential moving average.
  float _value;

  /**
   * Constructs the moving average, starting with #startValue# as its value. The #alphaOrN# argument
   * has two options:
   * - if <= 1 then it's used directly as the alpha value
   * - if > 1 then it's used as the "number of items that are considered from the past" (*)
   * (*) Of course this is an approximation. It actually sets the alpha value to 2 / (n - 1)
   */
  MovingAverage(float alphaOrN=1);
  MovingAverage(float alphaOrN, float startValue);
  virtual ~MovingAverage() {}

  /// Change the smoothing factor to #alphaOrN#.
  void setAlphaOrN(float alphaOrN);

  /// Resets the moving average.
  void reset();

  /// Resets the moving average to #startValue#.
  void reset(float startValue);

  /// Updates the moving average with new value #v# (also returns the current value).
  float update(float v);

  /// Returns the value of the moving average. This is undefined if isValid() == false.
  float get() const { return _value; }

  /// Returns true iff the moving average has already been started.
  bool isStarted() const;

  /// Returns the alpha value.
  float alpha() const { return _alpha; }

protected:
  void _setStarted(bool start);
};

class MovingStats : public Stats {
public:
  MovingAverage avg;
  float _var;

  /**
   * Constructs the moving statistics, starting with #startMean# and #startVar# as initial mean and
   * variance. The #alphaOrN# argument has two options:
   * - if <= 1 then it's used directly as the alpha value
   * - if > 1 then it's used as the "number of items that are considered from the past" (*)
   * (*) Of course this is an approximation. It actually sets the alpha value to 2 / (n - 1)
   */
  MovingStats(float alphaOrN=1);
  MovingStats(float alphaOrN, float startMean, float startVar);
  virtual ~MovingStats() {}

  /// Resets the statistics.
  virtual void reset();

  /// Resets the statistics.
  virtual void reset(float startMean, float startVar);

  /// Adds a value to the statistics (returns the mean).
  virtual float update(float value);

  /// The statistics.
  virtual float mean() const { return avg.get(); }
  virtual float var() const { return _var; }

  virtual bool isStarted() const;
};


/// Adaptive normalizer: normalizes values on-the-run using exponential moving
/// averages over mean and stddev.
class AdaptiveNormalizer : public MovingStats {
public:
  AdaptiveNormalizer(float smoothFactor=0.001f);
  AdaptiveNormalizer(float mean, float stddev, float smoothFactor=0.001f);
  virtual ~AdaptiveNormalizer() {}

  void setMean(float mean) { _mean = mean; }
  void setStddev(float stddev) { _stddev = stddev; };

  virtual float put(float value);

  virtual float get() { return _value; }

  float _value;
  float _mean;
  float _stddev;
};

/// Standard normalizer: normalizes values on-the-run using real mean and stddev.
class Normalizer : public SimpleStats {
public:
  Normalizer();
  Normalizer(float mean, float stddev);
  virtual ~Normalizer() {}

  void setMean(float mean) { _mean = mean; }
  void setStddev(float stddev) { _stddev = stddev; };

  virtual float put(float value);

  virtual float get() { return _value; }

  float _value;
  float _mean;
  float _stddev;
};

/// Regularizes signal into [0,1] by rescaling it using the min and max values.
class MinMaxScaler {
public:
  MinMaxScaler();
  virtual ~MinMaxScaler() {}

  virtual float put(float value);

  virtual float get() { return _value; }

  float _value;
  float _minValue;
  float _maxValue;
};

#endif // __CORESTATS_H__