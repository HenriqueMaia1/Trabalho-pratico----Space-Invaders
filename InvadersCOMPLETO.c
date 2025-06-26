#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

// DEFINES ou CONSTANTES

//fps
const float FPS = 60; 
//dimensoes da tela
const int SCREEN_W = 960;
const int SCREEN_H = 540;
//dimensao do solo (terra)
const int EARTH_H = 60;
//dimensoes da nave
const int NAVE_W = 100;
const int NAVE_H = 50;
//dimensoes de alien
const int ALIEN_W = 60;
const int ALIEN_H = 30;
//maximo de tiros
#define MAX_TIRO 50
//arquivo de recorde
const char *RECORD_FILE = "recorde.txt";
//maximo de powerups
#define MAX_POWERUPS 3
//tempo de powerup
#define TEMPO_POWERUP 10
//total de fases
#define TOTAL_FASES 10

// DEFINES DE ESTADOS DE JOGO
#define MENU 0
#define ESCOLHENDO_NAVE 1
#define ESCOLHENDO_DIFICULDADE 2
#define JOGANDO 3
#define GAME_OVER 4

// DEFINES DE DIFICULDADE
#define FACIL 1
#define MEDIO 2
#define DIFICIL 3

// DEFINES DE TIPOS DE NAVE
#define NAVE_A 0
#define NAVE_B 1
#define NAVE_C 2

// DEFINES DE TIPOS DE POWERUPS
#define TIRO_RAPIDO 0
#define TIRO_DUPLO 1

// STRUCTS

//struct nave
typedef struct {
    float x;
    float vel;
    int dir, esq;
    ALLEGRO_COLOR cor;
    int tipo; // NAVE_A, NAVE_B, NAVE_C
} Nave;

//struct alien
typedef struct {
    float x, y;
    float y_vel;
    int vivo;
    ALLEGRO_COLOR cor;
} Alien;

//struct tiros
typedef struct {
    float x, y;
    float y_vel;
    int ativo;
    ALLEGRO_COLOR cor;
} Tiro;


//struct powerup
typedef struct {
    float x, y;
    int ativo;
    int tipo; // TIRO_RAPIDO ou TIRO_DUPLO
} PowerUp;

// VARIÁVEIS GLOBAIS
int missoes_completas = 0;
int aliens_eliminados = 0;
int estado_jogo = MENU;
int dificuldade = FACIL;
int nave_selecionada = NAVE_A;
int tipo_powerup_ativo = TIRO_RAPIDO;
int podeAtirar = 1;
int fase_atual = 1;
int recorde = 0;
int tempo_missao = 30;
int aliens_objetivo = 10;

//matriz 5x5 de alies da struct Alien
Alien aliens[5][5];
//tiros da struct Tiro, com limite de max tiro
Tiro tiros[MAX_TIRO];
//powerups da struct powerup, com limite de max powerups
PowerUp powerups[MAX_POWERUPS];
//ints padroes de inicio
int tempo_jogo_segundos = 0;
int powerup_ativo = 0;
int tipo_powerup_ativo;
int tempo_powerup_ativo = 0;

// Cores por fase (RGB 0-255)
int cores_cenario[TOTAL_FASES][3] = {
    {0, 0, 30}, {10, 0, 40}, {20, 10, 50}, {30, 20, 60},
    {40, 30, 70}, {50, 40, 80}, {60, 50, 90}, {70, 60, 100},
    {80, 70, 110}, {90, 80, 120}
};

float aliens_x_vel = 3; // velocidade alien padrão, mudará pela dificuldade

/////////////////// FUNÇÕES BASE DO JOGO ///////////////////

////////////////////////////////////////////////////////////////////////////////// NAVE

//void que inicia a nave, com ponteiros para a struct que apontam para sua posicao inicial, velocidade padrao, valor base para dir. e esq. e cor
void initNave(Nave *nave) {
    nave->x = SCREEN_W / 2;
    nave->vel = 7;
    nave->dir = 0;
    nave->esq = 0;
    nave->tipo = nave_selecionada;
    // Cor e velocidade da nave conforme o tipo
    switch (nave->tipo) {
        case NAVE_A: nave->cor = al_map_rgb(204, 0, 0); break; // vermelho
        case NAVE_B: nave->cor = al_map_rgb(0, 204, 0); break; // verde
        case NAVE_C: nave->cor = al_map_rgb(0, 0, 204); break; // azul
    }
}

