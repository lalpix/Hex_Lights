#include "Hex_draw.h"

void  Hex_controller::calculate_outer_leds(){
    int num_of_contact_points=0;
    int direc_arr[6][2]={{-1,-1},{-1,1},{0,2},{1,1},{1,-1},{0,-2}};
    for(int i=0;i<NUM_BOXES;i++){
    //mySerial.print("box num: ");
    //mySerial.println(i);
    int x=position[i][0];
    int y=position[i][1];
    for(int k=0;k<6;k++){
        int n = (x+direc_arr[k][0]);
        int m = (y+direc_arr[k][1]);
        for(int q=0;q<NUM_BOXES;q++){
        if(position[q][0]==n && position[q][1]==m && q!=i){
            //mySerial.print("has contact at side: ");
            //mySerial.print(k);
            //mySerial.print(" with box num: ");
            //mySerial.println(q);
            points_of_contact[i][k]=q+1; //+1 so 0 is no contant point
            num_of_contact_points++;
        }
        }
    }
    }
    outer_led_num=TOTAL_LEDS-(num_of_contact_points*LED_IN_SIDE);

}
void  Hex_controller::create_outer_path(){//NOT FOR ALL SETUPS  
    uint16_t led_count_1=0;
    uint16_t led_count_2=0;
    uint8_t start_1=0;
    uint8_t end_1 =0;
    uint8_t start_2=0;
    uint8_t end_2 =0;
    for(int i=0;i<NUM_BOXES;i++){
    start_1=nodes[i]->side_start_led(0);
    end_1 =nodes[i]->side_start_led(0) +LEDS_IN_BOX;
    start_2=nodes[i]->side_start_led(0);
    end_2 =nodes[i]->side_start_led(0) +LEDS_IN_BOX;
    for(int j=0;j<6;j++){
        if(points_of_contact[i][j]!=0){
        if(points_of_contact[i][j]>i){//its contanct with next box
            end_1=nodes[i]->side_start_led(j);
            start_2=nodes[i]->side_start_led(j+1);
        }else{//its contantc with previous box
            end_2=(j!=0) ? nodes[i]->side_start_led(j) : nodes[i]->side_start_led(j)+30;
            start_1=nodes[i]->side_start_led(j+1);
        }
        }  
    }
    for(int a=start_1;a<end_1;a++){
        outer_led_adr[led_count_1]=a;
        led_count_1++;
    }
    if(start_2<end_2){
        for(int a=end_2;a>start_2;a--){
        outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
        led_count_2++;
        }
    }else{
        uint16_t end_22 = end_2;
        uint16_t start_22= start_2-(start_2%30);
        end_2 = start_22+30;
        for(int a=end_2;a>start_2;a--){
        outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
        led_count_2++;
        }
        for(int a=end_22;a>start_22;a--){
        outer_led_adr[(outer_led_num-1)- led_count_2]=a-1;
        led_count_2++;
        }
    }
    }
}
//basic fill functions
void  Hex_controller::fill_leds_on_Vert_lvl(uint8_t lvl,CRGB clr){
    for (uint8_t i = 0; i < NUM_BOXES; i++){
    int line_lvl= lvl-(10*position[i][0])-1;
    if(line_lvl>=0 && line_lvl<15)
        nodes[i]->fill_vert_line(clr,leds,line_lvl);        
    }
    }
void  Hex_controller::fill_leds_on_Hor_lvl(uint8_t lvl,CRGB clr){
    for (uint8_t i = 0; i < NUM_BOXES; i++){
    int line_lvl= lvl-(6*position[i][1]);
    if(line_lvl>=0 && line_lvl<12)
        nodes[i]->fill_hor_line(clr,leds,line_lvl);        
    }
    }
void  Hex_controller::fill_same_dir_sides(CRGB clr,int direc){
    direc = direc % 6;
    for(int i=0;i<NUM_BOXES;i++){//side is <0-5>
    nodes[i]->fill_side(clr,leds,direc);
    }
}
void  Hex_controller::fill_all_hex(CRGB clr){
    //mySerial.print("filling all hex with clr: ");
    printCRGB(clr);
    for(int i = 0;i<TOTAL_LEDS;i++){
    leds[i] = clr;
    }
}
void  Hex_controller::fill_one_led_all_hex(CRGB clr,uint8_t n){
    for(int i=0;i<NUM_BOXES;i++){
    nodes[i]->fill_one_led(clr,leds,n);
    }
}
//user input
void  Hex_controller::set_pre_anim(uint8_t n){
    mode = PresetAnim;
    fill_done = true;
    preset = n;
    step=0;
    animation_step=0;
    change = true;
}
void  Hex_controller::set_speed(uint16_t spd){
    drawEveryNthMs=spd;
    change = true;
    }
