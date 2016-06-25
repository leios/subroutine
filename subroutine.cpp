/*-------------subroutine.cpp-------------------------------------------------//
*
* Purpose: We are creating a webcomic with code using cairo.
*
*   Notes: This is where we keep main()
*          Prime numbers of slides have not been implemented
*-----------------------------------------------------------------------------*/

#include <cairo.h>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

// defining a global variable for the number of panels
#define num_panel 1

// Class to hold all panel data
class panel{
    public:
        int box_height, box_width, lw;
        cairo_surface_t *panel_surface[num_panel];
        cairo_t *panel_ctx[num_panel];
        cairo_surface_t *bg_surface;
        cairo_t *bg_ctx;
        double gap;
        double xpos[num_panel], ypos[num_panel];
};

// Class for people (stick figures), arms, legs, etc, are all read from l to r.
// Note: unfinished
class people{
    public: 
        double pos_x, pos_y, head_x, head_y, arm[2], leg[2], wrist[2], shin[2],
               width, height;

        // direction person is looking -- -1 left, 0 is middle, 1 right
        int direction;
};

// Function to draw initial layout
void create_scene(panel &comic);
void write_image(panel comic, std::string pngname);
void destroy_all(panel &comic);

// Function to draw from an array of points
// Drawing without interpolation may eb done on-the-fly
void draw_array(std::vector<double> &x, std::vector<double> &y, int panel_num,
                panel &comic);

// Function to find Bezier control points
void find_bpoints(std::vector<double> &p1, std::vector<double> &p2, 
                  std::vector<double> &K);

// Function to draw a stick figure
void stick_figure(double height, double pos_x, double pos_y, cairo_t *ctx,
                  people &person);

// consec_harmonic in beta, buggy
// Create a function for consecutive harmonic potentials
void consec_harmonic(std::vector<double> pos_x, std::vector<double> pos_y, 
                     double harmonic_num, int panel_num, double scale,
                     panel &comic);

// pointy_arc not implemented
// Draw a pointy arc
void pointy_arc(double pos_x, double pos_y, double radius, double start_angle,
                double end_angle, double scale, int panel_num, panel &comic);

// writing textbubbles
void write_text(std::string text, people person, int panel_num, panel comic);

