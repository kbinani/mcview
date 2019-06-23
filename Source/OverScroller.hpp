/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <chrono>

class AnimationUtils {
public:
    static long currentAnimationTimeMillis() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
    }
};

template <typename T>
T signum(T val) {
    return (T(0) < val) - (val < T(0));
}

class OverScroller;

class SplineOverScroller {
    friend class OverScroller;
protected:
    // Initial position
    int mStart;

    // Current position
    int mCurrentPosition;

    // Final position
    int mFinal;

    // Initial velocity
    int mVelocity;

    // Current velocity
    float mCurrVelocity;

    // Constant current deceleration
    float mDeceleration;

    // Animation starting time, in system milliseconds
    long mStartTime;

    // Animation duration, in milliseconds
    int mDuration;

    // Duration to complete spline component of animation
    int mSplineDuration;

    // Distance to travel along spline animation
    int mSplineDistance;

    // Whether the animation is currently in progress
    bool mFinished;

    // The allowed overshot distance before boundary is reached.
    int mOver;

    // Fling friction
    // ViewConfiguration.getScrollFriction();
    float mFlingFriction = 0.015f;

    // Current state of the animation.
    int mState = SPLINE;

    // Constant gravity value, used in the deceleration phase.
    static float constexpr GRAVITY = 2000.0f;

    // A context-specific coefficient adjusted to physical values.
    float mPhysicalCoeff;

    float DECELERATION_RATE;
    static float constexpr INFLEXION = 0.35f; // Tension lines cross at (INFLEXION, 1)
    static float constexpr START_TENSION = 0.5f;
    static float constexpr END_TENSION = 1.0f;
    static float constexpr P1 = START_TENSION * INFLEXION;
    static float constexpr P2 = 1.0f - END_TENSION * (1.0f - INFLEXION);

    static int constexpr NB_SAMPLES = 100;
    float SPLINE_POSITION[NB_SAMPLES + 1];
    float SPLINE_TIME[NB_SAMPLES + 1];

    static int constexpr SPLINE = 0;
    static int constexpr CUBIC = 1;
    static int constexpr BALLISTIC = 2;

public:
    void setFriction(float friction) {
        mFlingFriction = friction;
    }

    SplineOverScroller() {
        mFinished = true;
        const float ppi = 160.0f;
        const float gravityEarth = 9.80665; // GRAVITY_EARTH (m/s^2)
        const float inchPerMeter = 39.37f; // inch/meter
        const float friction = 0.84f; // look and feel tuning
        mPhysicalCoeff = gravityEarth * inchPerMeter * ppi * friction;
        DECELERATION_RATE = (float) (log(0.78) / log(0.9));

        float x_min = 0.0f;
        float y_min = 0.0f;
        for (int i = 0; i < NB_SAMPLES; i++) {
            const float alpha = (float) i / NB_SAMPLES;

            float x_max = 1.0f;
            float x, tx, coef;
            while (true) {
                x = x_min + (x_max - x_min) / 2.0f;
                coef = 3.0f * x * (1.0f - x);
                tx = coef * ((1.0f - x) * P1 + x * P2) + x * x * x;
                if (fabsf(tx - alpha) < 1E-5) break;
                if (tx > alpha) x_max = x;
                else x_min = x;
            }
            SPLINE_POSITION[i] = coef * ((1.0f - x) * START_TENSION + x) + x * x * x;

            float y_max = 1.0f;
            float y, dy;
            while (true) {
                y = y_min + (y_max - y_min) / 2.0f;
                coef = 3.0f * y * (1.0f - y);
                dy = coef * ((1.0f - y) * START_TENSION + y) + y * y * y;
                if (fabsf(dy - alpha) < 1E-5) break;
                if (dy > alpha) y_max = y;
                else y_min = y;
            }
            SPLINE_TIME[i] = coef * ((1.0f - y) * P1 + y * P2) + y * y * y;
        }
        SPLINE_POSITION[NB_SAMPLES] = SPLINE_TIME[NB_SAMPLES] = 1.0f;
    }