void  Hex_controller::set_color(CRGB clr,int idx){
    clr_arr[idx]=clr;
    printCRGB(clr_arr[idx]);
    change = true;
    }
void  Hex_controller::set_brigtness(uint8_t b){
    brightness=b;
    change = true;
}
void  Hex_controller::set_serial(HardwareSerial &ser){
    mySerial = ser;
}
void  Hex_controller::set_fade(bool f){
    fade=f;
}
void  Hex_controller::set_rainbow(int r){
    if(r==1){
    rainbow = rgb2hsv_approximate(clr_arr[0]).h;
    }else{rainbow=0;}
    //mySerial.print("rainbow is now: ");
    //mySerial.println(rainbow);
}
// not sure if using this at all
void  Hex_controller::pause_play(bool f){
    change = f ? true : false;
}
//internal functions
void  Hex_controller::change_mode(int m){
    fill_mode = Not_fill_mode;
    fill_done = true;
    change = true;
    mode = (Mode)m;
    preset = 0;
    step_count=0;
    step=1;
    }

void  Hex_controller::change_fill_mode(FillMode new_fill_mode){
    //mySerial.print("changing fill mode to: ");
    //mySerial.println(new_fill_mode);
    fill_mode = new_fill_mode;
    fill_done = false;
    lastDrew = 0;
    step_count=0;
    step = 1;

    switch (fill_mode)
    {
    case Fill_by_lines_fTop ://horizontal
        step_count = MAX_HORZ_LVL;
        step = -1;
        break;
    case Fill_by_lines_fRight://vertical
        step_count = MAX_VERT_LVL;
        step=-1;
        break;
    case Fill_by_rotation_fTop:
        step_count = TOTAL_LEDS;
        step = -1;
        break;
    case Fill_by_lines_fLeft:
    case Fill_by_lines_fBottom :
    case Fill_by_rotation_fBottom:
    case Fill_by_lines_meet_in_midle_LR:
    case Fill_by_lines_meet_in_midle_TB:
        break;
    default: //Stationar
        drawEveryNthMs = 1000;
        break;  
    }
    //mySerial.print("step count is: ");
    //mySerial.println(step_count);
    //mySerial.print("step is: ");
    //mySerial.println(step);
}
void  Hex_controller::printCRGB(CRGB clr){
    mySerial.printf("R: %h G: %h B: %h\n",clr.r,clr.g,clr.b);
}