// Create person (stickfigure for now)
void draw_person(people &person, cairo_t *ctx);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/
int main (){

    panel comic;
    comic.box_height = 2000;
    comic.box_width  = 2000;
    comic.gap        = 10.0;
    comic.lw         = 5;

    create_scene(comic);

    std::vector <double> x(5), y(5);
    x[0] = 0; y[0] = 2000;
    x[1] = 0; y[1] = 1750;
    x[2] = 1000; y[2] = 1500;
    x[3] = 2000; y[3] = 1750;
    x[4] = 2000; y[4] = 2000;

    for (size_t i = 0; i < num_panel; ++i){

        // Drawing background first
        cairo_pattern_t *pat
            = cairo_pattern_create_linear (0.0, 2000,  0.0, 0.0);
        cairo_pattern_add_color_stop_rgba(pat, 1, .3, .3, 1, 1);
        cairo_pattern_add_color_stop_rgba(pat, 0, 1, 1, 1, 1);
        cairo_set_source(comic.panel_ctx[i], pat);
        cairo_paint(comic.panel_ctx[i]);

        cairo_arc (comic.panel_ctx[i], 0, 0, 300, 2*M_PI, 1.5 * M_PI);
        cairo_move_to(comic.panel_ctx[i], 10, 10);
        cairo_set_source_rgb(comic.panel_ctx[i], 1.0, 1.0, 0);
        cairo_fill_preserve(comic.panel_ctx[i]);
        cairo_set_source_rgb(comic.panel_ctx[i], 1.0, 0.5, 0);
        cairo_stroke(comic.panel_ctx[i]);

        // create an array to work with
        draw_array(x, y, i, comic);
        cairo_move_to(comic.panel_ctx[i], 500, 500);
        cairo_set_source_rgb(comic.panel_ctx[i], 0, .75, 0);
        cairo_fill_preserve(comic.panel_ctx[i]);
        cairo_set_source_rgb(comic.panel_ctx[i], 0,.5, 0);
        cairo_stroke(comic.panel_ctx[i]);
        
        // Draw stick figure
        std::vector<people> Bob(num_panel);
        stick_figure(500, 1000, 1250, comic.panel_ctx[i], Bob[i]);

        write_text("Hello World!", Bob[i], i, comic);

    }

/*
    // Writing "Hello World!"
    cairo_set_font_size (comic.panel_ctx[0], 100.0);
    cairo_set_source_rgb (comic.panel_ctx[0], 1.0, 1.0, 1.0);
    cairo_move_to (comic.panel_ctx[0], 500, 750);
    cairo_show_text (comic.panel_ctx[0], "Hello World!");

    // Drawing line from stick figure to words
    // create array to draw through... because... yeah...
    std::vector<double> x_line(3), y_line(3);
    x_line[0] = 900; y_line[0] = 1050;
    x_line[1] = 800; y_line[1] = 950;
    x_line[2] = 750; y_line[2] = 800;

    draw_array(x_line, y_line, 0, comic);

    cairo_stroke(comic.panel_ctx[0]);
*/

    // Create array for consecutive harmonic segment
    //std::vector<double> pos_x(3), pos_y(3);
    //pos_x[0] = 0; pos_y[0] = 500;
    //pos_x[1] = 1000; pos_y[1] = 1000;
    //pos_x[2] = 2000; pos_y[2] = 1500;

    //consec_harmonic(pos_x, pos_y, 10, 0, .01, comic);

    write_image(comic, "subroutine.png");

    destroy_all(comic);

}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// Function to draw initial layout 
void create_scene(panel &comic){

    // Create bg surface and context
    // Note: The size of the surface is dependent on the number of boxes

    // Note: When considering box position, we assume a maximum width of 3 boxes
    int surface_width, surface_height;
    if (num_panel == 1){
        surface_width = comic.box_width + comic.gap;
        surface_height = comic.box_height + comic.gap;
    }
    else if (num_panel % 2 == 0){
        surface_width = (comic.box_width + comic.gap / 2) * 2 + comic.gap / 2;
        surface_height = (comic.box_height + comic.gap / 2) * (num_panel / 2)
                         + comic.gap / 2;
    }
    else{
        surface_width = (comic.box_width + comic.gap / 2) * 3 + comic.gap / 2;
        surface_height = (comic.box_height + comic.gap / 2)
                         * ceil(num_panel / 3.0) + comic.gap / 2;
    }

    // creating background for writing to
    comic.bg_surface = 
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surface_width, 
                                   surface_height);

    comic.bg_ctx = cairo_create(comic.bg_surface);

    // Create the panel surfaces and contexts
    for (int i = 0; i < num_panel; ++i){
        comic.panel_surface[i] = 
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, comic.box_width,
                                       comic.box_height);
        comic.panel_ctx[i] = cairo_create(comic.panel_surface[i]);

        // Setting border and source
        cairo_set_source_rgb(comic.panel_ctx[i], 0.0, 0.0, 0.0);
        cairo_set_line_width(comic.panel_ctx[i], comic.lw);
    }

    // Set the border and source
    cairo_set_source_rgb(comic.bg_ctx, 0.0, 0.0, 0.0);
    cairo_set_line_width(comic.bg_ctx, comic.lw * 2);

    // Now we need to draw the boxes

    // curr is the current box we are drawing
    int curr = 0;
    double xpos, ypos;
    
    // For 1 panel
    if (num_panel == 1){
        cairo_rectangle(comic.panel_ctx[0], 0, 0, 
                        comic.box_width, comic.box_height);

        // painting bg box to surface
        cairo_stroke(comic.panel_ctx[0]);
        comic.xpos[0] = comic.gap / 2;
        comic.ypos[0] = comic.gap / 2;
    }

    // For even numbers of panlse
    else if (num_panel % 2 == 0){
        curr = 0;
        for (size_t i = 0; i < 2; ++i){
            for (size_t j = 0; j < (size_t)(num_panel / 2); ++j){
                xpos = i * (comic.box_width + comic.gap / 2) + (comic.gap / 2);
                ypos = j * (comic.box_height + comic.gap / 2) + (comic.gap / 2);

                cairo_rectangle(comic.panel_ctx[curr], 0, 0, 
                                comic.box_width, comic.box_height);

                // painting bg box to surface
                cairo_stroke(comic.panel_ctx[curr]);
                comic.xpos[curr] = xpos;
                comic.ypos[curr] = ypos;
                curr += 1;
            }
        }

    }

    // For everything else, (except primes)
    else{
        curr = 0;

        // Multiples of 3 or not
        switch(num_panel % 3){

            // Multiple of 3
            case 0:
                std::cout << "we are in case 0" << '\n';
                for (size_t i = 0; i < 3; ++i){
                    for (size_t j = 0; j < ceil(num_panel / 3.0); ++j){
                        xpos = i * (comic.box_width + comic.gap / 2)
                               + (comic.gap / 2);
                        ypos = j * (comic.box_height + comic.gap / 2) 
                               + (comic.gap / 2);

                        cairo_rectangle(comic.panel_ctx[curr], 0, 0, 
                                        comic.box_width, comic.box_height);
                        // painting bg box to surface
                        cairo_stroke(comic.panel_ctx[curr]);
                        comic.xpos[curr] = xpos;
                        comic.ypos[curr] = ypos;
                        curr += 1;
                    }
                }
                break;

            // Not multiple of 3
            case 2:
                std::cout << "we are in case 2" << '\n';
                // creating initial rows of 3
                for (size_t i = 0; i < 3; ++i){
                    for (size_t j = 0; j < ceil(num_panel / 3.0) - 1; ++j){
                        xpos = i * (comic.box_width + comic.gap / 2)
                               + (comic.gap / 2);
                        ypos = j * (comic.box_height + comic.gap / 2) 
                               + (comic.gap / 2);
                        cairo_rectangle(comic.panel_ctx[curr], 0, 0, 
                                        comic.box_width, comic.box_height);

                        // painting bg box to surface
                        cairo_stroke(comic.panel_ctx[curr]);
                        comic.xpos[curr] = xpos;
                        comic.ypos[curr] = ypos;
                        curr += 1;
                    }
                }
                // creating final row of 2
                for (int i = 0; i < 2; ++i){
                    // The x and y positions here are defined around the 
                    // middle of the surface we are drawing to
                    xpos = surface_width / 2 + (i-1) * (comic.box_width);
                    if ((i-1) < 0){
                        xpos -= comic.gap / 4;
                    }
                    else{
                        xpos += comic.gap / 4;
                    }
                    ypos = (ceil(num_panel / 3.0) - 1) * ((comic.box_height 
                           + comic.gap / 2) + comic.gap / 2);
                    cairo_rectangle(comic.panel_ctx[curr], 0, 0,
                                    comic.box_width, comic.box_height);
                    // painting bg box to surface
                    cairo_stroke(comic.panel_ctx[curr]);
                    comic.xpos[curr] = xpos;
                    comic.ypos[curr] = ypos;
                    curr += 1;
                }
                break;

            case 1:
                std::cout << "primes have not been implemented, sorry!" << '\n';
        }
    }

    // writing and destroying rendering device
    // write_image(comic, "subroutine.png");

}