//void que desenha a nave, com seu formato triangular e posicoes baseadas em dimensoes pre-definidas da nave e a sua cor
void draw_nave(Nave nave) {
    float y_base = SCREEN_H - 0.8*EARTH_H;
    al_draw_filled_triangle(nave.x, y_base - NAVE_H/1.2, 
                            nave.x - NAVE_W/2, y_base,
                            nave.x + NAVE_W/2, y_base, 
                            nave.cor);
}

//void que atualiza o movimento da nave para dir. ou esq. por ponteiros para a struct, que previnem que a nave saia da tela pela esq. ou dir.
void update_nave(Nave *nave) {
    if(nave->dir && nave->x + nave->vel <= SCREEN_W) {
        nave->x += nave->vel;
    }
    if(nave->esq && nave->x - nave->vel >= 0) {
        nave->x -= nave->vel;
    }
}

////////////////////////////////////////////////////////////////////////////////// ALIENS

//inicia a posicao inicial de cada alien individualmente na matriz, com ponteiros para a struct, que definem a posicao exata e cor
void initAlien(Alien *alien, int i, int j) {
    alien->x = 10 + j*(ALIEN_W + 80);
    alien->y = 10 + i*(ALIEN_H + 20);
    alien->y_vel = ALIEN_H;
    alien->vivo = 1;
    alien->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
}

//inicia posicao inicial de cada alien individualmente, porem dentro da matriz, preenchendo-a 
void initAliens_matriz(Alien aliens[5][5]) {
    for(int i=0; i<5; i++) {
        for(int j=0; j<5; j++) {
            initAlien(&aliens[i][j], i, j);
        }
    }
}

//void que desenha o formato dos aliens baseado em dimensoes pre-definidas
void draw_alien(Alien alien) {
    al_draw_filled_rectangle(alien.x, alien.y, 
                             alien.x + ALIEN_W, alien.y + ALIEN_H,
                             alien.cor);
}

//atualiza a posicao e a vida do alien. Se ele estiver vivo e chegar na borda, desce 1 posicao (que equivale ao y de 1 alien + um espacamento) e anda pro lado oposto ate chegar no solo ou morrer
void update_aliens(Alien aliens[5][5]) {
    int inverter = 0;
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            if (aliens[i][j].vivo) {
                if ((aliens[i][j].x + ALIEN_W + aliens_x_vel > SCREEN_W) || 
                    (aliens[i][j].x + aliens_x_vel < 0)) {
                    inverter = 1;
                    break;
                }
            }
        }
        if(inverter) break;
    }

    if (inverter) {
        aliens_x_vel *= -1;
        for (int i=0; i<5; i++) {
            for (int j=0; j<5; j++) {
                aliens[i][j].y += aliens[i][j].y_vel;
            }
        }
    }

    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            aliens[i][j].x += aliens_x_vel;
        }
    }
}

//verifica colisao de alien com o solo
int colisao_alien_solo(Alien alien) {
    float y_base_nave = SCREEN_H - 0.8 * EARTH_H;
    float topo_nave = y_base_nave - NAVE_H / 1.2;

    if (alien.y + ALIEN_H > SCREEN_H - EARTH_H || alien.y + ALIEN_H >= topo_nave)
        return 1;
    return 0;
}

//desenha a matriz inteira dos aliens, para a transicao final para a tela game over
void draw_aliens(Alien aliens[5][5]) {
    for (int i=0; i<5; i++)
        for (int j=0; j<5; j++)
            if (aliens[i][j].vivo)
                draw_alien(aliens[i][j]);
}

////////////////////////////////////////////////////////////////////////////////// TIROS

