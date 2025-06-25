#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

//dimensoes da tela
const int SCREEN_W = 960;
const int SCREEN_H = 540;

//dimensao do solo (terra)
const int EARTH_H = 60;

//void que desenha o fundo e que desenha o solo
void draw_scenario(){
	al_clear_to_color(al_map_rgb(0, 0, 30));
	//formato, tamanhos e cor do solo em relacao as dimensoes da tela
	al_draw_filled_pieslice(SCREEN_W / 2, SCREEN_H + 1030,  1100, ALLEGRO_PI,
	ALLEGRO_PI, al_map_rgb(0, 128, 255));
}

//demensoes da nave
const int NAVE_W = 100;
const int NAVE_H = 50;

//dimensoes dos aliens
const int ALIEN_W = 60;
const int ALIEN_H = 30;

//constante de frames por segundo (frames do jogo correndo por segundo na tela, ou seja, em que velocidade os eventos na tela se atualizam)
const float FPS = 60; 

//constante que armazena o arquivo .txt de recorde
const char *RECORD_FILE = "recorde.txt";

//define de maximo de tiros da nave
#define MAX_TIRO 50

//struct da nave, contendo posicoes (x e y), movimentos e cor
typedef struct nave {
	float x;
	float vel;
	int dir, esq;
	ALLEGRO_COLOR cor;
} Nave;

//struct do alien, contendo posicoes (x e y), velocidade y, estado de vida e cor
typedef struct Alien {
	float x, y;
	float y_vel;
	int vivo;
	ALLEGRO_COLOR cor;
} Alien;

//struct do tiro, contendo posicoes (x e y), velocidade y, estado de atividade e cor
typedef struct Tiro {
	float x, y;
	float y_vel;
	int ativo;
	ALLEGRO_COLOR cor;
} Tiro;

////////////////////////////////////////////////////////////////////////////////// NAVE

//void que inicia a nave, com ponteiros para a struct que apontam para sua posicao inicial, velocidade padrao, valor base para dir. e esq. e cor
void initNave (Nave *nave){
	nave->x = SCREEN_W / 2;
	nave->vel = 7;
	nave->dir = 0;
	nave->esq = 0;
	nave->cor = al_map_rgb(204, 0, 0);
}

//void que desenha a nave, com seu formato triangular e posicoes baseadas em dimensoes pre-definidas da nave e a sua cor
void draw_nave (Nave nave){
	float y_base = SCREEN_H - 0.8*EARTH_H;
	al_draw_filled_triangle(nave.x, y_base - NAVE_H/1.2, 
							nave.x - NAVE_W/2, y_base,
   							nave.x + NAVE_W/2, y_base, 
							nave.cor);
}

//void que atualiza o movimento da nave para dir. ou esq. por ponteiros para a struct, que previnem que a nave saia da tela pela esq. ou dir.
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

//inicia a posicao inicial de cada alien individualmente, com ponteiros para a struct, que definem a posicao exata e cor
void initAlien (Alien *alien, int i, int j) {
	alien->x = 10 + j*(ALIEN_W + 80);
	alien->y = 10 + i*(ALIEN_H + 20);
	alien->y_vel = ALIEN_H;
	alien->vivo = 1;
	alien->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
}

//inicia posicao inicial de cada alien individualmente, porem dentro da matriz, preenchendo-a 
void initAliens_matriz(Alien aliens[5][5]){
	for (int i=0; i<5; i++){
		for (int j=0; j<5; j++){
			initAlien(&aliens[i][j], i, j);
		}
	}
}

//void que desenha o formato dos aliens baseado em dimensoes pre-definidas
void draw_alien(Alien alien){
	al_draw_filled_rectangle(alien.x, alien.y, 
							 alien.x + ALIEN_W, alien.y + ALIEN_H,
                             alien.cor);
}

//velocidade x padrao definida para todos os aliens
float aliens_x_vel = 3;

//atualiza a posicao e a vida do alien. Se ele estiver vivo e chegar na borda, desce 1 posicao (que equivale ao y de 1 alien + um espacamento) e anda pro lado oposto ate chegar no solo ou morrer
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

//desenha a matriz inteira dos aliens, para a transicao final para a tela game over
void draw_aliens(Alien aliens[5][5]) {
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			if (aliens[i][j].vivo)
				draw_alien(aliens[i][j]);
}

////////////////////////////////////////////////////////////////////////////////// TIROS

//variavel que estabelece que o jogador pode atirar
int podeAtirar = 1;

