#include "app.h"

static const int CALIBRATION_TARGET_X[] = {
    TOUCH_CALIBRATION_X_LEFT,
    TOUCH_CALIBRATION_X_RIGHT,
    TOUCH_CALIBRATION_X_RIGHT,
    TOUCH_CALIBRATION_X_LEFT
};
static const int CALIBRATION_TARGET_Y[] = {
    TOUCH_CALIBRATION_Y_TOP,
    TOUCH_CALIBRATION_Y_TOP,
    TOUCH_CALIBRATION_Y_BOTTOM,
    TOUCH_CALIBRATION_Y_BOTTOM
};
static const uint8_t CALIBRATION_TARGET_COUNT = 4;

static TS_Point calibrationPoints[CALIBRATION_TARGET_COUNT];
static uint8_t calibrationStep = 0;
static bool calibrationFinished = false;
static bool calibrationSaved = false;

static void resetCalibration()
{
    calibrationStep = 0;
    calibrationFinished = false;
    calibrationSaved = false;
}

static void drawCalibrationTarget(int x, int y)
{
    gfx->drawCircle(x, y, 11, COLOR_WARNING);
    gfx->drawCircle(x, y, 3, COLOR_WARNING);
    gfx->drawFastHLine(x - 15, y, 31, COLOR_WARNING);
    gfx->drawFastVLine(x, y - 15, 31, COLOR_WARNING);
}

void drawTouchCalibrationScreen()
{
    clearScreen(COLOR_BACKGROUND);
    titleBar("TOUCH CALIBRATION");

    setDefaultFont();
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT);

    if (calibrationFinished)
    {
        centerText(
            calibrationSaved ? "Calibration saved" : "Calibration failed",
            94,
            calibrationSaved ? COLOR_OK : COLOR_ERROR,
            1);
        centerText(
            calibrationSaved ? "Tap Home to finish" : "Tap to retry",
            120,
            COLOR_TEXT,
            1);
        drawNav1("Again");
        if (calibrationSaved)
            drawNav4("Home");
        return;
    }

    char instruction[24];
    snprintf(
        instruction,
        sizeof(instruction),
        "Tap target %u of %u",
        static_cast<unsigned>(calibrationStep + 1),
        static_cast<unsigned>(CALIBRATION_TARGET_COUNT));
    centerText(instruction, 47, COLOR_TEXT, 1);

    drawCalibrationTarget(
        CALIBRATION_TARGET_X[calibrationStep],
        CALIBRATION_TARGET_Y[calibrationStep]);

}

void enterTouchCalibrationScreen()
{
    resetCalibration();
    state.currentScreen = SCREEN_TOUCH_CALIBRATION;
    drawCurrentScreen();
}

static TouchCalibration makeCalibration()
{
    TouchCalibration calibration;
    calibration.xLeft = static_cast<int16_t>(
        (static_cast<int32_t>(calibrationPoints[0].x) + calibrationPoints[3].x) / 2);
    calibration.xRight = static_cast<int16_t>(
        (static_cast<int32_t>(calibrationPoints[1].x) + calibrationPoints[2].x) / 2);
    calibration.yTop = static_cast<int16_t>(
        (static_cast<int32_t>(calibrationPoints[0].y) + calibrationPoints[1].y) / 2);
    calibration.yBottom = static_cast<int16_t>(
        (static_cast<int32_t>(calibrationPoints[2].y) + calibrationPoints[3].y) / 2);
    return calibration;
}

void handleTouchCalibrationTouch(int x, int y)
{
    if (calibrationFinished)
    {
        if (!calibrationSaved || navPressed1(x, y))
        {
            resetCalibration();
            drawCurrentScreen();
        }
        else if (navPressed4(x, y))
        {
            enterHomeScreen();
        }
        return;
    }

    const TS_Point point = getRawTouch();
    if (point.z < TOUCH_MIN_PRESSURE)
        return;

    calibrationPoints[calibrationStep] = point;
    if (++calibrationStep < CALIBRATION_TARGET_COUNT)
    {
        drawCurrentScreen();
        return;
    }

    calibrationFinished = true;
    calibrationSaved = saveTouchCalibration(makeCalibration());
    drawCurrentScreen();
}