//void que inicia os tiros estabelecendo a atividade, a posicao e a velocidade. Inicializa 0 tiros ativos, esperando que um tiro seja disparado
void initTiros(Tiro tiros[]) {
    for (int i=0; i<MAX_TIRO; i++) {
        tiros[i].ativo = 0;
        tiros[i].x = 0;
        tiros[i].y = 0;
        tiros[i].y_vel = -10;
        tiros[i].cor = al_map_rgb(255, 255, 255);
    }
}

//void que atualiza os tiros e, que se ocuparem a mesma posicao de um alien qualquer (acertando-o), o alien especifico e eliminado
void update_tiros(Tiro tiros[], Alien aliens[5][5], Nave nave) {
    for (int i=0; i<MAX_TIRO; i++) {
        //se o tiro for "ativado", o vetor do tiro (tiro em si) equivale a sua posicao inicial + velocidade para cima
        if (tiros[i].ativo) {
            tiros[i].y += tiros[i].y_vel;

            //para a matriz de aliens, se um alien dentro da matriz estiver vivo e for acertado por um tiro qualquer, o tiro deixa de ser ativo e o alien deixa de estar vivo. 
			//o contador de aliens mortos aumenta e o "liberador de tiros" volta a ser 1, permitindo outro tiro
            for (int linha=0; linha < 5; linha++) {
                for (int col=0; col < 5; col++) {
                    if (aliens[linha][col].vivo &&
                        tiros[i].x >= aliens[linha][col].x && 
                        tiros[i].x <= aliens[linha][col].x + ALIEN_W &&
                        tiros[i].y >= aliens[linha][col].y && 
                        tiros[i].y <= aliens[linha][col].y + ALIEN_H) {
                            tiros[i].ativo = 0;
                            aliens[linha][col].vivo = 0;
                            aliens_eliminados++;

                            // Chance de spawnar powerup (5%) / o powerup e dropado de um alien morto
                            if ((rand() % 100) < 5) {
                                float px = aliens[linha][col].x + ALIEN_W / 2;
                                float py = aliens[linha][col].y + ALIEN_H;
                                for (int p = 0; p < MAX_POWERUPS; p++) {
                                    if (!powerups[p].ativo) {
                                        powerups[p].ativo = 1;
                                        powerups[p].x = px;
                                        powerups[p].y = py;
                                        if (rand() % 2 == 0) {
                                            powerups[p].tipo = TIRO_RAPIDO;
                                        } 
                                        else {
                                            powerups[p].tipo = TIRO_DUPLO;
                                        }
                                    break;
                                    }
                                }
                            }

                            podeAtirar = 1;
                            break;
                    }
                }
            }
            //se o tiro sair da tela, o "liberador de tiros" volta a ser 1, permitindo outro tiro
            if (tiros[i].y < 0) {
                tiros[i].ativo = 0;
                podeAtirar = 1;
            }
        }
    }
}

