#include "lvgl.h"
#include "SPI.h"
#include "math.h"
#include "stm32746g_discovery_lcd.h"
#include <cstdlib>

extern "C" {
#include "C:\Users\ulysse\Documents\projet_instrumentation\base746\minuteurmdpi.c"
} 


#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

#define LCD_FRAMEBUFFER ((uint32_t *)0xC0000000) // Adresse du framebuffer LCD



static lv_color_t buf_1[BUFFER_SIZE];
static lv_color_t buf_2[BUFFER_SIZE];
static lv_display_t *display;

//extern const lv_image_dsc_t minuteurmdpi;  // Utilisez lv_image_dsc_t et non LV_IMG_DECLARE

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void testLvgl()
{
  // Initialisations générales
  lv_obj_t * label;

  lv_obj_t * btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  lv_obj_t * btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Toggle");
  lv_obj_center(label);
}




#ifdef ARDUINO

#include "lvglDrivers.h"



#define NB_CASES 10

static lv_obj_t *wheel_container;
static lv_obj_t *arrow;
static int current_angle = 0;

// Déclaration du tableau pour stocker les points des lignes de la roue
static lv_point_t line_points[NB_CASES][2];




// Fonction pour créer la roue
void create_roue_de_la_fortune()
{
    // Conteneur circulaire
    lv_obj_t *wheel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wheel, 220, 220);
    lv_obj_center(wheel);
    lv_obj_set_style_radius(wheel, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(wheel, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_border_color(wheel, lv_color_black(), 0);
    lv_obj_set_style_border_width(wheel, 2, 0);

    // Centre du conteneur
    lv_coord_t cx = lv_obj_get_width(wheel) / 2;
    lv_coord_t cy = lv_obj_get_height(wheel) / 2;
    int rayon = cx;

    for (int i = 0; i < NB_CASES; i++)
    {
        float angle = i * (360.0f / NB_CASES) * 3.14159f / 180.0f;

        line_points[i][0].x = cx;
        line_points[i][0].y = cy;
        line_points[i][1].x = (lv_coord_t)(cx + rayon * cosf(angle));
        line_points[i][1].y = (lv_coord_t)(cy + rayon * sinf(angle));

        lv_obj_t *line = lv_line_create(wheel);
        lv_line_set_points(line, (const lv_point_precise_t *)line_points[i], 2);
        lv_obj_set_style_line_color(line, lv_color_black(), 0);
        lv_obj_set_style_line_width(line, 2, 0);
    }
}



// Fonction de rotation animée
void rotate_wheel(int target_angle)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, wheel_container);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_angle);
    lv_anim_set_values(&a, current_angle * 10, target_angle * 10);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    // Mettre à jour l’angle courant
    current_angle = target_angle % 360;

    lv_anim_start(&a);
}

static void button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        int random_steps = (rand() % NB_CASES + 1) * (360 / NB_CASES);
        int target_angle = current_angle + random_steps + 720; // Ajoute 2 tours pour l’effet
        rotate_wheel(target_angle);
    }
}

// Fonction pour créer le bouton
void create_spin_button()
{
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20); // Corrigé : légèrement au-dessus du bas
    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "SPIN");
    lv_obj_center(label);
}


void create_roue_de_la_fortune(); 

static void my_disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    for (int y = area->y1; y <= area->y2; y++) {
        uint32_t * dest = LCD_FRAMEBUFFER + y * SCREEN_WIDTH + area->x1;
        memcpy(dest, color_p, (area->x2 - area->x1 + 1) * 4); // 4 octets/pixel
        color_p += (area->x2 - area->x1 + 1) * 4;
    }
    lv_disp_flush_ready(disp);
}

void mySetup()
{
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS); // ARGB8888
    BSP_LCD_SelectLayer(1);
    BSP_LCD_DisplayOn();

    lv_init();

    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(display, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_disp_flush);
    
    // Appeler la fonction pour créer la roue
    create_roue_de_la_fortune();
    create_spin_button();

    // lv_obj_t * label = lv_label_create(lv_screen_active());
    // lv_label_set_text(label, "Hello World !");
    // lv_obj_center(label);
}

