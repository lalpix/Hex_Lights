#include "HEX_controller.h"
// init function
void Hex_controller::calculate_outer_leds()
{
    int num_of_contact_points = 0;
    int direc_arr[6][2] = {
        {1, -1},
        {1, 1},
        {0, 2},
        {-1, 1},
        {-1, -1},
        {0, -2},
    };
    Serial.println("calculating outer leds");
    // for all boxes
    for (int i = 0; i < Hex_controller::NumBoxes; i++)
    {
        int x = position[i][0];
        int y = position[i][1];
        // check all other boxes
        for (int q = 0; q < Hex_controller::NumBoxes; q++)
        {
            // for all directions
            for (int k = 0; k < 6; k++)
            {
                int n = (x + direc_arr[k][0]);
                int m = (y + direc_arr[k][1]);

                // if they are touching
                if (position[q][0] == n && position[q][1] == m && q != i)
                {
                    // Serial.printf("box %d with box %d has contact on %d\n",i,q,k);
                    nodes[i]->connectionPoints[k] = q + 1; // q+1 so 0 can be no contact
                    num_of_contact_points++;
                }
            }
        }
    }
    outer_led_num = Hex_controller::TotalLeds - (num_of_contact_points * LED_IN_SIDE);
}
void Hex_controller::create_outer_path()
{
    uint16_t box_i = 0;
    uint8_t side_i = 0;
    uint32_t path_i = 0;
    int16_t start_side = -1;
    while (1)
    {
        // if box has this side free, add to path.
        if (nodes[box_i]->connectionPoints[side_i] == 0)
        {
            if (box_i == 0 && side_i == start_side)
            {
                break;
            }
            // Serial.printf("adding to path side %d of box %d\n", side_i, box_i);
            for (int i = 0; i < LED_IN_SIDE; i++)
            {
                outer_led_adr[path_i] = nodes[box_i]->side_start_led(side_i) + i;
                path_i++;
            }

            if (start_side == -1)
            {
                start_side = side_i;
            }
            side_i = (side_i + 1) % 6;
        }
        else
        {
            box_i = nodes[box_i]->connectionPoints[side_i] - 1;
            side_i = (side_i + 3 + 1) % 6;
        }
    }
}
void Hex_controller::calculate_horz_vert_vals()
{
    HorzMax = INT8_MIN;
    HorzMin = INT8_MAX;
    VertMax = INT8_MIN;
    VertMin = INT8_MAX;

    for (int i = 0; i < Hex_controller::NumBoxes; i++)
    {
        if (position[i][1] > HorzMax)
        {
            HorzMax = position[i][1];
        }
        if (position[i][1] < HorzMin)
        {
            HorzMin = position[i][1];
        }
        if (position[i][0] > VertMax)
        {
            VertMax = position[i][0];
        }
        if (position[i][0] < VertMin)
        {
            VertMin = position[i][0];
        }
    }
    HorzMin = HorzMin * 6;
    HorzMax = 2 + 2 * LED_IN_SIDE + HorzMax * 6;
    VertMin = VertMin * (2 * LED_IN_SIDE);
    VertMax = 3 * LED_IN_SIDE + VertMax * (LED_IN_SIDE * 2) + 1;
    HorzCount = HorzMax - HorzMin;
    VertCount = VertMax - VertMin;
    // mySerial.printf(" min horz-%d verz-%d max horz-%d verz-%d CNT h: %d v:%d\n", HorzMin, VertMin, HorzMax, VertMax, HorzCount, VertCount);
}
// basic fill functions
void Hex_controller::fill_leds_on_Vert_lvl(int16_t lvl, CRGB clr)
{
    // mySerial.printf("fill vert lvl %d\n", lvl);
    for (uint8_t i = 0; i < Hex_controller::NumBoxes; i++)
    {
        int line_lvl = lvl - (10 * position[i][0]);
        if (line_lvl >= 0 && line_lvl <= 15)
            nodes[i]->fill_vert_line(clr, leds, 15 - line_lvl);
    }
}
void Hex_controller::fill_leds_on_Hor_lvl(int16_t lvl, CRGB clr)
{
    for (uint8_t i = 0; i < Hex_controller::NumBoxes; i++)
    {
        int line_lvl = lvl - (6 * position[i][1]);
        if (line_lvl >= 0 && line_lvl < 12)
            nodes[i]->fill_hor_line(clr, leds, line_lvl);
    }
}
void Hex_controller::fill_same_dir_sides(CRGB clr, int direc)
{
    direc = direc % 6;
    for (int i = 0; i < Hex_controller::NumBoxes; i++)
    { // side is <0-5>
        nodes[i]->fill_side(clr, leds, direc);
    }
}
void Hex_controller::fill_one_hex(uint8_t hex, CRGB clr)
{
    nodes[hex]->fill_hex(clr, leds);
    mode = NotUpdating;
}
void Hex_controller::fill_all_hex(CRGB clr)
{
    for (int i = 0; i < Hex_controller::TotalLeds; i++)
    {
        leds[i] = clr;
    }
}
void Hex_controller::fill_one_led_all_hex(CRGB clr, uint8_t n)
{
    for (int i = 0; i < Hex_controller::NumBoxes; i++)
    {
        nodes[i]->fill_one_led(clr, leds, n);
    }
}
void Hex_controller::fill_one_side_one_hex(CRGB clr, uint8_t hex, uint8_t dir)
{
    dir = dir % 6;
    hex = hex % Hex_controller::NumBoxes;
    nodes[hex]->fill_side(clr, leds, dir);
}
// user input
void Hex_controller::set_pre_anim(uint8_t n)
{
    mode = PresetAnim;
    fill_done = true;
    preset = n;
    step = 0;
    animation_step = 0;
}
void Hex_controller::set_speed(uint16_t spd)
{
    drawEveryNthMs = 100 - spd;
}
void Hex_controller::set_color(CRGB clr, int idx)
{

    clr_arr[idx] = clr;
    printCRGB(clr_arr[idx]);
    if (mode == Mode::TwoColorFading)
    {
        change_mode(Mode::TwoColorFading);
    }
}
void Hex_controller::set_brightness(uint8_t b)
{
    maxBrightness = b;
}
void Hex_controller::set_serial(HardwareSerial &ser)
{
    mySerial = ser;
}
void Hex_controller::set_fade(uint8_t f)
{
    fade = f;
}
void Hex_controller::set_rainbow(int r)
{
    if (r == 1)
    {
        rainbow = rgb2hsv_approximate(clr_arr[0]).h;
    }
    else
    {
        rainbow = 0;
    }
}