    void updateScroll(float q) {
        mCurrentPosition = mStart + round(q * (mFinal - mStart));
    }

private:
    /*
     * Get a signed deceleration that will reduce the velocity.
     */
    static float getDeceleration(int velocity) {
        return velocity > 0 ? -GRAVITY : GRAVITY;
    }

    /*
     * Modifies mDuration to the duration it takes to get from start to newFinal using the
     * spline interpolation. The previous duration was needed to get to oldFinal.
     */
    void adjustDuration(int start, int oldFinal, int newFinal) {
        const int oldDistance = oldFinal - start;
        const int newDistance = newFinal - start;
        const float x = fabs((float) newDistance / oldDistance);
        const int index = (int) (NB_SAMPLES * x);
        if (index < NB_SAMPLES) {
            const float x_inf = (float) index / NB_SAMPLES;
            const float x_sup = (float) (index + 1) / NB_SAMPLES;
            const float t_inf = SPLINE_TIME[index];
            const float t_sup = SPLINE_TIME[index + 1];
            const float timeCoef = t_inf + (x - x_inf) / (x_sup - x_inf) * (t_sup - t_inf);
            mDuration *= timeCoef;
        }
    }

public:
    void startScroll(int start, int distance, int duration) {
        mFinished = false;

        mCurrentPosition = mStart = start;
        mFinal = start + distance;

        mStartTime = AnimationUtils::currentAnimationTimeMillis();
        mDuration = duration;

        // Unused
        mDeceleration = 0.0f;
        mVelocity = 0;
    }

    void finish() {
        mCurrentPosition = mFinal;
        // Not reset since WebView relies on this value for fast fling.
        // TODO: restore when WebView uses the fast fling implemented in this class.
        // mCurrVelocity = 0.0f;
        mFinished = true;
    }

    void setFinalPosition(int position) {
        mFinal = position;
        mFinished = false;
    }

    void extendDuration(int extend) {
        const long time = AnimationUtils::currentAnimationTimeMillis();
        const int elapsedTime = (int) (time - mStartTime);
        mDuration = elapsedTime + extend;
        mFinished = false;
    }

    bool springback(int start, int min, int max) {
        mFinished = true;

        mCurrentPosition = mStart = mFinal = start;
        mVelocity = 0;

        mStartTime = AnimationUtils::currentAnimationTimeMillis();
        mDuration = 0;

        if (start < min) {
            startSpringback(start, min, 0);
        } else if (start > max) {
            startSpringback(start, max, 0);
        }

        return !mFinished;
    }

private:
    void startSpringback(int start, int end, int /*velocity*/) {
        // mStartTime has been set
        mFinished = false;
        mState = CUBIC;
        mCurrentPosition = mStart = start;
        mFinal = end;
        const int delta = start - end;
        mDeceleration = getDeceleration(delta);
        // TODO take velocity into account
        mVelocity = -delta; // only sign is used
        mOver = abs(delta);
        mDuration = (int) (1000.0 * sqrt(-2.0 * delta / mDeceleration));
    }

public:
    void fling(int start, int velocity, int min, int max, int over) {
        mOver = over;
        mFinished = false;
        mCurrVelocity = mVelocity = velocity;
        mDuration = mSplineDuration = 0;
        mStartTime = AnimationUtils::currentAnimationTimeMillis();
        mCurrentPosition = mStart = start;

        if (start > max || start < min) {
            startAfterEdge(start, min, max, velocity);
            return;
        }

        mState = SPLINE;
        double totalDistance = 0.0;

        if (velocity != 0) {
            mDuration = mSplineDuration = getSplineFlingDuration(velocity);
            totalDistance = getSplineFlingDistance(velocity);
        }

        mSplineDistance = (int) (totalDistance * signum(velocity));
        mFinal = start + mSplineDistance;

        // Clamp to a valid final position
        if (mFinal < min) {
            adjustDuration(mStart, mFinal, min);
            mFinal = min;
        }

        if (mFinal > max) {
            adjustDuration(mStart, mFinal, max);
            mFinal = max;
        }
    }

private:
    double getSplineDeceleration(int velocity) {
        return log(INFLEXION * abs(velocity) / (mFlingFriction * mPhysicalCoeff));
    }

