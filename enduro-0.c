#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <math.h>

#define NUMCARS 10

const float FPS = 100;  
const int SCREEN_W = 960;
const int SCREEN_H = 540;


int SKY_H;

float CAR_W = 100;
float CAR_H = 20;
float WHEEL_W = 30;
float WHEEL_H = 40;

float TRACK_TOP_W;
float TRACK_BOTTOM_W;

float TRACK_TOP_LEFT_X;
float TRACK_TOP_LEFT_Y;
float TRACK_BOTTOM_LEFT_X;
float TRACK_BOTTOM_LEFT_Y;

float TRACK_TOP_RIGHT_X;
float TRACK_TOP_RIGHT_Y;
float TRACK_BOTTOM_RIGHT_X;
float TRACK_BOTTOM_RIGHT_Y;

float THETA;

/*
const float FPS = 100;  
const int SCREEN_W = 640*1.5;
const int SCREEN_H = 540;

int SKY_H = SCREEN_H/4;

const float CAR_W = 100;
const float CAR_H = 50;

const int TRACK_TOP_W = SCREEN_W/10;
const int TRACK_BOTTOM_W = SCREEN_W/5;

const int TRACK_TOP_LEFT_X = SCREEN_W/2 - TRACK_TOP_W/2;
const int TRACK_TOP_LEFT_Y = SKY_H;
const int TRACK_BOTTOM_LEFT_X = SCREEN_W/2 - TRACK_BOTTOM_W/2;
const int TRACK_BOTTOM_LEFT_Y = SCREEN_H;
*/

int score = 0;

typedef struct Car {
	float x,y; //x center, y center
	float xoffset; //distancia do x para a margem esquerda da pista (ajuda em alguns calculos)
	int accel, breaking, right, left;
	int status;
	int id;
	int delay;
	ALLEGRO_COLOR cor;
} Car;


void init_global_vars() {
	
	SKY_H = SCREEN_H/4;

	TRACK_TOP_W = SCREEN_W/100;
	TRACK_BOTTOM_W = SCREEN_W/1.2;
	
	TRACK_TOP_LEFT_X = SCREEN_W/2 - TRACK_TOP_W/2;
	TRACK_TOP_LEFT_Y = SKY_H;
	TRACK_BOTTOM_LEFT_X = SCREEN_W/2 - TRACK_BOTTOM_W/2;
	TRACK_BOTTOM_LEFT_Y = SCREEN_H;
	
	TRACK_TOP_RIGHT_X = SCREEN_W/2 + TRACK_TOP_W/2;
	TRACK_TOP_RIGHT_Y = SKY_H;
	TRACK_BOTTOM_RIGHT_X = SCREEN_W/2 + TRACK_BOTTOM_W/2;
	TRACK_BOTTOM_RIGHT_Y = SCREEN_H;

	
	
	//track angle
	THETA = atan((TRACK_BOTTOM_LEFT_Y - TRACK_TOP_LEFT_Y)/(TRACK_TOP_LEFT_X - TRACK_BOTTOM_LEFT_X));
	
	
}


void draw_scenario(ALLEGRO_DISPLAY *display) {
	
	//grass
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(55,171,38); 
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);   
	
	//sky
	al_draw_filled_rectangle(0, 0, SCREEN_W, SKY_H, al_map_rgb(0,255,247));
   
   //desenha a pista:
   al_draw_line(TRACK_TOP_LEFT_X, TRACK_TOP_LEFT_Y, TRACK_BOTTOM_LEFT_X, TRACK_BOTTOM_LEFT_Y, al_map_rgb(255,255,255), 10); 
   al_draw_line(TRACK_TOP_RIGHT_X, TRACK_TOP_RIGHT_Y, TRACK_BOTTOM_RIGHT_X, TRACK_BOTTOM_RIGHT_Y, al_map_rgb(255,255,255), 10);    
   
  
}



 
int main(int argc, char **argv){
	
	//srand(time(NULL));
	srand(2);
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	
	init_global_vars();

	
   
	//----------------------- rotinas de inicializacao ---------------------------------------
   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }
   
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }
 
   timer = al_create_timer(1.0 / FPS);
   if(!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }
 
   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }


	//inicializa o modulo allegro que carrega as fontes
    al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
    al_init_ttf_addon();


	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   	

 

   event_queue = al_create_event_queue();
   if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }
   
   al_install_keyboard();
   
	
	//registra na fila de eventos que eu quero identificar quando a tela foi alterada
   al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1
   al_register_event_source(event_queue, al_get_timer_event_source(timer));
   
   al_register_event_source(event_queue, al_get_keyboard_event_source());   
   
  

	//reinicializa a tela
   al_flip_display();
	//inicia o temporizador
   al_start_timer(timer);



   
   int playing = 1;
	//enquanto playing for verdadeiro, faca:
   while(playing) {
      ALLEGRO_EVENT ev;
	  //espera por um evento e o armazena na variavel de evento ev
      al_wait_for_event(event_queue, &ev);
	  
	if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
		//verifica qual tecla foi
		switch(ev.keyboard.keycode) {
			//se a tecla for o W
			case ALLEGRO_KEY_W:
				
			break;
			//se a tecla for o S
			case ALLEGRO_KEY_S:
				
			break;

			case ALLEGRO_KEY_A:
			
			break;

			case ALLEGRO_KEY_D:
				
			break;

			case ALLEGRO_KEY_ESCAPE:
				playing = 0;
			break;
			}

		}	 
	else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
		//verifica qual tecla foi
		switch(ev.keyboard.keycode) {
			case ALLEGRO_KEY_W:
			
			break;
			//se a tecla for o S
			case ALLEGRO_KEY_S:
				
			break;

			case ALLEGRO_KEY_A:
				
			break;

			case ALLEGRO_KEY_D:
				
			break;
			}

		}
	
	//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
    else if(ev.type == ALLEGRO_EVENT_TIMER) {
		
		draw_scenario(display);
	
		//reinicializo a tela
		 al_flip_display();
		 

    }
	//se o tipo de evento for o fechamento da tela (clique no x da janela)
	else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
		playing = 0;
	}
	
  } //fim do while
     
		
	al_rest(1);

 
	
	//reinicializa a tela
	al_flip_display();	
    al_rest(2);	
	
  
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);
   
 
   return 0;
}