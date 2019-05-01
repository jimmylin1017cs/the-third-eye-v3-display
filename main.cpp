#include "image.h"
#include "mjpeg_streaming.h"
#include "socket_server.h"
#include "DAI_pull.h"
#include "fusion.h"

#include <iostream>
#include <vector>
#include <map>

#include <fstream> // read tag file

//static void * cap;
static cv::Mat m;
static image im;
static image **alphabet;
static std::vector<unsigned char> frame;
static std::vector<person_box> boxes;

static std::map<int, person_location> beacon_data;

static std::map<std::string, image> tag_pics;

static std::map<double, std::vector<unsigned char>> frame_buffer;
static std::vector<double> frame_buffer_remove;

// sort id -> name
static std::map<int, std::string> id_map_name;

// dataset id -> name
static std::map<int, std::string> all_name_data;
static std::vector<std::string> enable_name;
static std::vector<int> display_ids;

static bool begin_with_boxes = false;

// load tag pic function
void load_tag_pic();
void draw_tag_pic(image im, std::string tag_name, int &all_tag_count, int &legal_tag_count);

// load enable tag
void load_enable_tag();

// declare in image.c
static char fusion_name_result[MAX_SORT_NUM][30];
// just for old fusion
// void do_fusion(Fusion_result *match_result, int *match_result_size, image im, int num, float thresh, 
//    box *boxes, float **probs, char **names, image **alphabet, int classes, int *sort_ids)
//void call_fusion(int frameCt, image im, int num, float thresh, box *boxes, float **probs, char **names, image **alphabet, int classes, int *sort_ids)
void call_fusion(int frameCt, image im)
{
    // define in image.h
    //int FPS = 15;
    //int FUSION_ENABLE = 1;

    // declare in draw_detections_sort()
    Fusion_result match_result[MAX_DATA_PATH_NUM];
    int fusion_suc = 0;
    int match_result_size = 0;
    
    //if (FUSION_ENABLE && frameCt%FPS==0 && frameCt!=0)
    if (FUSION_ENABLE && frameCt != 0)
    {
        do_fusion(match_result, &match_result_size, im, boxes, beacon_data, fusion_name_result);
    }

    id_map_name.clear();

    for(int i = 0; i < boxes.size(); i++)
    {
        int id = boxes[i].id;
        if(strlen(fusion_name_result[id]) == 0)
        {
            id_map_name[id] = "unknown";
        }
        else
        {
            id_map_name[id] = fusion_name_result[id];
        }
    }

    for(auto &it : id_map_name)
    {
        std::cout<<it.first<<" : "<<it.second<<std::endl;
    }
}

