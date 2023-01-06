#include "HEX_controller.h"

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
    uint16_t led_count_1 = 0;
    uint16_t led_count_2 = 0;
    uint8_t start_1 = 0;
    uint8_t end_1 = 0;
    uint8_t start_2 = 0;
    uint8_t end_2 = 0;
    for (int i = 0; i < NUM_BOXES; i++)
    {

        Hexnode *tmp = nodes[i];
        start_1 = 0;
        end_1 = 0;
        start_2 = 0;
        end_2 = 0;
        // check all sides for contact
        for (int j = 0; j < 6; j++)
        {

            if (tmp->connectionPoints[j] != 0)
            { // if contact

                if (tmp->connectionPoints[j] > i)
                { // its contanct with next box
                    end_1 = j;
                    start_2 = j + 1;
                }
                else
                { // its contantc with previous box
                    end_2 = j;
                    start_1 = j + 1;
                }
            }
        }
        // mySerial.printf("box=%d 1s:%d 1e:%d 2s:%d 2e:%d\n", i, start_1, end_1, start_2, end_2);
        // add lets to outer leds

        // sequence 1 - to the end
        if (start_1 > end_1)
        {
            end_1 += 6;
        }
        for (int a = start_1; a < end_1; a++)
        {
            int led_pos = tmp->side_start_led(a % 6);
            // mySerial.printf("seq1 adding side %d\n", a % 6);
            for (int l = 0; l < LED_IN_SIDE; l++)
            {
                outer_led_adr[led_count_1] = led_pos + l;
                led_count_1++;
            }
        }
        // sequence 2 - coming back
        if (start_2 > end_2)
        {
            end_2 += 6;
        }
        for (int a = end_2 - 1; a >= start_2; a--)
        {

            int led_pos = tmp->side_start_led(a % 6);
            // mySerial.printf("seq2 adding side %d\n", a % 6);
            for (int l = 0; l < LED_IN_SIDE; l++)
            {
                outer_led_adr[(outer_led_num - 1) - led_count_2] = led_pos + ((LED_IN_SIDE - 1) - l);
                led_count_2++;
            }
        }
    }
}
void Hex_controller::calculate_ledCount_inDir()
{
    int8_t HorzMax, HorzMin, VertMax, VertMin;

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
    HorzCount = 2 + 2 * LED_IN_SIDE + (HorzMax - HorzMin) * (LED_IN_SIDE + 1);
    VertCount = 3 * LED_IN_SIDE + (VertMax - VertMin) * (LED_IN_SIDE * 2) + 1;
    // mySerial.printf("cnt horz-%d verz-%d\n", HorzCount, VertCount);
}
// basic fill functions
void Hex_controller::fill_leds_on_Vert_lvl(uint8_t lvl, CRGB clr)
{
    // mySerial.printf("fill vert lvl %d\n", lvl);
    for (uint8_t i = 0; i < NUM_BOXES; i++)
    {
        int line_lvl = lvl - (10 * position[i][0]);
        if (line_lvl >= 0 && line_lvl <= 15)
            nodes[i]->fill_vert_line(clr, leds, 15 - line_lvl);
    }
}
void Hex_controller::fill_leds_on_Hor_lvl(uint8_t lvl, CRGB clr)
{
    // mySerial.printf("fill vert lvl %d\n", lvl);
    // printCRGB(clr);

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
// user input
void Hex_controller::set_pre_anim(uint8_t n)
{
    mode = PresetAnim;
    fill_done = true;
    preset = n;
    step = 0;
    animation_step = 0;
    change = true;
}
void Hex_controller::set_speed(uint16_t spd)
{
    drawEveryNthMs = spd;
    change = true;
}
void Hex_controller::set_color(CRGB clr, int idx)
{
    clr_arr[idx] = clr;
    printCRGB(clr_arr[idx]);
    change = true;
}
void Hex_controller::set_brigtness(uint8_t b)
{
    maxBrightness = b;
    change = true;
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
// not sure if using this at all
void Hex_controller::pause_play(bool f)
{
    change = f ? true : false;
}
// internal functions
void Hex_controller::change_mode(Mode m)
{
    fill_mode = Not_fill_mode;
    fill_done = true;
    change = true;
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
        step_count = HorzCount;
        step = -1;
        break;
    case Fill_by_lines_fRight: // vertical
        step_count = VertCount;
        step = -1;
        break;
    case Fill_by_rotation_fTop:
        step_count = TOTAL_LEDS;
        step = -1;
        break;
    case Fill_by_lines_fLeft:
    case Fill_by_lines_fBottom:
    case Fill_by_rotation_fBottom:
    case Fill_by_lines_meet_in_midle_LR:
    case Fill_by_lines_meet_in_midle_TB:
        break;
    default: // Stationar
        drawEveryNthMs = 1000;
        break;
    }
    // mySerial.print("step count is: ");
    // mySerial.println(step_count);
    // mySerial.print("step is: ");
    // mySerial.println(step);
}
void Hex_controller::printCRGB(CRGB clr)
{
    mySerial.printf("R: %x G: %x B: %x\n", clr.r, clr.g, clr.b);
}

void Hex_controller::update()
{
    if (millis() > (lastDrew + drawEveryNthMs) && change)
    {
        // mySerial.print("updating now: ");
        if (rainbow > 0)
        {
            // mySerial.print("rainbow hue is: ");
            // mySerial.println(rainbow);
            clr_arr[0].setHue(rainbow);
            // printCRGB(clr_arr[0]);
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
            if (preset != 0 && fill_done)
            {
                // mySerial.println("animation update");
                animation_step++;
                if (1 < animation_step)
                { // for presets with len 2
                    animation_step = 0;
                }
                // mySerial.print("preset fill anim step:");
                // mySerial.print(animation_step);
                // mySerial.print(" and preset: ");
                // mySerial.println(preset);
                switch (preset)
                {
                case 1:
                    change_fill_mode(preset1[animation_step]);
                    break;
                case 2:
                    change_fill_mode(preset2[animation_step]);
                    break;
                case 3:
                    change_fill_mode(preset3[animation_step]);
                    break;
                case 4:
                    change_fill_mode(preset4[animation_step]);
                    break;
                default:
                    // notihing
                    break;
                }
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
            // nodes[0]->fill_n_leds(CHSV(100, 255, 250), leds, LEDS_IN_BOX);
            mySerial.println("ERROR: False mode");

            break;
        }
        if (!fill_done && fill_mode != Not_fill_mode)
        {

            if (rainbow)
            {
                clr_arr[animation_step] = clr_arr[0];
            }
            switch (fill_mode)
            {
            case Fill_by_lines_fTop:
            case Fill_by_lines_fBottom:
                if (step_count >= 0 && HorzCount >= step_count)
                {
                    fill_leds_on_Hor_lvl(step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_fLeft:
            case Fill_by_lines_fRight:
                if (step_count >= 0 && VertCount >= step_count)
                {
                    fill_leds_on_Vert_lvl(step_count, clr_arr[animation_step]);
                }
                else
                {
                    fill_done = true;
                }
                break;
            case Fill_by_lines_meet_in_midle_LR:
                if (step_count >= 0 && (VertCount / 2) >= step_count)
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
                if (step_count >= 0 && (HorzCount / 2) >= step_count)
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
        FastLED.show();
    }
}