// Function to write to png
void write_image(panel comic, std::string pngname){
    for (size_t i = 0; i < num_panel; ++i){
        cairo_set_source_surface(comic.bg_ctx, comic.panel_surface[i],
                                 comic.xpos[i], comic.ypos[i]);
        cairo_paint(comic.bg_ctx);
    }
    cairo_surface_write_to_png (comic.bg_surface, pngname.c_str());
}

// Function to destroy all contexts and surfaces
void destroy_all(panel &comic){
    for (size_t i = 0; i < num_panel; ++i){
        cairo_destroy(comic.panel_ctx[i]);
        cairo_surface_destroy (comic.panel_surface[i]);
    }
    cairo_destroy (comic.bg_ctx);
    cairo_surface_destroy (comic.bg_surface);
}

// Function to draw points from an array
void draw_array(std::vector<double> &x, std::vector<double> &y, int panel_num,
                panel &comic){

    // Computing control points
    std::vector<double> p1x(x.size()), p2x(x.size()), 
                        p1y(x.size()), p2y(x.size());
    find_bpoints(p1x, p2x, x);
    find_bpoints(p1y, p2y, y);

    // Move to initial array point
    cairo_move_to (comic.panel_ctx[panel_num], x[0], y[0]);

    // Drawing the array
    for (size_t i = 0; i < x.size() - 1; ++i){
        //ypos = abs(y[i] - comic.box_height);
        cairo_curve_to(comic.panel_ctx[panel_num], p1x[i], p1y[i], 
                       p2x[i], p2y[i], x[i+1], y[i+1]);
    }

    //cairo_stroke(comic.panel_ctx[panel_num]);
}

