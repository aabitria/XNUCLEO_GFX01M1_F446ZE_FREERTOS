#include <gui/screen1_screen/Screen1View.hpp>
#include <touchgfx/widgets/canvas/CWRUtil.hpp>
#include <cstring>
#include <cstdio>

#define TEMP_CIRC_MIN_ARC_DEG       (-120)
#define TEMP_CIRC_MAX_ARC_DEG       (120)
#define TEMP_CIRC_MAX_ARC_LEN       (TEMP_CIRC_MAX_ARC_DEG - TEMP_CIRC_MIN_ARC_DEG)
#define TEMP_MIN_LIMIT              (0)
#define TEMP_MAX_LIMIT              (60)

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::display_temp(float temp)
{
	char str[5] = {0};
	int16_t end_arc, start_arc = -120;

	snprintf(str, sizeof(str), "%.1f", temp);
	Unicode::snprintf(textArea2Buffer, TEXTAREA2_SIZE, str);
	textArea2.invalidate();

	end_arc = start_arc + ((int16_t)temp * TEMP_CIRC_MAX_ARC_LEN / TEMP_MAX_LIMIT);

	circle1.updateArcEnd((int)end_arc);
	circle1.invalidate();

}
