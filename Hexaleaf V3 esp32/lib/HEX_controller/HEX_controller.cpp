#include "HEX_controller.h"
// init function
void Hex_controller::count_preset_lenghts()
{
}
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
    // for all boxes
    for (int i = 0; i < NUM_BOXES; i++)
    {
        int x = position[i][0];
        int y = position[i][1];
        // check all other boxes
        for (int q = 0; q < NUM_BOXES; q++)
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
    outer_led_num = TOTAL_LEDS - (num_of_contact_points * LED_IN_SIDE);
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

    for (int i = 0; i < NUM_BOXES; i++)
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
    // mySerial.printf(" min horz-%d verz-%d max horz-%d verz-%d\n", HorzMin, VertMin, HorzMax, VertMax);
}
// basic fill functions
void Hex_controller::fill_leds_on_Vert_lvl(int16_t lvl, CRGB clr)
{
    // mySerial.printf("fill vert lvl %d\n", lvl);
    for (uint8_t i = 0; i < NUM_BOXES; i++)
    {
        int line_lvl = lvl - (10 * position[i][0]);
        if (line_lvl >= 0 && line_lvl <= 15)
            nodes[i]->fill_vert_line(clr, leds, 15 - line_lvl);
    }
}
void Hex_controller::fill_leds_on_Hor_lvl(int16_t lvl, CRGB clr)
{
    for (uint8_t i = 0; i < NUM_BOXES; i++)
    {
        int line_lvl = lvl - (6 * position[i][1]);
        if (line_lvl >= 0 && line_lvl < 12)
            nodes[i]->fill_hor_line(clr, leds, line_lvl);
    }
}
void Hex_controller::fill_same_dir_sides(CRGB clr, int direc)
{
    direc = direc % 6;
    for (int i = 0; i < NUM_BOXES; i++)
    { // side is <0-5>
        nodes[i]->fill_side(clr, leds, direc);
    }
}
void Hex_controller::fill_all_hex(CRGB clr)
{
    // mySerial.print("filling all hex with clr: ");
    // printCRGB(clr);
    for (int i = 0; i < TOTAL_LEDS; i++)
    {
        leds[i] = clr;
    }
}
void Hex_controller::fill_one_led_all_hex(CRGB clr, uint8_t n)
{
    for (int i = 0; i < NUM_BOXES; i++)
    {
        nodes[i]->fill_one_led(clr, leds, n);
    }
}
void Hex_controller::fill_one_side_one_hex(CRGB clr, uint8_t hex, uint8_t dir)
{
    dir = dir % 6;
    hex = hex % NUM_BOXES;
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
    drawEveryNthMs = spd;
}
void Hex_controller::set_color(CRGB clr, int idx)
{
    clr_arr[idx] = clr;
    printCRGB(clr_arr[idx]);
}
void Hex_controller::set_brightness(uint8_t b)
{
    maxBrightness = b;
}
void Hex_controller::set_serial(HardwareSerial &ser)
{
    mySerial = ser;
}
void Hex_controller::set_fade(bool f)
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
    // mySerial.print("rainbow is now: ");
    // mySerial.println(rainbow);
}
// internal functions
void Hex_controller::change_mode(Mode m)
{
    fill_mode = Not_fill_mode;
    fill_done = true;
    mode = m;
    preset = 0;
    step_count = 0;
    step = 1;
    if (mode == Mode::AudioBeatReact || mode == Mode::AudioFreqPool)
    {
        drawEveryNthMs = 1;
        fade = 70;
    }
    else
    {
        drawEveryNthMs = 100;
    }
}

void Hex_controller::change_fill_mode(FillMode new_fill_mode)
{
    // mySerial.print("changing fill mode to: ");
    // mySerial.println(new_fill_mode);
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
        step_count = TOTAL_LEDS;
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
    mode = (Mode)(((int)mode) + 1);
    if (mode == Mode_num)
        mode = (Mode)0;
}

void Hex_controller::printCRGB(CRGB clr)
{
    mySerial.printf("R: %x G: %x B: %x\n", clr.r, clr.g, clr.b);
}