//void que desenha os tiros, apenas se o tiro estiver ativo (jogador atirou)(e isso somente se estiver dentro do maximo de tiros)
void draw_tiros(Tiro tiros[]) {
    for (int i=0; i<MAX_TIRO; i++) {
        if (tiros[i].ativo) {
            al_draw_filled_circle(tiros[i].x, tiros[i].y, 5, tiros[i].cor);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////// POWERUPS

void initPowerUps() {
    for (int i=0; i<MAX_POWERUPS; i++) {
        powerups[i].ativo = 0;
        powerups[i].x = 0;
        powerups[i].y = 0;
        powerups[i].tipo = TIRO_RAPIDO;
    }
}

void atualizar_powerups(Nave nave) {
    for (int i=0; i<MAX_POWERUPS; i++) {
        if (powerups[i].ativo) {
            powerups[i].y += 3;
            if (powerups[i].y >= SCREEN_H - EARTH_H) {
                powerups[i].ativo = 0;
            }
            // colisão powerup com nave (nos da o power up)
            if (powerups[i].y >= SCREEN_H - 0.8 * EARTH_H - NAVE_H / 1.2 &&
                powerups[i].x >= nave.x - NAVE_W / 2 &&
                powerups[i].x <= nave.x + NAVE_W / 2) {
                powerup_ativo = 1;
                tipo_powerup_ativo = powerups[i].tipo;
                tempo_powerup_ativo = TEMPO_POWERUP;
                powerups[i].ativo = 0;
            }
        }
    }
}

void desenhar_powerups() {
    for (int i=0; i<MAX_POWERUPS; i++) {
        if (powerups[i].ativo) {
            ALLEGRO_COLOR cor;
            if (powerups[i].tipo == TIRO_RAPIDO) {
                cor = al_map_rgb(255, 255, 0);
            } 
            else {
                cor = al_map_rgb(0, 255, 255);
            }           
            al_draw_filled_circle(powerups[i].x, powerups[i].y, 10, cor);
        }
    }
}

// TIROS conforme powerup e tipo de nave

//void que estabelece o tiro em si. Ele estabelece as dimensoes do tiro e sua velocidade. 
//Se nao houver um tiro ativo (jogador nao esta atirando ou um tiro anterior ainda esta na tela e nao destruiu um alien) o jogador volta a poder atirar
void atirar(Tiro tiros[], Nave nave) {
    float y_base = SCREEN_H - 0.8*EARTH_H;
    int disparos = 1;
    int espacamento = 10;
    int vel_tiro = -10;

    // Poder de tiro do powerup
    if (powerup_ativo) {
        if (tipo_powerup_ativo == TIRO_RAPIDO)
            vel_tiro = -14;
        else if (tipo_powerup_ativo == TIRO_DUPLO)
            disparos = 2;
    }

    // Tiros diferentes por tipo de nave (exemplo simples)
    if (nave.tipo == NAVE_B) disparos = 2;  // Nave B dispara 2 tiros sempre
    else if (nave.tipo == NAVE_C) vel_tiro = -12; // Nave C tiros um pouco mais lentos

    for (int d=0; d < disparos; d++) {
        for (int i=0; i<MAX_TIRO; i++) {
            if (!tiros[i].ativo) {
                tiros[i].ativo = 1;
                tiros[i].x = nave.x + (d * espacamento) - (espacamento * (disparos-1)/2);
                tiros[i].y = y_base - NAVE_H/1.2;
                tiros[i].y_vel = vel_tiro;
                tiros[i].cor = nave.cor;
                break;
            }
        }
    }
}

// Verifica se todos aliens morreram
int todos_os_aliens_mortos(Alien aliens[5][5]) {
    for (int i=0; i<5; i++)
        for (int j=0; j<5; j++)
            if (aliens[i][j].vivo) return 0;
    return 1;
}

////////////////////////////////////////////////////////////////////////////////// MISSOES E FASES

void resetar_missao() {
    aliens_eliminados = 0;
    tempo_jogo_segundos = 0;
    aliens_objetivo = 10 + fase_atual * 5;
    tempo_missao = 30 - (fase_atual/3)*5;
    if (tempo_missao < 10) tempo_missao = 10;

    // Velocidade dos aliens aumenta com dificuldade e fase
    aliens_x_vel = 2 + fase_atual * 0.3;
    if (dificuldade == FACIL) aliens_x_vel *= 0.7;
    else if (dificuldade == DIFICIL) aliens_x_vel *= 1.3;

    initAliens_matriz(aliens);
    initTiros(tiros);
    initPowerUps();
    podeAtirar = 1;
    powerup_ativo = 0;
}

void desenhar_missao(ALLEGRO_FONT *fonte) {
    char texto[128];
    sprintf(texto, "Fase %d - Missão: elimine %d aliens em %d seg", fase_atual, aliens_objetivo, tempo_missao);
    al_draw_text(fonte, al_map_rgb(255,255,255), 10, 10, 0, texto);

    sprintf(texto, "Aliens eliminados: %d", aliens_eliminados);
    al_draw_text(fonte, al_map_rgb(255,255,255), 10, 30, 0, texto);

    if (powerup_ativo) {
        char pwr[64];
        if (tipo_powerup_ativo == TIRO_RAPIDO) {
            sprintf(pwr, "Power-up ativo: %s (%ds)", "Tiro Rápido", tempo_powerup_ativo);
        } 
        else {
            sprintf(pwr, "Power-up ativo: %s (%ds)", "Tiro Duplo", tempo_powerup_ativo);
        }
        al_draw_text(fonte, al_map_rgb(255, 255, 0), 10, 50, 0, pwr);
    }
}

void atualizar_tempo_missao(ALLEGRO_TIMER *timer) {
    if (al_get_timer_count(timer) % 60 == 0) { //se fps for 60
        tempo_jogo_segundos++;
        if (powerup_ativo && --tempo_powerup_ativo <= 0) {
            powerup_ativo = 0;
        }
    }
}

void verificar_missao() {
    if (tempo_jogo_segundos >= tempo_missao) {
        if (!todos_os_aliens_mortos(aliens)) {
            // Tempo acabou e ainda tem aliens vivos: Game Over
            estado_jogo = GAME_OVER;
        }
    }

    if (todos_os_aliens_mortos(aliens)) {
        missoes_completas++;  // incrementa aqui
        fase_atual++;
        if (fase_atual > TOTAL_FASES) {
            estado_jogo = MENU;
            fase_atual = 1;
        } else {
            resetar_missao();
        }
    }
}


////////////////////// TELAS /////////////////////////

void desenhar_menu(ALLEGRO_FONT *fonte) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, 100, ALLEGRO_ALIGN_CENTER, "SPACE INVADERS - MENU PRINCIPAL");
    al_draw_text(fonte, al_map_rgb(200, 200, 200), SCREEN_W/2, 180, ALLEGRO_ALIGN_CENTER, "Pressione 1 para Jogar");
    al_draw_text(fonte, al_map_rgb(200, 200, 200), SCREEN_W/2, 220, ALLEGRO_ALIGN_CENTER, "Pressione ESC para Sair");
    char texto[64];
    int missoes_completas = fase_atual - 1;
    if (missoes_completas < 0) missoes_completas = 0; // segurança
    sprintf(texto, "Missões completas: %d / %d", missoes_completas, TOTAL_FASES);
    al_draw_text(fonte, al_map_rgb(255, 255, 0), SCREEN_W/2, 260, ALLEGRO_ALIGN_CENTER, texto);
    al_flip_display();
}

void desenhar_selecao_nave(ALLEGRO_FONT *fonte) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, 100, ALLEGRO_ALIGN_CENTER, "ESCOLHA SUA NAVE");
    al_draw_text(fonte, al_map_rgb(204, 0, 0), SCREEN_W/2, 160, ALLEGRO_ALIGN_CENTER, "1 - Nave A (vermelha, tiro simples rápido)");
    al_draw_text(fonte, al_map_rgb(0, 204, 0), SCREEN_W/2, 200, ALLEGRO_ALIGN_CENTER, "2 - Nave B (verde, tiro duplo)");
    al_draw_text(fonte, al_map_rgb(0, 0, 204), SCREEN_W/2, 240, ALLEGRO_ALIGN_CENTER, "3 - Nave C (azul, tiro simples normal)");
    al_flip_display();
}