// internal functions

void Hex_controller::change_mode(Mode m)
{
    fill_mode = Not_fill_mode;
    fill_done = true;
    mode = m;
    fade = 30;
    preset = 0;
    step_count = 0;
    step = 1;
    if (mode == Mode::AudioBeatReact || mode == Mode::AudioFreqPool)
    {
        drawEveryNthMs = 1;
        fade = 70;
    }
    else if (mode == Mode::RainbowSwipeHorz || mode == Mode::RainbowSwipeVert || mode == Mode::RandColorRandHexFade)
    {
        drawEveryNthMs = 30;
    }
    else
    {
        drawEveryNthMs = 100;
    }
}
void Hex_controller::change_fill_mode(FillMode new_fill_mode)
{

    fill_mode = new_fill_mode;
    fill_done = false;
    lastDrew = 0;
    step_count = 0;
    step = 1;

    switch (fill_mode)
    {
    case Fill_by_lines_fTop: // horizontal
        step_count = HorzMax;
        step = -1;
        break;
    case Fill_by_lines_fRight: // vertical
        step_count = VertMax;
        step = -1;
        break;
    case Fill_by_rotation_fTop:
        step_count = Hex_controller::TotalLeds;
        step = -1;
        break;
    case Fill_by_lines_fLeft:
        step_count = VertMin;
        break;
    case Fill_by_lines_fBottom:
        step_count = HorzMin;
        break;
    case Fill_by_rotation_fBottom:
    case Fill_by_lines_meet_in_midle_LR:
    case Fill_by_lines_meet_in_midle_TB:
        break;
    default: // Stationar
        Serial.printf("ERROR:Wrong fill mode\n");
        break;
    }
}
void Hex_controller::next_mode()
{

    if (mode != PresetAnim)
    {
        mode = (Mode)(((int)mode) + 1);
        if (mode == Mode_num)
            mode = (Mode)0;
        change_mode(mode);
    }
    if (mode == PresetAnim)
    {
        if (preset == preset_num)
        {
            preset = 0;
            mode = (Mode)(((int)mode) + 1);
            if (mode == Mode_num)
                mode = (Mode)0;
            change_mode(mode);
        }
        else
        {
            preset += 1;
            set_pre_anim(preset);
        }
    }
}
void Hex_controller::update()
{
    if (millis() > (lastDrew + drawEveryNthMs))
    {
        if (rainbow > 0) // hue changing
        {
            clr_arr[0].setHue(rainbow);
            rainbow++;
            if (rainbow >= 255)
                rainbow = 1;
        }
        switch (mode)
        {
        case Stationar:
        {
            fill_all_hex(clr_arr[0]);
            break;
        }
        case StationarOuter:
        {
            for (int i = 0; i < outer_led_num; i++)
            {
                leds[outer_led_adr[i]] = clr_arr[0];
            }
            break;
        }
        case RotationOuter:
        {
            if (step_count >= outer_led_num)
                step_count = 0;
            leds[outer_led_adr[step_count]] = clr_arr[0];
            step_count++;
            break;
        }
        case RandColorRandHex:
        {
            step_count++;
            if (step_count > 5)
            { // update every 10 ticks
                step_count = 0;
                uint8_t idx = rand() % Hex_controller::NumBoxes;
                nodes[idx]->fill_hex(CHSV(random8(), random8(), 255), leds);
            }
            break;
        }
        case RandColorRandHexFade:
        {
            rainbow = 0;
            // for all hex
            for (int h = 0; h < NumBoxes; h++)
            {
                if (nodes[h]->color == nodes[h]->colorTo)
                {

                    nodes[h]->colorTo = CRGB(random8(), random8(), random8());
                }
                leds = nodes[h]->fadeToColor(leds, 1);
            }

            break;
        }
        case PresetAnim:
        {
            if (preset != 0 && fill_done)
            {
                animation_step++;
                if (preset_lenght[preset - 1] <= animation_step)
                {
                    animation_step = 0;
                }
                change_fill_mode((FillMode)presets[preset - 1][animation_step]);
            }
            break;
        }
        case RainbowSwipeVert:
        {
            CRGB tmp[VertCount];
            fill_rainbow(tmp, VertCount, step_count);
            for (int i = 0; i <= VertCount; i++)
            {
                fill_leds_on_Vert_lvl(i + VertMin, tmp[i]);
            }
            step_count = (step_count > 255) ? 0 : step_count + 1;
            break;
        }
        case RainbowSwipeHorz:
        {
            CRGB tmp[HorzCount];
            fill_rainbow(tmp, HorzCount, step_count);
            for (int i = 0; i < HorzCount; i++)
            {
                fill_leds_on_Hor_lvl(i + HorzMin, tmp[i]);
            }
            step_count = (step_count > 255) ? 0 : step_count + 1;
            break;
        }
        case AudioBeatReact:
        {
            float magnitudeBand[FREQ_BAND_COUNT] = {0.0f};
            float magnitudeBandWeightedMax = 0.0f;

            newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);
            for (int i = 0; i < Hex_controller::NumBoxes; i++)
            {
                int idx = i + kBeatDetectBandOffset;

                beatHist_[idx][hist_ptr] = magnitudeBand[idx] * kFreqBandAmp[idx] * sensitivityFactor_;
                float histAVG = 0;
                for (int h = 0; h < HIST_NUM_BEAT; h++)
                {
                    histAVG += beatHist_[idx][h];
                }
                histAVG = histAVG / (float)HIST_NUM_BEAT;

                if (beatHist_[idx][hist_ptr] > histAVG * BeatThresholdMultyplier)
                {
                    beatVisIntensity_[i] = 250;
                }
                else
                {
                    if (beatVisIntensity_[i] >= 150)
                        beatVisIntensity_[i] -= 20;
                    else if (beatVisIntensity_[i] >= 50)
                        beatVisIntensity_[i] -= 5;
                    else if (beatVisIntensity_[i] >= 20)
                        beatVisIntensity_[i] -= 1;
                }
                if (rainbow != 0)
                {
                    nodes[i]->fill_hex(CHSV((255 / Hex_controller::NumBoxes) * i, 255, beatVisIntensity_[i]), leds);
                }
            }

            hist_ptr--;
            if (hist_ptr < 0)
            {
                hist_ptr = HIST_NUM_BEAT;
            }
            break;
        }
        case AudioFreqPool:
        {
            float magnitudeBand[FREQ_BAND_COUNT] = {0.0f};
            float magnitudeBandWeightedMax = 0.0f;

            newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);

            int poolsForOneHex = FREQ_BAND_COUNT / Hex_controller::NumBoxes;
            for (int i = 0; i < Hex_controller::NumBoxes; i++)
            {
                float bandMagAvg = 0;
                float bandAmpAvg = 0;
                for (int j = 0; j < poolsForOneHex; j++)
                {
                    int idx = (i * poolsForOneHex) + j;
                    bandMagAvg += magnitudeBand[idx] * kFreqBandAmp[idx];
                }
                bandMagHistory[i][hist_ptr] = bandMagAvg;
                hist_ptr++;
                if (hist_ptr >= HIST_NUM_FREQ)
                {
                    hist_ptr = 0;
                }

                float histAvgMag = 0;
                for (int h = 0; h < HIST_NUM_FREQ; h++)
                {
                    histAvgMag += bandMagHistory[i][h];
                }
                histAvgMag = histAvgMag / ((float)HIST_NUM_FREQ);
                int histMult = 100 * (bandMagAvg / histAvgMag);
                uint8_t lightness = min((int)(bandMagAvg * histMult), 255);
                uint8_t ledNum = lightness / (255 / 12);
                nodes[i]
                    ->fill_n_hor_lines(CRGB::Green, CRGB::Red, leds, ledNum);
            }
            break;
        }
        case TwoColorFading:
            rainbow = 0;
            for (int h = 0; h < NumBoxes; h++)
            {
                if (nodes[h]->color == nodes[h]->colorTo)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        int dif = abs(clr_arr[1][i]) - abs(clr_arr[0][i]);
                        int dir = rand() % abs(dif);
                        if (dif < 0)
                        {
                            dir = dir * (-1);
                        }
                        nodes[h]->colorTo[i] = clr_arr[0][i] + dir;
                    }
                }
                leds = nodes[h]->fadeToColor(leds, 1);
            }
            break;
        case NotUpdating:
            break;
        default:
            // here test code
            mySerial.println("ERROR: False mode");
            break;
        }
        if (!fill_done && fill_mode != Not_fill_mode)
        {
            CRGB tmp = clr_arr[animation_step];
            if (rainbow)
            {
                tmp = clr_arr[0];
            }
            switch (fill_mode)
            {
            case Fill_by_lines_fTop:
            case Fill_by_lines_fBottom:
                if (step_count >= HorzMin && step_count <= HorzMax)
                {
                    fill_leds_on_Hor_lvl(step_count, tmp);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_fLeft:
            case Fill_by_lines_fRight:
                if (step_count >= VertMin && step_count <= VertMax)
                {
                    fill_leds_on_Vert_lvl(step_count, tmp);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_meet_in_midle_LR:
                if (step_count >= VertMin && (VertCount / 2) >= step_count)
                {
                    fill_leds_on_Vert_lvl(step_count + VertMin, clr_arr[animation_step]);
                    fill_leds_on_Vert_lvl(VertMax - step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_meet_in_midle_TB:
                if (step_count >= HorzMin && (HorzCount / 2) >= step_count)
                {
                    fill_leds_on_Hor_lvl(step_count + HorzMin, clr_arr[animation_step]);
                    fill_leds_on_Hor_lvl(HorzMax - step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_rotation_fTop:
            case Fill_by_rotation_fBottom:
                if (step_count <= Hex_controller::TotalLeds)
                {
                    leds[step_count] = clr_arr[animation_step];
                }
                else
                {
                    fill_done = true;
                }
                break;
            }
            step_count += step;
        }
        lastDrew = millis();
        if (mode != NotUpdating)
        {
            fadeToBlackBy(leds, Hex_controller::TotalLeds, fade);
        }
        FastLED.show(maxBrightness);
    }
}