    double getSplineFlingDistance(int velocity) {
        const double l = getSplineDeceleration(velocity);
        const double decelMinusOne = DECELERATION_RATE - 1.0;
        return mFlingFriction * mPhysicalCoeff * exp(DECELERATION_RATE / decelMinusOne * l);
    }

    /* Returns the duration, expressed in milliseconds */
    int getSplineFlingDuration(int velocity) {
        const double l = getSplineDeceleration(velocity);
        const double decelMinusOne = DECELERATION_RATE - 1.0;
        return (int) (1000.0 * exp(l / decelMinusOne));
    }

    void fitOnBounceCurve(int start, int end, int velocity) {
        // Simulate a bounce that started from edge
        const float durationToApex = - velocity / mDeceleration;
        // The float cast below is necessary to avoid integer overflow.
        const float velocitySquared = (float) velocity * velocity;
        const float distanceToApex = velocitySquared / 2.0f / fabs(mDeceleration);
        const float distanceToEdge = abs(end - start);
        const float totalDuration = (float) sqrt(
                2.0 * (distanceToApex + distanceToEdge) / fabs(mDeceleration));
        mStartTime -= (int) (1000.0f * (totalDuration - durationToApex));
        mCurrentPosition = mStart = end;
        mVelocity = (int) (- mDeceleration * totalDuration);
    }

    void startBounceAfterEdge(int start, int end, int velocity) {
        mDeceleration = getDeceleration(velocity == 0 ? start - end : velocity);
        fitOnBounceCurve(start, end, velocity);
        onEdgeReached();
    }

    void startAfterEdge(int start, int min, int max, int velocity) {
        if (start > min && start < max) {
            mFinished = true;
            return;
        }
        const bool positive = start > max;
        const int edge = positive ? max : min;
        const int overDistance = start - edge;
        bool keepIncreasing = overDistance * velocity >= 0;
        if (keepIncreasing) {
            // Will result in a bounce or a to_boundary depending on velocity.
            startBounceAfterEdge(start, edge, velocity);
        } else {
            const double totalDistance = getSplineFlingDistance(velocity);
            if (totalDistance > abs(overDistance)) {
                fling(start, velocity, positive ? min : start, positive ? start : max, mOver);
            } else {
                startSpringback(start, edge, velocity);
            }
        }
    }

public:
    void notifyEdgeReached(int start, int end, int over) {
        // mState is used to detect successive notifications
        if (mState == SPLINE) {
            mOver = over;
            mStartTime = AnimationUtils::currentAnimationTimeMillis();
            // We were in fling/scroll mode before: current velocity is such that distance to
            // edge is increasing. This ensures that startAfterEdge will not start a new fling.
            startAfterEdge(start, end, end, (int) mCurrVelocity);
        }
    }

private:
    void onEdgeReached() {
        // mStart, mVelocity and mStartTime were adjusted to their values when edge was reached.
        // The float cast below is necessary to avoid integer overflow.
        const float velocitySquared = (float) mVelocity * mVelocity;
        float distance = velocitySquared / (2.0f * fabs(mDeceleration));
        const float sign = signum(mVelocity);

        if (distance > mOver) {
            // Default deceleration is not sufficient to slow us down before boundary
             mDeceleration = - sign * velocitySquared / (2.0f * mOver);
             distance = mOver;
        }

        mOver = (int) distance;
        mState = BALLISTIC;
        mFinal = mStart + (int) (mVelocity > 0 ? distance : -distance);
        mDuration = - (int) (1000.0f * mVelocity / mDeceleration);
    }

public:
    bool continueWhenFinished() {
        switch (mState) {
            case SPLINE:
                // Duration from start to null velocity
                if (mDuration < mSplineDuration) {
                    // If the animation was clamped, we reached the edge
                    mCurrentPosition = mStart = mFinal;
                    // TODO Better compute speed when edge was reached
                    mVelocity = (int) mCurrVelocity;
                    mDeceleration = getDeceleration(mVelocity);
                    mStartTime += mDuration;
                    onEdgeReached();
                } else {
                    // Normal stop, no need to continue
                    return false;
                }
                break;
            case BALLISTIC:
                mStartTime += mDuration;
                startSpringback(mFinal, mStart, 0);
                break;
            case CUBIC:
                return false;
        }

        update();
        return true;
    }