// Function to find Bezier control points, these are ~representations of the 
// slope at the starting and end points
void find_bpoints(std::vector<double> &p1, std::vector<double> &p2, 
                  std::vector<double> &K){

    int n = K.size()-1;
    double m;

    std::vector<double> a(n), b(n), c(n), r(n);

    // Adding first elements to coefficient vectors
    a[0] = 0;
    b[0] = 2;
    c[0] = 1;
    r[0] = K[0] + 2*K[1];

    // Adding internal elements to coefficient vectors
    for (int i = 1; i < n - 1; ++i){
        a[i] = 1;
        b[i] = 4;
        c[i] = 1;
        r[i] = 4 * K[i] + 2 * K[i+1];
    }

    // Adding final element
    a[n - 1] = 2;
    b[n - 1] = 7;
    c[n - 1] = 0;
    r[n - 1] = 8 * K[n-1] + K[n];

    // Thomas algorithm to solve Ax=b

    // Resize p vectors to make sure they work
    p1.resize(n);
    p2.resize(n);

    // Computing coefficients
    for (int i = 1; i < n; ++i){
        m = a[i] / b[i-1];
        b[i] = b[i] - m * c[i - 1];
        r[i] = r[i] - m * r[i - 1];
    }

    // Computing p1
    p1[n-1] = r[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i){
        p1[i] = (r[i] - c[i] * p1[i+1]) / b[i];
    }

    // Now computing p2
    for (int i = 0; i < n-1; ++i){
        p2[i] = 2*K[i+1] - p1[i+1];
    }

    p2[n-1] = 0.5 * (K[n] + p1[n-1]);
}

// Function to draw a stick figure
// Note: position is for the center of stick figure
void stick_figure(double height, double pos_x, double pos_y, cairo_t *ctx,
                  people &person){

    // Draw head
    double head_y = pos_y - height * 0.333;
    double head_x = pos_x;
    double crotch_y = pos_y + 0.16666 * height;
    cairo_arc(ctx, head_x, head_y, .16666 * height, 0, 2*M_PI);

    // Fill head
    cairo_move_to(ctx, pos_x, head_y);
    cairo_set_source_rgb(ctx, 1.0, 1.0, 1.0);
    cairo_fill_preserve(ctx);
    cairo_set_source_rgb(ctx, 0, 0, 0);
    cairo_stroke(ctx);

    // Drawing body
    cairo_move_to(ctx, pos_x, head_y + 0.16666 * height);
    cairo_line_to(ctx, pos_x, pos_y + 0.16666 * height);

    // Drawing legs
    cairo_move_to(ctx, pos_x, crotch_y);
    cairo_line_to(ctx, pos_x + 0.05 * height, pos_y + 0.5 * height);
    cairo_move_to(ctx, pos_x, crotch_y);
    cairo_line_to(ctx, pos_x - 0.05 * height, pos_y + 0.5 * height);

    // Drawing arms
    cairo_move_to(ctx, pos_x, pos_y - 0.16666 * height);
    cairo_line_to(ctx, pos_x + 0.05 * height, crotch_y);
    cairo_move_to(ctx, pos_x, pos_y - 0.16666 * height);
    cairo_line_to(ctx, pos_x - 0.05 * height, crotch_y);

    // Giving person identity
    person.head_x = head_x;
    person.head_y = head_y;
    person.height = height;
    person.width = 2 * 0.16666 *  height;
    person.direction = -1;
    person.pos_x = pos_x;
    person.pos_y = pos_y;

    // Should draw arm and leg in future.

    cairo_stroke(ctx);
}