void loop()
{
}

void myTask(void *pvParameters)
{
  // Init
  TickType_t xLastWakeTime;
  // Lecture du nombre de ticks quand la tâche débute
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    // Loop

    // Endort la tâche pendant le temps restant par rapport au réveil,
    // ici 200ms, donc la tâche s'effectue toutes les 200ms
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // toutes les 200 ms
  }
}

#else

#include "lvgl.h"
#include "app_hal.h"
#include <cstdio>

int main(void)
{
  printf("LVGL Simulator\n");
  fflush(stdout);

  lv_init();
  hal_setup();

  testLvgl();

  hal_loop();
  return 0;
}

#endif

void create_roue_de_la_fortune2()
{
    lv_obj_t *roue = lv_img_create(lv_scr_act());  // Utilise lv_img_create
    lv_img_set_src(roue, &minuteurmdpi);              // Utilise lv_img_set_src
    lv_obj_center(roue);
}











































//----------------------------------------------------------------------------------------------------------------------



// #include "lvgl.h"
// #include "SPI.h"
// #include "math.h"
// #include "C:\Users\ulysse\Documents\projet_instrumentation\base746\roue_png.h" 





// #define SCREEN_WIDTH 480
// #define SCREEN_HEIGHT 272
// #define BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)



// static lv_color_t buf_1[BUFFER_SIZE];
// static lv_color_t buf_2[BUFFER_SIZE];
// static lv_display_t *display;


// // extern "C" {
// // #include "C:\Users\ulysse\Documents\projet_instrumentation\base746\roue_png.c"
// // }

// LV_IMG_DECLARE(roue_png);



// static void event_handler(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);

//     if(code == LV_EVENT_CLICKED) {
//         LV_LOG_USER("Clicked");
//     }
//     else if(code == LV_EVENT_VALUE_CHANGED) {
//         LV_LOG_USER("Toggled");
//     }
// }

// void testLvgl()
// {
//   // Initialisations générales
//   lv_obj_t * label;

//   lv_obj_t * btn1 = lv_button_create(lv_screen_active());
//   lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
//   lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

//   label = lv_label_create(btn1);
//   lv_label_set_text(label, "Button");
//   lv_obj_center(label);

//   lv_obj_t * btn2 = lv_button_create(lv_screen_active());
//   lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
//   lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
//   lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
//   lv_obj_set_height(btn2, LV_SIZE_CONTENT);

//   label = lv_label_create(btn2);
//   lv_label_set_text(label, "Toggle");
//   lv_obj_center(label);
// }

// #ifdef ARDUINO

// #include "lvglDrivers.h"

// // à décommenter pour tester la démo
// // #include "demos/lv_demos.h"

// void create_roue_de_la_fortune(); // Forward declaration

// void mySetup()
// {
//     // Initialiser LVGL
//     lv_init();
    
//     // Créer et configurer l'affichage
//     display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
//     lv_display_set_buffers(display, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    


//           // Conteneur pour centrer la roue
//     lv_obj_t *container = lv_obj_create(lv_scr_act());
//     lv_obj_center(container);
//     lv_obj_set_size(container, 240, 240);
//     lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_style_bg_color(container, lv_palette_main(LV_PALETTE_BLUE), 0);

//     lv_obj_t *roue_png = lv_img_create(container);
//     lv_img_set_src(roue_png, &roue); // Utilise ton image PNG (convertie en C-array par exemple avec lv_img_conv)
//     lv_obj_center(roue_png);



// }

// void loop()
// {
//   // Inactif (pour mise en veille du processeur)
// }