    /*
     * Update the current position and velocity for current time. Returns
     * true if update has been done and false if animation duration has been
     * reached.
     */
    bool update() {
        const long time = AnimationUtils::currentAnimationTimeMillis();
        const long currentTime = time - mStartTime;

        if (currentTime == 0) {
            // Skip work but report that we're still going if we have a nonzero duration.
            return mDuration > 0;
        }
        if (currentTime > mDuration) {
            return false;
        }

        double distance = 0.0;
        switch (mState) {
            case SPLINE: {
                const float t = (float) currentTime / mSplineDuration;
                const int index = (int) (NB_SAMPLES * t);
                float distanceCoef = 1.f;
                float velocityCoef = 0.f;
                if (index < NB_SAMPLES) {
                    const float t_inf = (float) index / NB_SAMPLES;
                    const float t_sup = (float) (index + 1) / NB_SAMPLES;
                    const float d_inf = SPLINE_POSITION[index];
                    const float d_sup = SPLINE_POSITION[index + 1];
                    velocityCoef = (d_sup - d_inf) / (t_sup - t_inf);
                    distanceCoef = d_inf + (t - t_inf) * velocityCoef;
                }

                distance = distanceCoef * mSplineDistance;
                mCurrVelocity = velocityCoef * mSplineDistance / mSplineDuration * 1000.0f;
                break;
            }

            case BALLISTIC: {
                const float t = currentTime / 1000.0f;
                mCurrVelocity = mVelocity + mDeceleration * t;
                distance = mVelocity * t + mDeceleration * t * t / 2.0f;
                break;
            }

            case CUBIC: {
                const float t = (float) (currentTime) / mDuration;
                const float t2 = t * t;
                const float sign = signum(mVelocity);
                distance = sign * mOver * (3.0f * t2 - 2.0f * t * t2);
                mCurrVelocity = sign * mOver * 6.0f * (- t + t2);
                break;
            }
        }

        mCurrentPosition = mStart + (int)round(distance);

        return true;
    }
};

class ViscousFluidInterpolator {
private:
    /** Controls the viscous fluid effect (how much of it). */
    static constexpr float VISCOUS_FLUID_SCALE = 8.0f;

    // must be set to 1.0 (used in viscousFluid())
    float VISCOUS_FLUID_NORMALIZE;

    // account for very small floating-point error
    float VISCOUS_FLUID_OFFSET;

    static float viscousFluid(float x) {
        x *= VISCOUS_FLUID_SCALE;
        if (x < 1.0f) {
            x -= (1.0f - (float)exp(-x));
        } else {
            float start = 0.36787944117f;   // 1/e == exp(-1)
            x = 1.0f - (float)exp(1.0f - x);
            x = start + x * (1.0f - start);
        }
        return x;
    }

public:
    ViscousFluidInterpolator()
        : VISCOUS_FLUID_NORMALIZE(1.0f / viscousFluid(1.0f))
        , VISCOUS_FLUID_OFFSET(1.0f - VISCOUS_FLUID_NORMALIZE * viscousFluid(1.0f))
    {
    
    }

    float getInterpolation(float input) {
        const float interpolated = VISCOUS_FLUID_NORMALIZE * viscousFluid(input);
        if (interpolated > 0) {
            return interpolated + VISCOUS_FLUID_OFFSET;
        }
        return interpolated;
    }
};

/**
 * This class encapsulates scrolling with the ability to overshoot the bounds
 * of a scrolling operation. This class is a drop-in replacement for
 * {@link android.widget.Scroller} in most cases.
 */