// Note: not working in current state. Requires straight line along horizontal
void consec_harmonic(std::vector<double> pos_x, std::vector<double> pos_y, 
                     double harmonic_num, int panel_num, double scale,
                     panel &comic){

    // determining length of consecutive harmonic segment
    double length = 0;
    std::vector<double> tot_length(pos_x.size());
    tot_length[0] = 0;
    for (size_t i = 1; i < pos_x.size(); ++i){
        length += sqrt((pos_x[i] - pos_x[i-1]) * (pos_x[i] - pos_x[i-1])
                       - (pos_y[i] - pos_y[i-1]) * (pos_y[i] - pos_y[i-1])); 
        tot_length[i] = length;
    }

    // Position of well
    double well_pos = (length / (2*(double)harmonic_num));
    double xtick, slope;
    int res = 10, length_index = 0;
    std::vector<double> x(res+1), y(res+1);

    // Drawing consecutive potentials
    for (int i = 0; i < harmonic_num; ++i){
        for (int j = 0; j <= res; ++j){

            xtick = (double)j * length / (double)harmonic_num / (double)res;
            y[j] = -scale * (xtick - well_pos) * (xtick - well_pos);
            x[j] = xtick + (double)i * length / (double)harmonic_num;
            //std::cout << xtick << '\t' << i << '\t' << length << '\t' 
            //          << harmonic_num << '\t' << x[j] << '\n';

            // Adding in x and y positions from the read in array
            // Find the segment of the provided array that we are closest to
            for (size_t k = 1; k < tot_length.size(); ++k){
                if (x[j] >= tot_length[k-1] && x[j] < tot_length[k]){
                    length_index = k-1;
                    std::cout << k << '\n';
                    //continue;
                }
            }

            // Probably breaking here.
            // finding slope of length_index / length_index - 1
            slope = (pos_y[length_index+1] - pos_y[length_index])
                    / (pos_x[length_index+1] - pos_x[length_index]);

            y[j] += pos_y[length_index] + slope * x[j];

            std::cout << pos_y[length_index] << '\t' << pos_x[length_index]
                      << '\t' << slope << '\t' << length_index << '\n';
            
            
            std::cout << xtick << '\t' << x[j] << '\t' << y[j] << '\n';

        }

        draw_array(x, y, panel_num, comic);
        cairo_stroke(comic.panel_ctx[panel_num]);

    }

}

// writing textbubbles
void write_text(std::string text, people person, int panel_num, panel comic){

    // Determing position relative to top of panel
    double overhead = comic.box_height - person.head_y;

    // Determine size in x of box
    double text_width = comic.box_width / 3.0, text_x;

    // Determing x position of text
    if(person.direction == -1){
        text_x = person.pos_x - 1.5 * person.width;
    }
    else if(person.direction == 0){
        text_x = person.pos_x;
    }
    else{
        text_x = person.pos_x + 1.5 * person.width;
    }

    // Determing how to parse the text before detemining y pos
    // COMING SOON!

    double text_y = person.head_y - overhead / 3.0;

    // Draw line from person to text
    std::vector<double> x_line(3), y_line(3);

    x_line[0] = person.head_x + 0.75 * (double)person.direction * person.width;
    x_line[2] = text_x;
    x_line[1] = ((x_line[2] + x_line[0]) / 2)
                + (double)person.direction * comic.box_width / 100.0;

    y_line[0] = person.head_y - comic.box_height / 50.0;
    y_line[2] = text_y + comic.box_height / 50.0;
    y_line[1] = ((y_line[2] + y_line[0]) / 2.0) + comic.box_height / 100.0;

    for (int i = 0; i < 3; ++i){
        std::cout << x_line[i] << '\t' << y_line[i] << '\n';
    }
    
    draw_array(x_line, y_line, panel_num, comic);

    // Draw text
    cairo_set_font_size (comic.panel_ctx[0], 100.0);
    cairo_set_source_rgb (comic.panel_ctx[0], 1.0, 1.0, 1.0);

    // Determining where to move to
    cairo_text_extents_t textbox;
    cairo_text_extents(comic.panel_ctx[panel_num], text.c_str(), &textbox);
    cairo_move_to (comic.panel_ctx[0], text_x - textbox.width / 2.0, text_y);
    cairo_show_text (comic.panel_ctx[0], text.c_str());

    cairo_stroke(comic.panel_ctx[panel_num]);

}