// void myTask(void *pvParameters)
// {
//   // Init
//   TickType_t xLastWakeTime;
//   // Lecture du nombre de ticks quand la tâche débute
//   xLastWakeTime = xTaskGetTickCount();
//   while (1)
//   {
//     // Loop

//     // Endort la tâche pendant le temps restant par rapport au réveil,
//     // ici 200ms, donc la tâche s'effectue toutes les 200ms
//     vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // toutes les 200 ms
//   }
// }

// #else

// #include "lvgl.h"
// #include "app_hal.h"
// #include <cstdio>

// int main(void)
// {
//   printf("LVGL Simulator\n");
//   fflush(stdout);

//   lv_init();
//   hal_setup();

//   testLvgl();

//   hal_loop();
//   return 0;
// }

// #endif


// //void create_wheel_of_fortune()

// void create_roue_de_la_fortune()
// {
//     // Conteneur pour centrer la roue
//     lv_obj_t *container = lv_obj_create(lv_scr_act());
//     lv_obj_center(container);
//     lv_obj_set_size(container, 240, 240);
//     lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_style_bg_color(container, lv_palette_main(LV_PALETTE_BLUE), 0);


//     // Créer un objet image pour la roue
//     lv_obj_t *roue = lv_img_create(container);
//     lv_img_set_src(roue, &roue_png); // Utilise ton image PNG (convertie en C-array par exemple avec lv_img_conv)
//     lv_obj_center(roue);


//     //sans le conteneur 
//     // lv_obj_t *img = lv_img_create(lv_scr_act());
//     // lv_img_set_src(img, &wheel_img);
//     // lv_obj_center(img);

//     // // Créer un triangle rouge pour indiquer la case gagnante
//     // // Pour faire un triangle, on utilise un canvas (simple et efficace)
//     // static lv_color_t arrow_buf[LV_CANVAS_BUF_SIZE_COLOR_FORMAT_RGB565(40, 40)];
//     // lv_obj_t *arrow_canvas = lv_canvas_create(container);
//     // lv_canvas_set_buffer(arrow_canvas, arrow_buf, 40, 40, LV_COLOR_FORMAT_RGB565);
//     // lv_canvas_fill_bg(arrow_canvas, lv_color_white(), LV_OPA_TRANSP); // Transparent

//     // // Points du triangle
//     // lv_point_t points[3] = {
//     //     {0, 0},   // Coin gauche
//     //     {40, 20}, // Pointe à droite
//     //     {0, 40}   // Coin gauche bas
//     // };

//     // // Style du triangle
//     // lv_draw_rect_dsc_t arrow_dsc;
//     // lv_draw_rect_dsc_init(&arrow_dsc);
//     // arrow_dsc.bg_color = lv_color_red();
//     // arrow_dsc.bg_opa = LV_OPA_COVER;

//     // // Zone englobante
//     // lv_area_t area;
//     // area.x1 = 0;
//     // area.y1 = 0;
//     // area.x2 = 39;
//     // area.y2 = 39;

//     // // Remplissage pixel par pixel pour le triangle
//     // for (int y = area.y1; y <= area.y2; y++)
//     // {
//     //     for (int x = area.x1; x <= area.x2; x++)
//     //     {
//     //         // Barycentric test simplifié
//     //         float alpha = (float)(20 * (y - 0) - 20 * (0 - 40)) / (float)(20 * (40 - 0) - 20 * (0 - 40));
//     //         float beta = (float)(0 * (y - 0) - 40 * (x - 0)) / (float)(20 * (40 - 0) - 20 * (0 - 40));
//     //         float gamma = 1.0f - alpha - beta;

//     //         if (alpha >= 0 && beta >= 0 && gamma >= 0)
//     //         {
//     //             lv_canvas_set_px(arrow_canvas, x, y, lv_color_red(), LV_OPA_COVER);
//     //         }
//     //     }
//     // }

//     // // Positionner la flèche à droite de la roue
//     // lv_obj_align(arrow_canvas, LV_ALIGN_RIGHT_MID, 20, 0);
// }