void Hex_controller::update()
{
    if (millis() > (lastDrew + drawEveryNthMs))
    {

        // mySerial.println("updating now: ");
        if (rainbow > 0)
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
        case Rotation:
        {
            // mySerial.println("rotate");
            for (int i = 0; i < NUM_BOXES; i++)
            {
                if (!fade)
                {
                    fill_one_led_all_hex(clr_arr[0], (step_count + 1) % LEDS_IN_BOX);
                    fill_one_led_all_hex(CRGB::Black, step_count % LEDS_IN_BOX);
                }
                else
                {
                    fill_one_led_all_hex(clr_arr[0], step_count % LEDS_IN_BOX);
                }
            }
            step_count++;
            break;
        }
        case RotationOuter:
        { ////this needs testing -----------------------------
            // mySerial.println("rotateOuter"); //using preset as util var
            if (step_count >= outer_led_num)
                step_count = 0;
            leds[outer_led_adr[step_count]] = clr_arr[0];
            step_count++;
            break;
        }
        case RandColorRandHex:
        {
            uint8_t idx = rand() % NUM_BOXES;
            nodes[idx]->fill_hex(CHSV(random8(), random8(), 255), leds);
            break;
        }
        case PresetAnim:
        {
            // Serial.printf("preset anim %d, anim step%d\n", preset, animation_step);
            if (preset != 0 && fill_done)
            {
                animation_step++;
                if (preset_lenght[preset - 1] <= animation_step)
                {
                    animation_step = 0;
                }
                change_fill_mode((FillMode)presets[preset - 1][animation_step]);
                // switch (preset)
                // {
                // case 1:
                //     change_fill_mode(preset[animation_step]);
                //     break;
                // case 2:
                //     change_fill_mode(preset2[animation_step]);
                //     break;
                // case 3:
                //     change_fill_mode(preset3[animation_step]);
                //     break;
                // case 4:
                //     change_fill_mode(preset4[animation_step]);
                //     break;
                // case 5:
                //     change_fill_mode(preset5[animation_step]);
                //     break;
                // default:
                //     // notihing
                //     break;
                // }
            }
            break;
        }
        case RainbowSwipeVert:
        {
            CRGB tmp[VertCount];
            fill_rainbow(tmp, VertCount, step_count);
            for (int i = 0; i <= VertCount; i++)
            {

                // CRGB clr=clr.setHue(((step_count+i)*step)%255);
                fill_leds_on_Vert_lvl(i, tmp[i]);
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
                // CRGB clr=clr.setHue(((step_count+i)*step)%255);
                fill_leds_on_Hor_lvl(i, tmp[i]);
            }
            step_count = (step_count > 255) ? 0 : step_count + 1;
            break;
        }
        case AudioBeatReact:
        {
            // long start = millis();
            float magnitudeBand[FREQ_BAND_COUNT] = {0.0f};
            float magnitudeBandWeightedMax = 0.0f;

            newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);
            for (int i = 0; i < NUM_BOXES; i++)
            {
                int idx = i + kBeatDetectBandOffset;

                beatHist_[idx][hist_ptr] = magnitudeBand[idx] * kFreqBandAmp[idx] * sensitivityFactor_;
                float histAVG = 0;
                for (int h = 0; h < HIST_NUM_BEAT; h++)
                {
                    histAVG += beatHist_[idx][h];
                    // if(i==0){
                    // Serial.printf("h%d: %f ", i, beatHist_[idx][h]);
                    // }
                }
                histAVG = histAVG / (float)HIST_NUM_BEAT;

                // Serial.printf("now:%f avg:%f\n", beatHist_[idx][hist_ptr], histAVG);

                if (beatHist_[idx][hist_ptr] > histAVG * BeatThresholdMultyplier)
                {
                    // Serial.printf("beat detected");
                    beatVisIntensity_[i] = 250;
                }
                //
                else
                {
                    if (beatVisIntensity_[i] >= 150)
                        beatVisIntensity_[i] -= 20;
                    if (beatVisIntensity_[i] >= 50)
                        beatVisIntensity_[i] -= 5;
                    if (beatVisIntensity_[i] >= 20)
                        beatVisIntensity_[i] -= 1;
                }
                if (rainbow != 0)
                {
                    nodes[i]->fill_hex(CHSV((255 / NUM_BOXES) * i, 255, beatVisIntensity_[i]), leds);
                }
                else
                {
                }
            }

            hist_ptr--;
            if (hist_ptr < 0)
            {
                hist_ptr = HIST_NUM_BEAT;
            }
            // long time = millis() - start;
            // Serial.printf("time:%lu\n", time);
            break;
        }
        case AudioFreqPool:
        {
            // mySerial.println("Audio freq pool----");
            float magnitudeBand[FREQ_BAND_COUNT] = {0.0f};
            float magnitudeBandWeightedMax = 0.0f;
            // mySerial.println("Start new audio reading");

            newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);
            // mySerial.println("Audio reading DONE");

            int poolsForOneHex = FREQ_BAND_COUNT / NUM_BOXES;
            for (int i = 0; i < NUM_BOXES; i++)
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
                // nodes[i]->fill_hex(CHSV(20 * i, 255, lightness), leds);
                uint8_t ledNum = lightness / (255 / 12);
                nodes[i]
                    ->fill_n_hor_lines(CRGB::Green, CRGB::Red, leds, ledNum);
            }
            break;
        }
        default:
            // here test code
            mySerial.println("ERROR: False mode");

            break;
        }
        if (!fill_done && fill_mode != Not_fill_mode)
        {
            // Serial.printf("preset anim %d, anim step%d\n", preset, animation_step);
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
                    fill_leds_on_Vert_lvl(step_count, clr_arr[animation_step]);
                    fill_leds_on_Vert_lvl(VertCount - step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_meet_in_midle_TB:
                if (step_count >= HorzMin && (HorzCount / 2) >= step_count)
                {
                    fill_leds_on_Hor_lvl(step_count, clr_arr[animation_step]);
                    fill_leds_on_Hor_lvl(HorzCount - step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_rotation_fTop:
            case Fill_by_rotation_fBottom:
                if (step_count <= TOTAL_LEDS)
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

        if (fade > 0)
        {
            for (int i = 0; i < TOTAL_LEDS; i++)
            {
                leds[i].fadeToBlackBy(fade);
            }
        }
        FastLED.show(maxBrightness);
    }
}