class OverScroller {
private:
    int mMode;

    SplineOverScroller mScrollerX;
    SplineOverScroller mScrollerY;

    ViscousFluidInterpolator mInterpolator;

    bool const mFlywheel = true;

    static int constexpr DEFAULT_DURATION = 250;
    static int constexpr SCROLL_MODE = 0;
    static int constexpr FLING_MODE = 1;

public:
    /**
     * The amount of friction applied to flings. The default value
     * is {@link ViewConfiguration#getScrollFriction}.
     *
     * @param friction A scalar dimension-less value representing the coefficient of
     *         friction.
     */
    void setFriction(float friction) {
        mScrollerX.setFriction(friction);
        mScrollerY.setFriction(friction);
    }

    /**
     *
     * Returns whether the scroller has finished scrolling.
     *
     * @return True if the scroller has finished scrolling, false otherwise.
     */
    bool isFinished() {
        return mScrollerX.mFinished && mScrollerY.mFinished;
    }

    /**
     * Force the finished field to a particular value. Contrary to
     * {@link #abortAnimation()}, forcing the animation to finished
     * does NOT cause the scroller to move to the final x and y
     * position.
     *
     * @param finished The new finished value.
     */
    void forceFinished(bool finished) {
        mScrollerX.mFinished = mScrollerY.mFinished = finished;
    }

    /**
     * Returns the current X offset in the scroll.
     *
     * @return The new X offset as an absolute distance from the origin.
     */
    int getCurrX() {
        return mScrollerX.mCurrentPosition;
    }

    /**
     * Returns the current Y offset in the scroll.
     *
     * @return The new Y offset as an absolute distance from the origin.
     */
    int getCurrY() {
        return mScrollerY.mCurrentPosition;
    }

    /**
     * Returns the absolute value of the current velocity.
     *
     * @return The original velocity less the deceleration, norm of the X and Y velocity vector.
     */
    float getCurrVelocity() {
        return (float)hypot(mScrollerX.mCurrVelocity, mScrollerY.mCurrVelocity);
    }

    /**
     * Returns the start X offset in the scroll.
     *
     * @return The start X offset as an absolute distance from the origin.
     */
    int getStartX() {
        return mScrollerX.mStart;
    }

    /**
     * Returns the start Y offset in the scroll.
     *
     * @return The start Y offset as an absolute distance from the origin.
     */
    int getStartY() {
        return mScrollerY.mStart;
    }

    /**
     * Returns where the scroll will end. Valid only for "fling" scrolls.
     *
     * @return The final X offset as an absolute distance from the origin.
     */
    int getFinalX() {
        return mScrollerX.mFinal;
    }

    /**
     * Returns where the scroll will end. Valid only for "fling" scrolls.
     *
     * @return The final Y offset as an absolute distance from the origin.
     */
    int getFinalY() {
        return mScrollerY.mFinal;
    }

    /**
     * Call this when you want to know the new location. If it returns true, the
     * animation is not yet finished.
     */
    bool computeScrollOffset() {
        if (isFinished()) {
            return false;
        }

        switch (mMode) {
            case SCROLL_MODE:
                break;
            case FLING_MODE:
                break;
        }

        switch (mMode) {
            case SCROLL_MODE: {
                long time = AnimationUtils::currentAnimationTimeMillis();
                // Any scroller can be used for time, since they were started
                // together in scroll mode. We use X here.
                const long elapsedTime = time - mScrollerX.mStartTime;

                const int duration = mScrollerX.mDuration;
                if (elapsedTime < duration) {
                    const float q = mInterpolator.getInterpolation(elapsedTime / (float) duration);
                    mScrollerX.updateScroll(q);
                    mScrollerY.updateScroll(q);
                } else {
                    abortAnimation();
                }
                break;
            }
            case FLING_MODE: {
                if (!mScrollerX.mFinished) {
                    if (!mScrollerX.update()) {
                        if (!mScrollerX.continueWhenFinished()) {
                            mScrollerX.finish();
                        }
                    }
                }

                if (!mScrollerY.mFinished) {
                    if (!mScrollerY.update()) {
                        if (!mScrollerY.continueWhenFinished()) {
                            mScrollerY.finish();
                        }
                    }
                }
                break;
            }
        }

        return true;
    }