int main()
{
    /*float box_x = (575 + 681) / 2;
    float box_y = (177 + 254) / 2;
    std::cout<<box_x<<", "<<box_y<<std::endl;
    std::vector<cv::Point2f> box_position = { cv::Point2f(box_x, box_y) };
    std::vector<cv::Point2f> beacon_position;
    convert_coordinate(box_position, beacon_position);

    std::cout<<beacon_position[0].x<<", "<<beacon_position[0].y<<std::endl;*/


    load_tag_pic();
    alphabet = load_alphabet();
    int red = 0, green = 255, blue = 0; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2
    std::string name = "person_name";

    int legal_tag_count = 0;
    int all_tag_count = 0;
    double time_stamp = 0;

    //cv::VideoCapture *cap = new cv::VideoCapture("time_counter.flv");
    //cv::VideoCapture *cap = new cv::VideoCapture("test.MTS");

    //iot_init();

    frame_buffer.clear();

    while(1)
    {
        time_stamp = receive_frame_with_time_stamp(frame, 8091, 200, 95);

        if(time_stamp)
        {
            std::cout<<"time_stamp : "<<time_stamp<<std::endl;

            m = cv::imdecode(frame, 1);
            im = mat_to_image(m);
            m.release();
            
            char time_stamp_label_str[4096] = {0};
            sprintf(time_stamp_label_str, "%lf - %d", time_stamp, frame_buffer.size());
            image time_stamp_label = get_label(alphabet, time_stamp_label_str, (im.h*.03));
            draw_label(im, 0, 0, time_stamp_label, rgb);
            free_image(time_stamp_label);

            m = image_to_mat(im);
            free_image(im);
            send_mjpeg(m, 8090, 200, 75);
            m.release();
            continue;

            frame_buffer.insert(std::pair<int, std::vector<unsigned char>>(time_stamp, frame));
        }
            
        /*
        
        /*if(!frame.empty())
        {
            m = cv::imdecode(frame, 1);
            send_mjpeg(m, 8090, 200, 95);
        }*/

        boxes.clear();
        //iot_talk_receive(boxes, display_ids, beacon_data);

        // load enable tag for draw
        //load_enable_tag();
        /*for(auto &it : enable_name)
        {
            std::cout<<it<<std::endl;
        }*/

        //if(!frame.empty())

        //std::cout<<"@@@@@@@@@@ boxes.size() : "<<boxes.size()<<std::endl;

        if(boxes.size())
        {
            begin_with_boxes = true;

            // -----------------------------------------------------
            // ---------------frame synchronize---------------------
            // -----------------------------------------------------
            int current_time_stamp = boxes[0].time_stamp;
            if(current_time_stamp > time_stamp) continue;

            if(frame_buffer.find(current_time_stamp) != frame_buffer.end())
            {
                m = cv::imdecode(frame_buffer[current_time_stamp], 1);
                im = mat_to_image(m);

                call_fusion(current_time_stamp, im);
            }
            else
            {
                continue;
            }

            frame_buffer_remove.clear();
            for(auto &it : frame_buffer)
            {
                if(it.first < current_time_stamp)
                {
                    frame_buffer_remove.push_back(it.first);
                }
            }

            red = 0;
            green = 255;
            blue = 0;
            rgb[0] = red;
            rgb[1] = green;
            rgb[2] = blue;

            char time_stamp_label_str[4096] = {0};
            sprintf(time_stamp_label_str, "%d - %d", current_time_stamp, frame_buffer.size());
            image time_stamp_label = get_label(alphabet, time_stamp_label_str, (im.h*.03));
            draw_label(im, 0, 0, time_stamp_label, rgb);

            for(auto &it : frame_buffer_remove)
            {
                frame_buffer.erase(it);
            }

            // -----------------------------------------------------
            // ----------------draw box & tag picture---------------
            // -----------------------------------------------------

            int is_legal_tag = 0;
            int legal_tag = 0;
            char person_tag[30];

            int legal_tag_count = 0;
            int all_tag_count = 0;

            width = im.h * .006;
            alphabet_size = im.h*.03;
            //alphabet_size = (im.h*.03)/10;

            // create enable name
            enable_name.clear();
            for(int i = 0; i < display_ids.size(); i++)
            {
                std::string tmp_name = all_name_data[display_ids[i]];
                std::string display_name = tmp_name.substr(0, tmp_name.find_last_of("."));
                std::cout<<"@@@@@@@@@@ enable_name : "<<display_name<<std::endl;
                enable_name.push_back(display_name);
            }

            for(int i = 0; i < boxes.size(); i++)
            {
                int id = boxes[i].id;

                left = boxes[i].x1;
                top = boxes[i].y1;
                right = boxes[i].x2;
                bot = boxes[i].y2;

                if(id_map_name.find(id) != id_map_name.end())
                {
                    std::string name = id_map_name[id];

                    //std::cout<<"@@@@@@@@@@ name : "<<name<<std::endl;

                    // not enable name
                    if(std::find(enable_name.begin(), enable_name.end(), name) == enable_name.end())
                    {
                        continue;
                    }


                    image label = get_label(alphabet, (char *)name.c_str(), alphabet_size);

                    if(name != "unknown")
                    {
                        red = 0;
                        green = 255;
                        blue = 0;
                        rgb[0] = red;
                        rgb[1] = green;
                        rgb[2] = blue;
                    }
                    else
                    {
                        red = 255;
                        green = 0;
                        blue = 0;
                        rgb[0] = red;
                        rgb[1] = green;
                        rgb[2] = blue;
                    }

                    draw_box_width(im, left, top, right, bot, width, red, green, blue);
                    draw_label(im, top + width, left, label, rgb);

                    /*if (FUSION_ENABLE && tag_pics.find(name) != tag_pics.end())
                    {
                        std::cout<<"draw_tag_pic"<<std::endl;
                        // draw right board for picture in tag_pic folder
                        draw_tag_pic(im, name, all_tag_count, legal_tag_count);
                    }*/
                }
            }

            m = image_to_mat(im);
            free_image(im);
            send_mjpeg(m, 8090, 200, 75);
        }

        /*if(!begin_with_boxes && frame_stamp % 3 == 0 && frame_buffer.find(frame_stamp) != frame_buffer.end())
        {
            std::cout<<"begin_with_boxes"<<std::endl;

            m = cv::imdecode(frame_buffer[frame_stamp], 1);
            im = mat_to_image(m);

            char frame_stamp_label_str[4096] = {0};
            sprintf(frame_stamp_label_str, "%d - %d", frame_stamp, frame_buffer.size());
            image frame_stamp_label = get_label(alphabet, frame_stamp_label_str, (im.h*.03));
            draw_label(im, 0, 0, frame_stamp_label, rgb);

            m = image_to_mat(im);
            send_mjpeg(m, 8090, 200, 95);
        }*/
    }

    return 0;
}

