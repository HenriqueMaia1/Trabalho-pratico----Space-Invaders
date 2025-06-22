#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

const int SCREEN_W = 960;
const int SCREEN_H = 540;

const int EARTH_H = 60;
void draw_scenario(){
	al_clear_to_color(al_map_rgb(0, 0, 30));
	al_draw_filled_pieslice(SCREEN_W / 2, SCREEN_H + 1030,  1100, ALLEGRO_PI,
	ALLEGRO_PI, al_map_rgb(0, 128, 255));
}

const int NAVE_W = 100;
const int NAVE_H = 50;

const int ALIEN_W = 60;
const int ALIEN_H = 30;

const float FPS = 100; 

const char *RECORD_FILE = "recorde.txt";

#define MAX_TIRO 50

typedef struct nave {
	float x;
	float vel;
	int dir, esq;
	ALLEGRO_COLOR cor;
} Nave;

typedef struct Alien {
	float x, y;
	float y_vel;
	int vivo;
	ALLEGRO_COLOR cor;
} Alien;

typedef struct Tiro {
	float x, y;
	float y_vel;
	int ativo;
	ALLEGRO_COLOR cor;
} Tiro;

////////////////////////////////////////////////////////////////////////////////// NAVE

void initNave (Nave *nave){
	nave->x = SCREEN_W / 2;
	nave->vel = 7;
	nave->dir = 0;
	nave->esq = 0;
	nave->cor = al_map_rgb(204, 0, 0);
}

void draw_nave (Nave nave){
	float y_base = SCREEN_H - 0.8*EARTH_H;
	al_draw_filled_triangle(nave.x, y_base - NAVE_H/1.2, 
							nave.x - NAVE_W/2, y_base,
   							nave.x + NAVE_W/2, y_base, 
							nave.cor);
}

void update_nave (Nave *nave){
	if(nave->dir && nave->x + nave->vel <= SCREEN_W){
		nave->x += nave->vel;
	}
	if(nave->esq && nave->x - nave->vel >= 0){
		nave->x -= nave->vel;
	}
}

////////////////////////////////////////////////////////////////////////////////// ALIENS

int aliens_eliminados = 0;

//inicia a posicao inicial de cada alien individualmente
void initAlien (Alien *alien, int i, int j) {
	alien->x = 10 + j*(ALIEN_W + 80);
	alien->y = 10 + i*(ALIEN_H + 20);
	alien->y_vel = ALIEN_H;
	alien->vivo = 1;
	alien->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
}

//inicia posicao inicial de cada alien individualmente, porem dentro da matriz
void initAliens_matriz(Alien aliens[5][5]){
	for (int i=0; i<5; i++){
		for (int j=0; j<5; j++){
			initAlien(&aliens[i][j], i, j);
		}
	}
}

void draw_alien(Alien alien){
	al_draw_filled_rectangle(alien.x, alien.y, 
							 alien.x + ALIEN_W, alien.y + ALIEN_H,
                             alien.cor);
}

float aliens_x_vel = 3;

//atualiza a posicao e a vida do alien. Se ele estiver vivo e chegar na borda, desce 1 posicao e anda pro lado oposto ate chegar no solo ou morrer
void update_aliens(Alien aliens[5][5]){
	int inverter = 0;
	for (int i=0; i<5; i++){
		for (int j=0; j<5; j++){
			if (aliens[i][j].vivo){
				if ((aliens[i][j].x + ALIEN_W + aliens_x_vel > SCREEN_W) || 
					(aliens[i][j].x + aliens_x_vel < 0)) {
					inverter = 1;
					break;
				}
			}
		}
	}

	if (inverter) {
		aliens_x_vel *= -1;
		for (int i=0; i<5; i++){
			for (int j=0; j<5; j++){
				aliens[i][j].y += aliens[i][j].y_vel;
			}
		}
	}

	for (int i=0; i<5; i++){
		for (int j=0; j<5; j++){
			aliens[i][j].x += aliens_x_vel;
		}
	}
}