    /**
     * Start scrolling by providing a starting point and the distance to travel.
     * The scroll will use the default value of 250 milliseconds for the
     * duration.
     *
     * @param startX Starting horizontal scroll offset in pixels. Positive
     *        numbers will scroll the content to the left.
     * @param startY Starting vertical scroll offset in pixels. Positive numbers
     *        will scroll the content up.
     * @param dx Horizontal distance to travel. Positive numbers will scroll the
     *        content to the left.
     * @param dy Vertical distance to travel. Positive numbers will scroll the
     *        content up.
     */
    void startScroll(int startX, int startY, int dx, int dy) {
        startScroll(startX, startY, dx, dy, DEFAULT_DURATION);
    }

    /**
     * Start scrolling by providing a starting point and the distance to travel.
     *
     * @param startX Starting horizontal scroll offset in pixels. Positive
     *        numbers will scroll the content to the left.
     * @param startY Starting vertical scroll offset in pixels. Positive numbers
     *        will scroll the content up.
     * @param dx Horizontal distance to travel. Positive numbers will scroll the
     *        content to the left.
     * @param dy Vertical distance to travel. Positive numbers will scroll the
     *        content up.
     * @param duration Duration of the scroll in milliseconds.
     */
    void startScroll(int startX, int startY, int dx, int dy, int duration) {
        mMode = SCROLL_MODE;
        mScrollerX.startScroll(startX, dx, duration);
        mScrollerY.startScroll(startY, dy, duration);
    }

    /**
     * Call this when you want to 'spring back' into a valid coordinate range.
     *
     * @param startX Starting X coordinate
     * @param startY Starting Y coordinate
     * @param minX Minimum valid X value
     * @param maxX Maximum valid X value
     * @param minY Minimum valid Y value
     * @param maxY Minimum valid Y value
     * @return true if a springback was initiated, false if startX and startY were
     *          already within the valid range.
     */
    bool springBack(int startX, int startY, int minX, int maxX, int minY, int maxY) {
        mMode = FLING_MODE;

        // Make sure both methods are called.
        const bool spingbackX = mScrollerX.springback(startX, minX, maxX);
        const bool spingbackY = mScrollerY.springback(startY, minY, maxY);
        return spingbackX || spingbackY;
    }

    void fling(int startX, int startY, int velocityX, int velocityY,
            int minX, int maxX, int minY, int maxY) {
        fling(startX, startY, velocityX, velocityY, minX, maxX, minY, maxY, 0, 0);
    }

    /**
     * Start scrolling based on a fling gesture. The distance traveled will
     * depend on the initial velocity of the fling.
     *
     * @param startX Starting point of the scroll (X)
     * @param startY Starting point of the scroll (Y)
     * @param velocityX Initial velocity of the fling (X) measured in pixels per
     *            second.
     * @param velocityY Initial velocity of the fling (Y) measured in pixels per
     *            second
     * @param minX Minimum X value. The scroller will not scroll past this point
     *            unless overX > 0. If overfling is allowed, it will use minX as
     *            a springback boundary.
     * @param maxX Maximum X value. The scroller will not scroll past this point
     *            unless overX > 0. If overfling is allowed, it will use maxX as
     *            a springback boundary.
     * @param minY Minimum Y value. The scroller will not scroll past this point
     *            unless overY > 0. If overfling is allowed, it will use minY as
     *            a springback boundary.
     * @param maxY Maximum Y value. The scroller will not scroll past this point
     *            unless overY > 0. If overfling is allowed, it will use maxY as
     *            a springback boundary.
     * @param overX Overfling range. If > 0, horizontal overfling in either
     *            direction will be possible.
     * @param overY Overfling range. If > 0, vertical overfling in either
     *            direction will be possible.
     */
    void fling(int startX, int startY, int velocityX, int velocityY,
            int minX, int maxX, int minY, int maxY, int overX, int overY) {
        // Continue a scroll or fling in progress
        if (mFlywheel && !isFinished()) {
            float oldVelocityX = mScrollerX.mCurrVelocity;
            float oldVelocityY = mScrollerY.mCurrVelocity;
            if (signum(velocityX) == signum(oldVelocityX) &&
                    signum(velocityY) == signum(oldVelocityY)) {
                velocityX += oldVelocityX;
                velocityY += oldVelocityY;
            }
        }

        mMode = FLING_MODE;
        mScrollerX.fling(startX, velocityX, minX, maxX, overX);
        mScrollerY.fling(startY, velocityY, minY, maxY, overY);
    }