void load_tag_pic()
{
    tag_pics.clear();
    all_name_data.clear();

    std::ifstream in("tag_pic/tag_pic.txt");

    if(!in)
    {
        std::cout<<"Cannot open input file."<<std::endl;
        exit(EXIT_FAILURE);
    }

    std::string str, tag_str;
    int id = 0;

    while (std::getline(in, str))
    {
        all_name_data.insert(std::pair<int, std::string>(id, str));
        id++;
        // output the line
        tag_str = "tag_pic/" + str;
        str = str.substr(0, str.size() - 4); // remove .jpg
        std::cout<<str<<std::endl;
        std::cout<<tag_str<<std::endl;
        tag_pics.insert(std::pair<std::string, image>(str, resize_image(load_image_color((char *)tag_str.c_str(), 100, 100), 100, 100)));

        /*cv::Mat m = image_to_mat(tag_pics);
        imshow(str, m);
        waitKey(1);*/
    }

    in.close();
}

void draw_tag_pic(image im, std::string tag_name, int &all_tag_count, int &legal_tag_count)
{
    int tag_pic_width = 220;
    int is_legal_tag = 0;

    int dx = im.w - tag_pic_width + 60;
    int dy;

    if (tag_name != "unknown")
    {
        legal_tag_count++;
        dy = 40 + (int)100 * (legal_tag_count - 1); // legal tag position
        is_legal_tag = 1;
    }
    else
    {
        dy = 40 + (int)160 * (all_tag_count - legal_tag_count); // unknown tag position
    }

    all_tag_count++;

    dy += (is_legal_tag) ? 30 : (int)im.h / 2;

    embed_image(tag_pics[tag_name], im, dx, dy);

    // draw label on tag
    float rgb[3];
    if(is_legal_tag)
    {
        image label = get_label(alphabet, (char *)tag_name.c_str(), (im.h*.03)/10);
        
        rgb[0] = 0.0;
        rgb[1] = 1.0;
        rgb[2] = 0.0;

        //draw_label(im, dy, dx - 40, label, rgb);
        draw_label(im, dy, dx, label, rgb);
        free_image(label);
    }
    else
    {
        image label = get_label(alphabet, (char *)tag_name.c_str(), (im.h*.03)/16);

        rgb[0] = 1.0;
        rgb[1] = 0.0;
        rgb[2] = 0.0;

        draw_label(im, dy, dx, label, rgb);
        free_image(label);
    }
}