void desenhar_selecao_dificuldade(ALLEGRO_FONT *fonte) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, 100, ALLEGRO_ALIGN_CENTER, "ESCOLHA A DIFICULDADE");
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, 160, ALLEGRO_ALIGN_CENTER, "1 - Fácil");
    al_draw_text(fonte, al_map_rgb(255, 255, 100), SCREEN_W/2, 200, ALLEGRO_ALIGN_CENTER, "2 - Médio");
    al_draw_text(fonte, al_map_rgb(255, 100, 100), SCREEN_W/2, 240, ALLEGRO_ALIGN_CENTER, "3 - Difícil");
    al_flip_display();
}

void desenhar_game_over(ALLEGRO_FONT *fonte) {
    al_clear_to_color(al_map_rgb(100, 0, 0));
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, SCREEN_H/2 - 40, ALLEGRO_ALIGN_CENTER, "GAME OVER!");
    char buffer[128];
    sprintf(buffer, "Você eliminou %d aliens.", aliens_eliminados);
    al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W/2, SCREEN_H/2, ALLEGRO_ALIGN_CENTER, buffer);
    al_draw_text(fonte, al_map_rgb(200, 200, 200), SCREEN_W/2, SCREEN_H/2 + 40, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para voltar ao menu.");
    al_flip_display();
}