    /**
     * Notify the scroller that we've reached a horizontal boundary.
     * Normally the information to handle this will already be known
     * when the animation is started, such as in a call to one of the
     * fling functions. However there are cases where this cannot be known
     * in advance. This function will transition the current motion and
     * animate from startX to finalX as appropriate.
     *
     * @param startX Starting/current X position
     * @param finalX Desired final X position
     * @param overX Magnitude of overscroll allowed. This should be the maximum
     *              desired distance from finalX. Absolute value - must be positive.
     */
    void notifyHorizontalEdgeReached(int startX, int finalX, int overX) {
        mScrollerX.notifyEdgeReached(startX, finalX, overX);
    }

    /**
     * Notify the scroller that we've reached a vertical boundary.
     * Normally the information to handle this will already be known
     * when the animation is started, such as in a call to one of the
     * fling functions. However there are cases where this cannot be known
     * in advance. This function will animate a parabolic motion from
     * startY to finalY.
     *
     * @param startY Starting/current Y position
     * @param finalY Desired final Y position
     * @param overY Magnitude of overscroll allowed. This should be the maximum
     *              desired distance from finalY. Absolute value - must be positive.
     */
    void notifyVerticalEdgeReached(int startY, int finalY, int overY) {
        mScrollerY.notifyEdgeReached(startY, finalY, overY);
    }

    /**
     * Returns whether the current Scroller is currently returning to a valid position.
     * Valid bounds were provided by the
     * {@link #fling(int, int, int, int, int, int, int, int, int, int)} method.
     *
     * One should check this value before calling
     * {@link #startScroll(int, int, int, int)} as the interpolation currently in progress
     * to restore a valid position will then be stopped. The caller has to take into account
     * the fact that the started scroll will start from an overscrolled position.
     *
     * @return true when the current position is overscrolled and in the process of
     *         interpolating back to a valid value.
     */
    bool isOverScrolled() {
        return ((!mScrollerX.mFinished &&
                mScrollerX.mState != SplineOverScroller::SPLINE) ||
                (!mScrollerY.mFinished &&
                        mScrollerY.mState != SplineOverScroller::SPLINE));
    }

    /**
     * Stops the animation. Contrary to {@link #forceFinished(boolean)},
     * aborting the animating causes the scroller to move to the final x and y
     * positions.
     *
     * @see #forceFinished(boolean)
     */
    void abortAnimation() {
        mScrollerX.finish();
        mScrollerY.finish();
    }

    /**
     * Returns the time elapsed since the beginning of the scrolling.
     *
     * @return The elapsed time in milliseconds.
     *
     * @hide
     */
    int timePassed() {
        const long time = AnimationUtils::currentAnimationTimeMillis();
        const long startTime = std::min(mScrollerX.mStartTime, mScrollerY.mStartTime);
        return (int) (time - startTime);
    }

    /**
     * @hide
     */
    bool isScrollingInDirection(float xvel, float yvel) {
        const int dx = mScrollerX.mFinal - mScrollerX.mStart;
        const int dy = mScrollerY.mFinal - mScrollerY.mStart;
        return !isFinished() && signum(xvel) == signum(dx) &&
                signum(yvel) == signum(dy);
    }
};