void load_enable_tag()
{
    std::ifstream in("selector/enable_tag.txt");

    if(!in)
    {
        std::cout<<"Cannot open enable_tag.txt."<<std::endl;
        exit(EXIT_FAILURE);
    }

    std::string str, tag_str;

    enable_name.clear();
    while (std::getline(in, str))
    {
        enable_name.push_back(str);
    }

    in.close();
}

// draw box sample code
/*if(boxes.size())
{
    int current_frame_stamp = boxes[0].frame_stamp;

    if(frame_buffer.find(current_frame_stamp) != frame_buffer.end())
    {
        m = cv::imdecode(frame_buffer[current_frame_stamp], 1);
        im = mat_to_image(m);
    }
    else
    {
        continue;
    }

    frame_buffer_remove.clear();
    for(auto &it : frame_buffer)
    {
        if(it.first < current_frame_stamp)
        {
            frame_buffer_remove.push_back(it.first);
        }
    }

    char frame_stamp_label_str[4096] = {0};
    sprintf(frame_stamp_label_str, "%d - %d", current_frame_stamp, frame_buffer.size());
    image frame_stamp_label = get_label(alphabet, frame_stamp_label_str, (im.h*.03));
    draw_label(im, 0, 0, frame_stamp_label, rgb);

    for(auto &it : frame_buffer_remove)
    {
        frame_buffer.erase(it);
    }

    for(int i = 0; i < boxes.size(); i++)
    {
        printf("%d = %d\n", boxes[i].frame_stamp, boxes[i].id);

        left = boxes[i].x1;
        top = boxes[i].y1;
        right = boxes[i].x2;
        bot = boxes[i].y2;

        boxes[i].frame_stamp;

        printf("person id : %d\nx1: %d, y1: %d\nx2: %d, y2: %d\n", boxes[i].id, left, top, right, bot);

        width = im.h * .006;
        draw_box_width(im, left, top, right, bot, width, red, green, blue);

        char labelstr[4096] = {0};
        sprintf(labelstr, "%d", boxes[i].id);
        image label = get_label(alphabet, labelstr, (im.h*.03));
        draw_label(im, top + width, left, label, rgb);
    }

    m = image_to_mat(im);
    send_mjpeg(m, 8090, 200, 95);
}*/

// draw tag pic sample code
/*while(1)
{
    //iot_receive(frame, boxes);

    if(!frame.empty())
    {
        m = cv::imdecode(frame, 1);
        im = mat_to_image(m);
        
        width = im.h * .006;
        alphabet_size = im.h*.03;

        image label;

        legal_tag_count = 0;
        all_tag_count = 0;

        for(int i = 0; i < boxes.size(); i++)
        {
            if(boxes[i].name == "unknown")
            {
                red = 255; green = 0; blue = 0;
            }
            else
            {
                red = 0; green = 255; blue = 0;
            }

            rgb[0] = red; rgb[1] = green; rgb[2] = blue; // setting color

            left = boxes[i].x1;
            top = boxes[i].y1;
            right = boxes[i].x2;
            bot = boxes[i].y2;
            draw_box_width(im, left, top, right, bot, width, red, green, blue);
            image label = get_label(alphabet, (char *)boxes[i].name.c_str(), alphabet_size);
            draw_label(im, top + width + alphabet_size, left, label, rgb);

            if(tag_pics.find(boxes[i].name) != tag_pics.end())
            {
                // draw right board for picture in tag_pic folder
                draw_tag_pic(im, boxes[i].name, all_tag_count, legal_tag_count);
                //std::cout<<"find tag picture, "<<all_tag_count<<", "<<legal_tag_count<<std::endl;
            }
        }

        //draw_box_width(im, left, top, right, bot, width, red, green, blue);
        
        //image label = get_label(alphabet, (char *)name.c_str(), alphabet_size);
        //draw_label(im, top + width + alphabet_size, left, label, rgb);

        m = image_to_mat(im);

        //send_mjpeg(m, 8090, 200, 95);

        cv::imshow("Receiver", m);
        cv::waitKey(1);
    }
}*/