//verifica colisao de alien com o solo
int colisao_alien_solo (Alien alien){

	float y_base_nave = SCREEN_H - 0.8 * EARTH_H;
	float topo_nave = y_base_nave - NAVE_H / 1.2;

	if (alien.y + ALIEN_H > SCREEN_H - EARTH_H || alien.y + ALIEN_H >= topo_nave)
		return 1;
	return 0;
}

//desenha a matriz inteira dos aliens, para a transicao final de game over
void draw_aliens(Alien aliens[5][5]) {
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			if (aliens[i][j].vivo)
				draw_alien(aliens[i][j]);
}

////////////////////////////////////////////////////////////////////////////////// TIROS

int podeAtirar = 1;

void initTiros(Tiro tiros[]) {
	for (int i=0; i < MAX_TIRO; i++){
		tiros[i].ativo = 0;
		tiros[i].x = 0;
		tiros[i].y = 0;
		tiros[i].y_vel = -10;
	}
}

void update_tiros(Tiro tiros[], Alien aliens[5][5]) {
	for (int i=0; i<MAX_TIRO; i++){
		if (tiros[i].ativo){
			tiros[i].y += tiros[i].y_vel;

			for (int linha=0; linha < 5; linha++){
				for (int col=0; col < 5; col++){
					if (aliens[linha][col].vivo &&
						tiros[i].x >= aliens[linha][col].x && 
						tiros[i].x <= aliens[linha][col].x + ALIEN_W &&
						tiros[i].y >= aliens[linha][col].y && 
						tiros[i].y <= aliens[linha][col].y + ALIEN_H){
							tiros[i].ativo = 0;
							aliens[linha][col].vivo = 0;
							aliens_eliminados++;
							podeAtirar = 1;
					}
				}
			}
			if (tiros[i].y < 0){
				tiros[i].ativo = 0;
				podeAtirar = 1;
			}
		}
	}
}

void draw_tiros(Tiro tiros[]) {
	for (int i=0; i < MAX_TIRO; i++){
		if (tiros[i].ativo){
			al_draw_filled_circle(tiros[i].x, tiros[i].y, 5, al_map_rgb(255, 255, 255));
		}
	}
}

void atirar(Tiro tiros[], Nave nave){
	float y_base = SCREEN_H - 0.8*EARTH_H;
	for (int i=0; i < MAX_TIRO; i++){
		if (!tiros[i].ativo){
			tiros[i].ativo = 1;
			tiros[i].x = nave.x;
			tiros[i].y = y_base - NAVE_H/1.2;
			tiros[i].y_vel = -10;
			break;
		}
	}
}

