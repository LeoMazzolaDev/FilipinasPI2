#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h> 

const int LARGURA_TELA = 1280;
const int ALTURA_TELA = 720;
const int LARGURA_FRAME = 150;
const int ALTURA_FRAME = 150;
const int VELOCIDADE_PERSONAGEM = 5;
const float GRAVIDADE = 0.4;
const float IMPULSO_PULO = -10.0;
const float ALTURA_CHAO = 500;
const float FPS = 60.0;

const int NUM_FRAMES_ANDAR = 2;
const int NUM_FRAMES_PULO = 1;

// Coordenadas do botão de início
const int BOTAO_INICIO_X1 = 520;
const int BOTAO_INICIO_X2 = 750;
const int BOTAO_INICIO_Y1 = 580;
const int BOTAO_INICIO_Y2 = 680;

int main() {
    al_init();
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();

    // Criar a tela e verificar sucesso
    ALLEGRO_DISPLAY* display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!display) {
        fprintf(stderr, "Falha ao criar a tela.\n");
        return -1;
    }

    // Criar o timer
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    al_start_timer(timer);

    

    // Carregar o sprite do personagem
    ALLEGRO_BITMAP* sprite_personagem = al_load_bitmap("src/felipe-150-sprites.png");
    if (!sprite_personagem) {
        fprintf(stderr, "Falha ao carregar o sprite do personagem.\n");
        al_destroy_display(display);
        return -1;
    }

    // Carregar as imagens de fundo das fases
    ALLEGRO_BITMAP* fase1 = al_load_bitmap("src/bg1-open-door.png");
    ALLEGRO_BITMAP* fase2 = al_load_bitmap("src/bg2-open-door.png");
    ALLEGRO_BITMAP* fase3 = al_load_bitmap("src/bg3-open-door.png");
    if (!fase1 || !fase2 || !fase3) {
        fprintf(stderr, "Falha ao carregar uma ou mais imagens de fase.\n");
        al_destroy_bitmap(sprite_personagem);       
        al_destroy_display(display);
        return -1;
    }

   
    // Carregar a tela de fim de jogo
    ALLEGRO_BITMAP* tela_fim = al_load_bitmap("src/bg-Fim.png");
    if (!tela_fim) {
        fprintf(stderr, "Falha ao carregar a tela de fim de jogo.\n");
        al_destroy_bitmap(sprite_personagem);
        al_destroy_bitmap(fase1);
        al_destroy_bitmap(fase2);
        al_destroy_bitmap(fase3);
        al_destroy_display(display);
        return -1;
    }

    // Carregar a tela inicial
    ALLEGRO_BITMAP* tela_inicial = al_load_bitmap("src/Tela_Inicial.png.jpg");
    if (!tela_inicial) {
        fprintf(stderr, "Falha ao carregar a tela inicial.\n");
        al_destroy_bitmap(sprite_personagem);
        al_destroy_bitmap(fase1);
        al_destroy_bitmap(fase2);
        al_destroy_bitmap(fase3);
        al_destroy_display(display);
        al_destroy_bitmap(tela_fim);
        return -1;
    }

    
    ALLEGRO_EVENT_QUEUE* fila_eventos = al_create_event_queue();
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_mouse_event_source());
    al_register_event_source(fila_eventos, al_get_display_event_source(display));
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));

    bool iniciar_jogo = false;
    bool sair = false;
    bool desenhar = true;
    int fase_atual = 1; 

    // Loop da tela inicial
    while (!iniciar_jogo && !sair) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        // Verificar saída do jogo
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = true;
        }
        // Verificar clique do mouse
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            int mouse_x = evento.mouse.x;
            int mouse_y = evento.mouse.y;

            // Verificar se o clique do mouse está dentro da área do botão start
            if (mouse_x >= BOTAO_INICIO_X1 && mouse_x <= BOTAO_INICIO_X2 &&
                mouse_y >= BOTAO_INICIO_Y1 && mouse_y <= BOTAO_INICIO_Y2) {
                iniciar_jogo = true;
            }
        }

        // Desenhar a tela inicial
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(tela_inicial, 0, 0, 0);
        al_flip_display();
    }

    // Verificar se o jogador escolheu sair do jogo
    if (sair) {
        al_destroy_bitmap(sprite_personagem);
        al_destroy_bitmap(fase1);
        al_destroy_bitmap(fase2);
        al_destroy_bitmap(fase3);
        al_destroy_bitmap(tela_inicial);
        al_destroy_bitmap(tela_fim);
        al_destroy_display(display);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
        return 0;
    }

    // Destruir a tela inicial, pois não será mais necessária
    al_destroy_bitmap(tela_inicial);

    // Variáveis do personagem
    float pos_x = 20;
    float pos_y = ALTURA_CHAO;
    float velocidade_y = 0;
    bool no_ar = false;
    bool olhando_para_direita = true;
    int frame_x = 0;  
    int frame_y = 0; 
    int contador_frames = 0;
    int tempo_entre_frames = 8;

    bool tecla_direita = false;
    bool tecla_esquerda = false;

    // Loop principal do jogo
    while (!sair) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = true;
        }
        else if (evento.type == ALLEGRO_EVENT_TIMER) {
            // Atualizar a posição do personagem com base nas teclas pressionadas
            if (tecla_direita) {
                pos_x += VELOCIDADE_PERSONAGEM;
                olhando_para_direita = true;
                if (!no_ar) frame_y = 300;
            }

            if (tecla_esquerda) {
                pos_x -= VELOCIDADE_PERSONAGEM;
                olhando_para_direita = false;
                if (!no_ar) frame_y = 450;            
            }

            // Aplicar gravidade enquanto no ar
            if (no_ar) {
                velocidade_y += GRAVIDADE;
                frame_y = olhando_para_direita ? 601 : 751; // Animação de pulo no sprite
            }
            pos_y += velocidade_y;

            // Verificar colisão com o chão
            if (pos_y >= ALTURA_CHAO) {
                pos_y = ALTURA_CHAO;
                velocidade_y = 0;
                no_ar = false;
                frame_y = olhando_para_direita ? 300 : 450; // Voltar à animação de andar
            }

            // Verificar se o jogador chegou ao final da fase
            if (pos_x >= LARGURA_TELA - LARGURA_FRAME) {
                if (fase_atual < 3) {
                    fase_atual++;
                    pos_x = 10; 
                }
                else {
                    // Ir para a tela de fim de jogo após completar a última fase
                    break;
                }
            }

            // Alternar entre os frames de animação de andar
            contador_frames++;
            if (contador_frames >= tempo_entre_frames) {
                if (tecla_direita || tecla_esquerda) {
                    frame_x = (frame_x + 1) % NUM_FRAMES_ANDAR; 
                }
                else if (no_ar) {
                    frame_x = (frame_x +1 ) % NUM_FRAMES_PULO;
                }
                contador_frames = 0;
            }

            desenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (evento.keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
                sair = true;
                break;
            case ALLEGRO_KEY_RIGHT:
                tecla_direita = true;
                break;
            case ALLEGRO_KEY_LEFT:
                tecla_esquerda = true;
                break;
            case ALLEGRO_KEY_SPACE:
                if (!no_ar) {
                    velocidade_y = IMPULSO_PULO;
                    no_ar = true;
                }
                break;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            switch (evento.keyboard.keycode) {
            case ALLEGRO_KEY_RIGHT:
                tecla_direita = false;
                break;
            case ALLEGRO_KEY_LEFT:
                tecla_esquerda = false;
                break;
            }
        }

        // Desenhar a tela apenas quando necessário
        if (desenhar && al_is_event_queue_empty(fila_eventos)) {
            desenhar = false;

            // Limpar a tela
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Desenhar o cenário da fase atual
            if (fase_atual == 1) {
                al_draw_bitmap(fase1, 0, 0, 0);
            }
            else if (fase_atual == 2) {
                al_draw_bitmap(fase2, 0, 0, 0);
            }
            else if (fase_atual == 3) {
                al_draw_bitmap(fase3, 0, 0, 0);
            }

            // Desenhar o personagem
            al_draw_bitmap_region(
                sprite_personagem,
                frame_x * LARGURA_FRAME, frame_y,
                LARGURA_FRAME, ALTURA_FRAME,
                pos_x, pos_y,
                0
            );

            // Atualizar a tela
            al_flip_display();
        }
    }

    // Exibir a tela de fim de jogo
    bool fim_jogo = true;
    while (fim_jogo) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            fim_jogo = false;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            fim_jogo = false;
        }

        // Desenhar a tela de fim
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_bitmap(tela_fim, 0, 0, 0);
        al_flip_display();
    }

    // Destruir recursos
    al_destroy_bitmap(sprite_personagem);
    al_destroy_bitmap(fase1);
    al_destroy_bitmap(fase2);
    al_destroy_bitmap(fase3);
    al_destroy_bitmap(tela_fim);
    al_destroy_display(display);
    al_destroy_event_queue(fila_eventos);
    al_destroy_timer(timer);

    return 0;
}