void  Hex_controller::update(){
    if(millis() > (lastDrew + drawEveryNthMs) && change){
    //mySerial.print("updating now: ");
    if(rainbow>0){
        //mySerial.print("rainbow hue is: ");
        //mySerial.println(rainbow);
        clr_arr[0].setHue(rainbow);
        printCRGB(clr_arr[0]);
        rainbow++;
        if(rainbow>=255)
        rainbow=1;  
    }
    switch(mode){
        case Stationar:{
        //mySerial.println("stationar");
        fill_all_hex(clr_arr[0]);
        //change = false;
        break;  }
        case StationarOuter:{
        for(int i=0;i<NUM_BOXES;i++){
            for(int j=0;j<6;j++){
            if (!(points_of_contact[i][j])){//negated
            nodes[i]->fill_side(clr_arr[0],leds,j);
            //mySerial.print("Set side: ");
            //mySerial.print(j);
            //mySerial.print(" in box ");
            //mySerial.println(i);
            }
            }
        }
        break;}
        case Rotation:{
        //mySerial.println("rotate");
        for(int i=0;i<NUM_BOXES;i++){
            if(!fade){
            fill_one_led_all_hex(clr_arr[0],(step_count+1)%LEDS_IN_BOX);
            fill_one_led_all_hex(CRGB::Black,step_count%LEDS_IN_BOX);
            }
            else{
            fill_one_led_all_hex(clr_arr[0],step_count%LEDS_IN_BOX);
            }
            }
            step_count++;                 
        break;}
        case RotationOuter:{////this needs testing -----------------------------
        //mySerial.println("rotateOuter"); //using preset as util var
        if(step_count>=outer_led_num)
            step_count=0;
        leds[outer_led_adr[step_count]]=clr_arr[0];
        step_count++;           
        break;}
        case RandColorRandHex:{
        if(step_count>drawEveryNthMs){
        uint8_t idx = rand()%NUM_BOXES;
        nodes[idx]->fill_hex(CHSV(random8(),255,255),leds);
        }
        step_count++;
        break;}
        case PresetAnim:{
        if(preset!=0 && fill_done){
            //mySerial.println("animation update");
            animation_step++;
            if(1<animation_step){ //for presets with len 2
            animation_step =0;  
            } 
            //mySerial.print("preset fill anim step:");
            //mySerial.print(animation_step);
            //mySerial.print(" and preset: ");
            //mySerial.println(preset);
            switch(preset){
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
                //notihing
                break;
            }
        }
        break;}
        case RainbowSwipeVert:{
        for(int i=0;i<=MAX_VERT_LVL;i++){
            fill_rainbow(vert_leds,MAX_VERT_LVL,step_count);
            //CRGB clr=clr.setHue(((step_count+i)*step)%255);
            fill_leds_on_Vert_lvl(i,vert_leds[i]);
        }
        step_count= (step_count>255) ? 0 : step_count+1; 
        break;}
        case RainbowSwipeHorz:{
        for(int i=0;i<MAX_HORZ_LVL;i++){
            fill_rainbow(horz_leds,MAX_HORZ_LVL,step_count);
            //CRGB clr=clr.setHue(((step_count+i)*step)%255);
            fill_leds_on_Hor_lvl(i,horz_leds[i]);
            }
        step_count= (step_count>255) ? 0 : step_count+1;
        break;}
        default:
        break;
        //mySerial.println("ERROR: False mode");
        }
    if(!fill_done && fill_mode!= Not_fill_mode){  
        //mySerial.print("filling with fillMode: ");
        //mySerial.println(fill_mode);
        //mySerial.print("Step count is: ");
        //mySerial.println(step_count);
        //mySerial.print("Step is: ");
        //mySerial.println(step);
        if(rainbow){clr_arr[animation_step]=clr_arr[0];}
        switch (fill_mode){
        case Fill_by_lines_fTop :
        case Fill_by_lines_fBottom :
            if(step_count>= 0 && MAX_HORZ_LVL >= step_count){
            fill_leds_on_Hor_lvl(step_count,clr_arr[animation_step]);}
            else{fill_done=true;}
            break;
        case Fill_by_lines_fLeft:
        case Fill_by_lines_fRight:
            if(step_count>=0 && MAX_VERT_LVL >= step_count){ 
            fill_leds_on_Vert_lvl(step_count,clr_arr[animation_step]);}
            else{fill_done=true;}
            break;
        case Fill_by_lines_meet_in_midle_LR:
            if(step_count>= 0 && (MAX_VERT_LVL/2) >= step_count){
            fill_leds_on_Vert_lvl(step_count,clr_arr[animation_step]);
            fill_leds_on_Vert_lvl(MAX_VERT_LVL-step_count,clr_arr[animation_step]);}
            else{fill_done=true;}
            break;
        case Fill_by_lines_meet_in_midle_TB:
            if(step_count>= 0 && (MAX_HORZ_LVL/2) >= step_count){
            fill_leds_on_Hor_lvl(step_count,clr_arr[animation_step]);
            fill_leds_on_Hor_lvl(MAX_HORZ_LVL-step_count,clr_arr[animation_step]);}
            else{fill_done=true;}
            break;
        case Fill_by_rotation_fTop:
        case Fill_by_rotation_fBottom:
            if(step_count <= TOTAL_LEDS){
            leds[step_count]= clr_arr[animation_step];}
            else{fill_done=true;}
            break;
        }
        step_count+=step;
        }
    lastDrew=millis();
    if(fade){
        for(int i=0;i<TOTAL_LEDS;i++){//use brightness
        leds[i].fadeToBlackBy(brightness);
        }
    }
    FastLED.show();
    
}
}