int todos_os_aliens_mortos (Alien aliens[5][5]){
	for (int i=0; i<5; i++){
		for (int j=0; j<5; j++){
			if (aliens[i][j].vivo){
				return 0;
			}
		}
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////// MAIN

int main(){

    ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	//inicializa o Alegro
	if(!al_init()) {
   		fprintf(stderr, "failed to initialize allegro!\n");
    	return -1;
	}

	al_init_font_addon();
	al_init_ttf_addon();

	ALLEGRO_FONT *fonte = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 16, 0);
	if (!fonte) {
    	fprintf(stderr, "Falha ao carregar fonte.\n");
    	return -1;
	}

	srand(time(NULL));

	//inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	

    //cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
	
	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	//instala o teclado
	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	
	//instala o mouse
	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize mouse!\n");
		return -1;
	}

	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(event_queue, al_get_mouse_event_source());  	
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	//inits de nave, aliens e tiros
	Nave nave;
	initNave(&nave);

	Alien aliens[5][5];
	initAliens_matriz(aliens);

	Tiro tiros[MAX_TIRO];
	initTiros(tiros);

	//inicia o temporizador
	al_start_timer(timer);

	int recorde = 0;
	FILE *arq = fopen(RECORD_FILE, "r");
	if (arq){
		fscanf(arq, "%d", &recorde);
		fclose(arq);
	}

	int playing = 1;
	//loop principal para rodar o jogo
	while(playing){ 

		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if(ev.type == ALLEGRO_EVENT_TIMER) {

			//desenha o cenario de fundo quando o timer do jogo e iniciado 
			draw_scenario();
			
			//atualiza o estado da nave (andando pra esquerda, direita, ou parada) e desenha a nave (vem depois do cenario pra ficar por cima)
			update_nave(&nave);
			draw_nave(nave);

			update_aliens(aliens); //atualiza o estado de todos os aliens de uma vez

			int alien_no_chao = 0;
			for (int i=0; i<5; i++){
				for (int j=0; j<5; j++){
					if (aliens[i][j].vivo){
						draw_alien(aliens[i][j]);
						if (colisao_alien_solo(aliens[i][j])){ //verifica se teve colisao do alien com o solo
						alien_no_chao = 1;
						}
					}
				}
			}

			//atualiza o estado dos tiros
			update_tiros(tiros, aliens);
			draw_tiros(tiros);

			//texto do recorde
			char texto[64];
			sprintf(texto, "Aliens eliminados: %d", aliens_eliminados);
			al_draw_text(fonte, al_map_rgb(255,255,255), 10, SCREEN_H - 25, 0, texto);

			if (alien_no_chao){
				al_flip_display();
				//faz ou fade-out do jogo pra tela de game over
				for (float alpha = 0; alpha <= 1.0; alpha += 0.05) {
					// redesenha o fundo e a nave/aliens/etc 
    				draw_scenario(); 
    				draw_nave(nave);
    				draw_aliens(aliens); 
    				draw_tiros(tiros);

    				al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H, al_map_rgba_f(0, 0, 0, alpha));

    				al_flip_display();
    				al_rest(0.01); 
				}
				playing = 0;
			}

			if (todos_os_aliens_mortos(aliens)){
				initAliens_matriz(aliens);
			}

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			
			if(al_get_timer_count(timer)%(int)FPS == 0)
				printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer)/FPS));
		}

		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			playing = 0;
		}

		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			switch(ev.keyboard.keycode){

				case ALLEGRO_KEY_A:
					nave.esq = 1;
				break;

				case ALLEGRO_KEY_D:
					nave.dir = 1;
				break;

				case ALLEGRO_KEY_SPACE:
					if (podeAtirar){
						atirar(tiros, nave);
						podeAtirar = 0;
					}
				break;
			}
			
		}

		//se o tipo de evento for um pressionar de uma tecla
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			switch(ev.keyboard.keycode){

				case ALLEGRO_KEY_A:
					nave.esq = 0;
				break;

				case ALLEGRO_KEY_D:
					nave.dir = 0;
				break;
				
			}
			
		}

	}

	if (aliens_eliminados > recorde){
		printf("\nNovo recorde! %d aliens eliminados.\n", aliens_eliminados);
		FILE *arq = fopen(RECORD_FILE, "w");
		if (arq){
			fprintf(arq, "%d", aliens_eliminados);
			fclose(arq);
			recorde = aliens_eliminados;
		}
		else {
			printf("\nGame Over. Aliens eliminados: %d\nRecorde Atual: %d\n", aliens_eliminados, recorde);
		}
	}

	int game_over = 1;
	char msg1[64];
	char msg2[64];
	char msg3[64];
	char msg4[64];

	sprintf(msg1, "Game Over!");
	sprintf(msg2, "Aliens eliminados: %d", aliens_eliminados);
	sprintf(msg3, "Recorde Atual: %d", recorde);
	sprintf(msg4, "Pressione qualquer tecla para sair");

	while (game_over){
		ALLEGRO_EVENT ev;
    	al_wait_for_event(event_queue, &ev);

    	if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
        	game_over = 0;
    	}
    	else if (ev.type == ALLEGRO_EVENT_KEY_DOWN){
        	game_over = 0;
    	}

    	al_clear_to_color(al_map_rgb(0, 0, 0));

		al_draw_text(fonte, al_map_rgb(255, 0, 0), SCREEN_W / 2, SCREEN_H / 2 - 20, ALLEGRO_ALIGN_CENTRE, msg1);
    	al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 20, ALLEGRO_ALIGN_CENTRE, msg2);
		al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 40, ALLEGRO_ALIGN_CENTRE, msg3);
		al_draw_text(fonte, al_map_rgb(200, 200, 200), SCREEN_W / 2, SCREEN_H / 2 + 80, ALLEGRO_ALIGN_CENTRE, msg4);

    	al_flip_display();
	}


	al_destroy_font(fonte);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

    return 0;
}