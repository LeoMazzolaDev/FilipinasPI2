#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h> 

const int LARGURA_TELA = 1280;
const int ALTURA_TELA = 720;
const int LARGURA_FRAME = 50;
const int ALTURA_FRAME = 50;
const int VELOCIDADE_PERSONAGEM = 5;
const float GRAVIDADE = 0.3;
const float IMPULSO_PULO = -5.0;
const float ALTURA_CHAO = 650;
const float FPS = 30.0;

int main() {
    // Inicializar Allegro e os módulos
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

    // Carregar o sprite do personagem quer estar na pasta src
    ALLEGRO_BITMAP* sprite_personagem = al_load_bitmap("src/felipe-sprite.png");
    if (!sprite_personagem) {
        fprintf(stderr, "Falha ao carregar o sprite do personagem.\n");
        al_destroy_display(display);
        return -1;
    }

    // Aki estamos carregar as imagens de fundo das 3 fases
    ALLEGRO_BITMAP* fase1 = al_load_bitmap("src/Fase1.png");
    ALLEGRO_BITMAP* fase2 = al_load_bitmap("src/Fase2.png");
    ALLEGRO_BITMAP* fase3 = al_load_bitmap("src/Fase3.png");
    if (!fase1 || !fase2 || !fase3) {
        fprintf(stderr, "Falha ao carregar uma ou mais imagens de fase.\n");
        al_destroy_bitmap(sprite_personagem);
        al_destroy_display(display);
        return -1;
    }

    // Carregar a tela inicial a partir que ta na pasta src
    ALLEGRO_BITMAP* tela_inicial = al_load_bitmap("src/Tela_Inicial.png.jpg");
    if (!tela_inicial) {
        fprintf(stderr, "Falha ao carregar a tela inicial.\n");
        al_destroy_bitmap(sprite_personagem);
        al_destroy_bitmap(fase1);
        al_destroy_bitmap(fase2);
        al_destroy_bitmap(fase3);
        al_destroy_display(display);
        return -1;
    }

    // Configurar eventos
    ALLEGRO_EVENT_QUEUE* fila_eventos = al_create_event_queue();
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_mouse_event_source());
    al_register_event_source(fila_eventos, al_get_display_event_source(display));
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));

    bool iniciar_jogo = false;
    bool sair = false;
    bool desenhar = true;
    int fase_atual = 1; // Iniciar na fase 1

    // Loop da tela inicial
    while (!iniciar_jogo && !sair) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);

        // Verificar saída do jogo
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            int mouse_x = evento.mouse.x;
            int mouse_y = evento.mouse.y;

            // estamos verificando se o clique do mouse está dentro da área do botão start que eh (550, 700) no x e (480, 785) no y
            if (mouse_x >= 550 && mouse_x <= 700 && mouse_y >= 480 && mouse_y <= 785) {
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
        al_destroy_display(display);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
        return 0;
    }

    // Destruir a tela inicial, pois não será mais necessária
    al_destroy_bitmap(tela_inicial);

    // Variáveis do personagem
    float pos_x = 50;
    float pos_y = ALTURA_CHAO;
    float velocidade_y = 0;
    bool no_ar = false;
    bool olhando_para_direita = true;
    int frame_x = 0;
    int frame_y = 0;
    int contador_frames = 0;
    int tempo_entre_frames = 10;

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
                if (!no_ar) {
                    frame_y = 100;  // Andando para a direita (50 * 2)
                }
            }
            if (tecla_esquerda) {
                pos_x -= VELOCIDADE_PERSONAGEM;
                if (!no_ar) {
                    frame_y = 150;  // Andando para a esquerda (50 * 3)
                }
            }

            // Aplicar gravidade enquanto no ar
            if (no_ar) {
                velocidade_y += GRAVIDADE;
            }
            pos_y += velocidade_y;

            // Verificar colisão com o chão
            if (pos_y >= ALTURA_CHAO) {
                pos_y = ALTURA_CHAO;
                velocidade_y = 0;
                no_ar = false;
                frame_y = olhando_para_direita ? 0 : 50;  // Parado para a direita (50 * 0) ou esquerda (50 * 1)
            }

            // Verificar se o jogador chegou ao final da fase
            if (pos_x >= LARGURA_TELA - LARGURA_FRAME) {
                if (fase_atual < 3) {
                    fase_atual++;
                    pos_x = 50; // Reiniciar a posição do jogador na nova fase
                }
                else {
                    // Final do jogo se todas as fases foram completadas
                    sair = true;
                }
            }

            // Alternar entre os dois frames para animação
            contador_frames++;
            if (contador_frames >= tempo_entre_frames) {
                frame_x = (frame_x == 0) ? 1 : 0;
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
                olhando_para_direita = true;
                break;
            case ALLEGRO_KEY_LEFT:
                tecla_esquerda = true;
                olhando_para_direita = false;
                break;
            case ALLEGRO_KEY_SPACE:
                if (!no_ar) {
                    velocidade_y = IMPULSO_PULO;
                    no_ar = true;
                    frame_y = olhando_para_direita ? 200 : 250;
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

    // Destruir recursos
    al_destroy_bitmap(sprite_personagem);
    al_destroy_bitmap(fase1);
    al_destroy_bitmap(fase2);
    al_destroy_bitmap(fase3);
    al_destroy_display(display);
    al_destroy_event_queue(fila_eventos);
    al_destroy_timer(timer);

    return 0;
}