/////////////////////// CENÁRIO ////////////////////////

//void que desenha o fundo e que desenha o solo
void draw_scenario(){
	al_clear_to_color(al_map_rgb(0, 0, 30));
	//formato, tamanhos e cor do solo em relacao as dimensoes da tela
	al_draw_filled_pieslice(SCREEN_W / 2, SCREEN_H + 1030,  1100, ALLEGRO_PI,
	ALLEGRO_PI, al_map_rgb(0, 128, 255));
}

///////////////////////// MAIN ///////////////////////////////

//variavel para estabelecer o inicio do jogo como 0s decorridos
int tempo_jogo = 0;

int main() {

	ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
    ALLEGRO_TIMER *timer = NULL;

    srand(time(NULL));

    /// ROTINAS ALLEGRO

	//rotina que inicializa o Alegro
	if(!al_init()) {
   		fprintf(stderr, "failed to initialize allegro!\n");
    	return -1;
	}

	//inicializa o módulo de primitivas do Allegro
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
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

    al_init_font_addon();
    al_init_ttf_addon();
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_FONT *fonte = al_load_ttf_font("arial.ttf", 24, 0);
    if (!fonte) {
        printf("Erro ao carregar fonte.\n");
        return -1;
    }

 	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	//cria a fila de eventos
	fila_eventos = al_create_event_queue();
	if(!fila_eventos) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}


	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(fila_eventos, al_get_display_event_source(display));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    //registra na fila os eventos de mouse (ex: clicar em um botao do mouse)
	al_register_event_source(fila_eventos, al_get_mouse_event_source());  	
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(fila_eventos, al_get_timer_event_source(timer));

    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(2);

    //inicializacoes do allegro de musicas
    ALLEGRO_SAMPLE *musica = al_load_sample("MiniGame2.wav");
    ALLEGRO_SAMPLE_INSTANCE *musicaInstance = al_create_sample_instance(musica);
    al_set_sample_instance_playmode(musicaInstance, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(musicaInstance, al_get_default_mixer());

    if (!musica){
	    fprintf(stderr, "Erro ao carregar o som!\n");
    }

    al_set_sample_instance_gain(musicaInstance, 0.5);

    al_play_sample_instance(musicaInstance);

    //inicia o temporizador
    al_start_timer(timer);

    //inits de nave, aliens e tiros
    Nave nave;
    initNave(&nave);

    initAliens_matriz(aliens);
    initTiros(tiros);
    initPowerUps();

    int sair = 0;
    int redraw = 1;

	//loop principal para rodar o jogo (enquanto "sair" for falso (0), ou seja, enquanto o jogador estiver jogando, faça as coisas)
    while (!sair) {
        ALLEGRO_EVENT evento;
		//espera por um evento e o armazena na variavel de evento ev
        al_wait_for_event(fila_eventos, &evento);

		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            redraw = 1;

            switch (estado_jogo) {
                case MENU:
                    // nada a atualizar no timer no menu
                    break;

                case ESCOLHENDO_NAVE:
                    // nada a atualizar
                    break;

                case ESCOLHENDO_DIFICULDADE:
                    // nada a atualizar
                    break;

                case JOGANDO:
                	//relogio de tempo de jogo na tela
			        if(al_get_timer_count(timer) % (int)FPS == 0) {
    			        tempo_jogo++;
			        }
                    update_nave(&nave);
                    update_aliens(aliens);
                    update_tiros(tiros, aliens, nave);
                    atualizar_powerups(nave);
                    atualizar_tempo_missao(timer);
                    verificar_missao();
                    // se algum alien chegou no chão ou bateu na nave, game over
                    for (int i=0; i<5; i++) {
                        for (int j=0; j<5; j++) {
                            if (aliens[i][j].vivo && colisao_alien_solo(aliens[i][j])) {
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
                                estado_jogo = GAME_OVER;
                            }
                        }
                    }
                    break;

                case GAME_OVER:
                    // nada a atualizar
                    break;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = 1;
        }
        //aperta a tecla
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (estado_jogo) {
                case MENU:
                    if (evento.keyboard.keycode == ALLEGRO_KEY_1) {
                        estado_jogo = ESCOLHENDO_NAVE;
                    } else if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                        sair = 1;
                    }
                    break;

                case ESCOLHENDO_NAVE:
                    if (evento.keyboard.keycode == ALLEGRO_KEY_1) {
                        nave_selecionada = NAVE_A;
                        initNave(&nave);
                        estado_jogo = ESCOLHENDO_DIFICULDADE;
                    } else if (evento.keyboard.keycode == ALLEGRO_KEY_2) {
                        nave_selecionada = NAVE_B;
                        initNave(&nave);
                        estado_jogo = ESCOLHENDO_DIFICULDADE;
                    } else if (evento.keyboard.keycode == ALLEGRO_KEY_3) {
                        nave_selecionada = NAVE_C;
                        initNave(&nave);
                        estado_jogo = ESCOLHENDO_DIFICULDADE;
                    }
                    break;

                case ESCOLHENDO_DIFICULDADE:
                    if (evento.keyboard.keycode == ALLEGRO_KEY_1) {
                        dificuldade = FACIL;
                        resetar_missao();
                        estado_jogo = JOGANDO;
                    } else if (evento.keyboard.keycode == ALLEGRO_KEY_2) {
                        dificuldade = MEDIO;
                        resetar_missao();
                        estado_jogo = JOGANDO;
                    } else if (evento.keyboard.keycode == ALLEGRO_KEY_3) {
                        dificuldade = DIFICIL;
                        resetar_missao();
                        estado_jogo = JOGANDO;
                    }
                    break;

                case JOGANDO:
                    if (evento.keyboard.keycode == ALLEGRO_KEY_A) nave.esq = 1;
                    if (evento.keyboard.keycode == ALLEGRO_KEY_D) nave.dir = 1;
                    if (evento.keyboard.keycode == ALLEGRO_KEY_SPACE && podeAtirar) {
                        atirar(tiros, nave);
                        podeAtirar = 0;
                    }
                    if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                        estado_jogo = MENU;
                    }
                    break;

                case GAME_OVER:
                    if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                        estado_jogo = MENU;
                    }
                    break;
            }
        }
        //solta a tecla
        else if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            if (estado_jogo == JOGANDO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_A) nave.esq = 0;
                if (evento.keyboard.keycode == ALLEGRO_KEY_D) nave.dir = 0;
            }
        }

        if (redraw && al_is_event_queue_empty(fila_eventos)) {
            redraw = 0;

            switch (estado_jogo) {
                case MENU:
                    desenhar_menu(fonte);
                    break;
                case ESCOLHENDO_NAVE:
                    desenhar_selecao_nave(fonte);
                    break;
                case ESCOLHENDO_DIFICULDADE:
                    desenhar_selecao_dificuldade(fonte);
                    break;
                case JOGANDO:
                    draw_scenario();
                    desenhar_missao(fonte);
                    draw_nave(nave);
                    draw_aliens(aliens);
                    draw_tiros(tiros);
                    desenhar_powerups();
                    //escrita do relogio
			        char tempo_texto[64];
			        sprintf(tempo_texto, "Tempo: %ds", tempo_jogo);
			        al_draw_text(fonte, al_map_rgb(255,255,255), SCREEN_W - 10, SCREEN_H - 25, ALLEGRO_ALIGN_RIGHT, tempo_texto);
                    al_flip_display();
                    break;
                case GAME_OVER:
                    desenhar_game_over(fonte);
                    break;
            }
        }
    }

    // Limpeza
    al_destroy_font(fonte);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(fila_eventos);
    al_destroy_sample(musica);

    return 0;
}