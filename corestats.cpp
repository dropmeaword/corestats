#include <corestats.h>

#include <Arduino.h>
#include <limits.h>
#include <math.h>
#include <float.h>

// //////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////

float Stats::stddev() const {
  return sqrt(var());
}

float Stats::normalize(float value) const {
  return ( value - mean() ) / (stddev() + FLT_MIN);
}


SimpleStats::SimpleStats(float startMean, float startVar) {
  reset(startMean, startVar);
}

void SimpleStats::reset(float startMean, float startVar) {
  _mean = startMean;
  _mean2 = startVar + sq(_mean);
  _nSamples = 0;
//  _min = FLT_MAX;
//  _max = -FLT_MAX;
}

float SimpleStats::update(float value) {
  if (_nSamples == ULONG_MAX)
    _nSamples = (ULONG_MAX / 4) * 3; // simple trick that makes sure we don't overflow

  // Add one to number of samples
  _nSamples++;

  // Update mean and mean2
  float prop = (float)(_nSamples-1) / (float)_nSamples;
  _mean  = _mean  * prop + value     / _nSamples;
  _mean2 = _mean2 * prop + sq(value) / _nSamples;

  // Update min and max
//  _min = min(_min, value);
//  _max = max(_max, value);

  return normalize(value);
}

float SimpleStats::var() const {
  float v = (_mean2 - sq(_mean));
  return max(v, (float)0); // make sure the result is >= 0
}

// //////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////

MovingAverage::MovingAverage(float alphaOrN) : _value(0.5f) {
  setAlphaOrN(alphaOrN);
  reset();
}

MovingAverage::MovingAverage(float alphaOrN, float startValue) : _value(startValue) {
  setAlphaOrN(alphaOrN);
}

void MovingAverage::setAlphaOrN(float alphaOrN)
{
  alphaOrN =  max(alphaOrN, 0); // make sure factor >= 0
  _alpha = (alphaOrN > 1 ?
      2 / (alphaOrN + 1) :
      alphaOrN);
}

void MovingAverage::reset() {
  _setStarted(false);
}

void MovingAverage::reset(float startValue) {
  _value = startValue;
  _setStarted(true);
}

float MovingAverage::update(float v) {
  if (!isStarted()) {
    _value = v;
    _setStarted(true); // start
    return _value;
  }
  else
    return (_value -= _alpha * (_value - v));
}

bool MovingAverage::isStarted() const {
  return _alpha >= 0;
}

void MovingAverage::_setStarted(bool start) {
  _alpha = (start ? +1 : -1) * abs(_alpha);
}

MovingStats::MovingStats(float alphaOrN) : avg(alphaOrN) { }
MovingStats::MovingStats(float alphaOrN, float startMean, float startVar)
  : avg(alphaOrN, startMean), _var(startVar) {
  reset(startMean, startVar);
}

void MovingStats::reset() {
  avg.reset();
  _var = 0;
}

void MovingStats::reset(float startMean, float startVar) {
  avg.reset(startMean);
  _var = startVar;
}

float MovingStats::update(float value)
 {
  avg.update(value);
  if (!isStarted())
    _var = 0;
  else {
    float diff = value - avg.get();
   _var   -= avg.alpha() * (_var - sq(diff));
  }

  return normalize(value);
}

bool MovingStats::isStarted() const {
  return avg.isStarted();
}

// //////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////

AdaptiveNormalizer::AdaptiveNormalizer(float smoothFactor)
  : MovingStats(smoothFactor),
    _value(0.5f),
    _mean(0.5f),
    _stddev(0.25f)
{}

AdaptiveNormalizer::AdaptiveNormalizer(float mean, float stddev, float smoothFactor)
	: MovingStats(smoothFactor),
    _value(mean),
    _mean(mean),
    _stddev(abs(stddev))
{}

float AdaptiveNormalizer::put(float value) {
  return (_value = MovingStats::update(value) * _stddev + _mean);
}

Normalizer::Normalizer()
  : SimpleStats(),
    _value(0.5f),
    _mean(0.5f),
    _stddev(0.25f)
{}

Normalizer::Normalizer(float mean, float stddev)
	: SimpleStats(),
    _value(mean),
    _mean(mean),
    _stddev(abs(stddev))
{}

float Normalizer::put(float value) {
  return (_value = SimpleStats::update(value) * _stddev + _mean);
}

MinMaxScaler::MinMaxScaler()
 : _value(0.5f),
   _minValue(FLT_MAX),
   _maxValue(FLT_MIN)
{}

float MinMaxScaler::put(float value)
{
  _minValue = min(value, _minValue);
  _maxValue = max(value, _maxValue);
  _value = (_minValue == _maxValue ? 0.5f : map(value, _minValue, _maxValue, 0.0f, 1.0f));
	return _value;
}