//void que inicia os tiros estabelecendo a atividade, a posicao e a velocidade. Inicializa 0 tiros ativos, esperando que um tiro seja disparado
void initTiros(Tiro tiros[]) {
	for (int i=0; i < MAX_TIRO; i++){
		tiros[i].ativo = 0;
		tiros[i].x = 0;
		tiros[i].y = 0;
		tiros[i].y_vel = -10;
	}
}

//void que atualiza os tiros e, que se ocuparem a mesma posicao de um alien qualquer (acertando-o), o alien especifico e eliminado
void update_tiros(Tiro tiros[], Alien aliens[5][5]) {
	for (int i=0; i<MAX_TIRO; i++){
		//se o tiro for "ativado", o vetor do tiro (tiro em si) equivale a sua posicao inicial + velocidade para cima
		if (tiros[i].ativo){
			tiros[i].y += tiros[i].y_vel;

			//para a matriz de aliens, se um alien dentro da matriz estiver vivo e for acertado por um tiro qualquer, o tiro deixa de ser ativo e o alien deixa de estar vivo. 
			//o contador de aliens mortos aumenta e o "liberador de tiros" volta a ser 1, permitindo outro tiro
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
			//se o tiro sair da tela, o "liberador de tiros" volta a ser 1, permitindo outro tiro
			if (tiros[i].y < 0){
				tiros[i].ativo = 0;
				podeAtirar = 1;
			}
		}
	}
}

//void que desenha os tiros, apenas se o tiro estiver ativo (jogador atirou)(e isso somente se estiver dentro do maximo de tiros)
void draw_tiros(Tiro tiros[]) {
	for (int i=0; i < MAX_TIRO; i++){
		if (tiros[i].ativo){
			al_draw_filled_circle(tiros[i].x, tiros[i].y, 5, al_map_rgb(255, 255, 255));
		}
	}
}

//void que estabelece o tiro em si. Ele estabelece as dimensoes do tiro e sua velocidade. 
//Se nao houver um tiro ativo (jogador nao esta atirando ou um tiro anterior ainda esta na tela e nao destruiu um alien) o jogador volta a poder atirar
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

//percorre a matriz de aliens e verifica se tem algum vivo. Caso nao tenha, retorna 1
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

//variavel para estabelecer o inicio do jogo como 0s decorridos
int tempo_jogo = 0;

int main(){

	//rotinas do alegro que inicializam o display, a fila de eventos e o timer como NULL, para que o jogo realmente se inicie do 0
    ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	//rotina que inicializa o Alegro
	if(!al_init()) {
   		fprintf(stderr, "failed to initialize allegro!\n");
    	return -1;
	}

	//adicoes de rotina do allegro
	al_init_font_addon();
	al_init_ttf_addon();

	//adicao de uma fonte arial ao alegro, para escrever coisas como "game over"
	ALLEGRO_FONT *fonte = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 16, 0);
	if (!fonte) {
    	fprintf(stderr, "Falha ao carregar fonte.\n");
    	return -1;
	}

	//comeca um tempo aleatorio como NULL
	srand(time(NULL));

	//inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }	

    //cria uma tela (display) com dimensoes de SCREEN_W, SCREEN_H pixels
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

	//variavel que estabelece, inicialmente, o recorde como 0
	int recorde = 0;
	//abertura do arquivo de recorde
	FILE *arq = fopen(RECORD_FILE, "r");
	if (arq){
		fscanf(arq, "%d", &recorde);
		fclose(arq);
	}

	//variavel que estabelece o valor de "playing", para controlarmos se o jogo esta sendo jogado ou nao
	int playing = 1;
	//loop principal para rodar o jogo (enquanto "playing" for verdadeiro (1), ou seja, enquanto o jogador estiver jogando, faça as coisas)
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

			//atualiza o estado de todos os aliens de uma vez (a matriz toda)
			update_aliens(aliens); 

			//relogio de tempo de jogo na tela
			if(al_get_timer_count(timer) % (int)FPS == 0) {
    			tempo_jogo++;
			}
			//escrita do relogio
			char tempo_texto[64];
			sprintf(tempo_texto, "Tempo: %ds", tempo_jogo);
			al_draw_text(fonte, al_map_rgb(255,255,255), SCREEN_W - 10, SCREEN_H - 25, ALLEGRO_ALIGN_RIGHT, tempo_texto);

			//variavel verificadora de aliens que colidem no solo, que começa como 0 por padrao
			int alien_no_chao = 0;
			//percorrendo a matriz de aliens, se for encontrado um alien na posicao i,j que esta vivo, desenhe-o. 
			//Se o alien dessa posicao estiver colidindo com o solo (chama a func. colisao_alien_solo) e estabelece alien_no_chao como 1
			for (int i=0; i<5; i++){
				for (int j=0; j<5; j++){
					if (aliens[i][j].vivo){
						draw_alien(aliens[i][j]);
						if (colisao_alien_solo(aliens[i][j])){ 
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

			//se alien no chao for verdadeiro (1), entao quer dizer que um alien tocou no solo, entao trocamos a tela (display) para a tela de game over
			//faz-se um fade-out para isso e redesenhamos os elementos da tela no momento da derrota (nave, aliens tocando o solo, etc) e trocamos a tela novamente
			//playing passa a valer 0, ou seja, o "while" do playing nao acontece pois agora o jogo acabou
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

			//se caso todos_os_aliens_mortos for verdadeiro para "aliens", ou seja, toda a matriz de aliens foi eliminada, iniciamos novamente a matriz de aliens para que o jogo continue
			if (todos_os_aliens_mortos(aliens)){
				initAliens_matriz(aliens);
			}

			//atualiza a tela (quando houver algo para mostrar)
			al_flip_display();
			
			//conta e imprime na tela os segundos passados desde o inicio do jogo
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

			//switch case do teclado para verificar se o jogador pressionou certas teclas para tornar certos dados como verdadeiros (1), como "nave.esq", que anda a nave pra esquerda
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

		//se o tipo de evento for um pressionar de uma tecla (dessa vez, soltando a tecla)
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);

			//o switch agora e pra cancelar certos dados, ja que a tecla deixou de ser pressionada
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

	//if para verificar se o contador de aliens eliminados e maior que o recorde. Se for, e escrito no arquivo "recorde" um novo recorde
	if (aliens_eliminados > recorde){
		printf("\nNovo recorde! %d aliens eliminados.\n", aliens_eliminados);
		FILE *arq = fopen(RECORD_FILE, "w");
		if (arq){
			fprintf(arq, "%d", aliens_eliminados);
			fclose(arq);
			recorde = aliens_eliminados;
		}
		//caso o recorde nao tenha sido batido, apenas imprima a tela de game over e as outras informacoes
		else {
			printf("\nGame Over. Aliens eliminados: %d\nRecorde Atual: %d\n", aliens_eliminados, recorde);
		}
	}

	//variavel que inicia por padrao game over como 1
	int game_over = 1;

	//chars e sprintfs de cada mensagem da tela de game over
	char msg1[64];
	char msg2[64];
	char msg3[64];
	char msg4[64];

	sprintf(msg1, "Game Over!");
	sprintf(msg2, "Aliens eliminados: %d", aliens_eliminados);
	sprintf(msg3, "Recorde Atual: %d", recorde);
	sprintf(msg4, "Pressione qualquer tecla para sair");

	//enquanto game over for verdadeiro, faca as seguintes acoes: se o "x" de fechar for clicado ou se qualquer tecla for pressionada, termine o evento (feche o jogo)
	while (game_over){
		ALLEGRO_EVENT ev;
    	al_wait_for_event(event_queue, &ev);

    	if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
        	game_over = 0;
    	}
    	else if (ev.type == ALLEGRO_EVENT_KEY_DOWN){
        	game_over = 0;
    	}

		//dimensoes dos textos na tela e game over
    	al_clear_to_color(al_map_rgb(0, 0, 0));

		al_draw_text(fonte, al_map_rgb(255, 0, 0), SCREEN_W / 2, SCREEN_H / 2 - 20, ALLEGRO_ALIGN_CENTRE, msg1);
    	al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 20, ALLEGRO_ALIGN_CENTRE, msg2);
		al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2 + 40, ALLEGRO_ALIGN_CENTRE, msg3);
		al_draw_text(fonte, al_map_rgb(200, 200, 200), SCREEN_W / 2, SCREEN_H / 2 + 80, ALLEGRO_ALIGN_CENTRE, msg4);

    	al_flip_display();
	}

	//destrua, no final, a fonta, o display e a fila de eventos, para que da proxima vez que o jogo for iniciado, ele seja iniciado realmente do 0 (a unica coisa salva e o arquivo de recordes)
	al_destroy_font(fonte);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

    return 0;
}