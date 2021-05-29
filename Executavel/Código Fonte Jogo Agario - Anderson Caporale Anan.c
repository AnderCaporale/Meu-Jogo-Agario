//         BIBLIOTECAS

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

//          DEFINIÇÕES
#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define QTD_INIMIGOS 300
#define QTD_LINHAS 100
#define VEL_LINHAS 10

//Enumeração das teclas para ficar mais facil na hora de usar
enum TECLAS {CIMA, BAIXO, DIREITA, ESQUERDA};

//          ESTRUTURAS

//Estrutura COR usada para o padrao RGB
typedef struct cor {
    int red;
    int green;
    int blue;
} COR;

//Estrutura JOGADOR, recebe as informações do jogador
typedef struct jogador {
    int x;
    int y;
    int raio;
    int velocidade;
    int estado; //0 = normal, 1 = envenenado
    int pontos;
    COR cor;
} JOGADOR;

//Estrutura INIMIGOS, recebe as informações do inimigos
typedef struct inimigos {
    int x;
    int y;
    int raio;
    int direcao;    //0 = Norte; 1=Sul; 2=Leste; 3=Oeste; 4=Nordeste; 5=Sudeste; 6=Sudoeste; 7=Noroeste
    int estado;     //0 = morto; 1 = vivo
    int tipo;       //0 = estáticas; 1 = envenenados; 2 = explosivas
    COR cor;
} INIMIGOS;

//Estrutura LINHA para as linhas do fundo
typedef struct linhas_background {
    int x1;
    int y1;
    int x2;
    int y2;
} LINHA;

//Estrutura TEMPO para os pontos
typedef struct tempo {
    time_t inicio;
    time_t fim;
    int andando;
} TEMPO;

//Estrutura FUNDO para guardar a posição da imagem de fundo
typedef struct fundo {
    float x;
    float y;
} FUNDO;



//          PROTÓTIPOS DE FUNÇÕES

void iniciarJogador (JOGADOR*);
void iniciarInimigos (INIMIGOS[]);
void iniciarBackground (LINHA[], LINHA []);
void iniciarFundo (FUNDO*);
void desenharInimigos (INIMIGOS[]);
void desenharFundo(LINHA[], LINHA []);
void moverBaixo (JOGADOR , INIMIGOS [], bool[], LINHA[], FUNDO *);
void moverCima (JOGADOR , INIMIGOS [], bool[], LINHA[], FUNDO *);
void moverDir(JOGADOR , INIMIGOS [], bool[], LINHA[], FUNDO *);
void moverEsq (JOGADOR , INIMIGOS [], bool[], LINHA[], FUNDO *);
void inimigosLinear (INIMIGOS []);
void inimigosPerseguidores (INIMIGOS [], JOGADOR );
void inimigosAleatorios(INIMIGOS []);
void respawnInimigos (INIMIGOS [], JOGADOR , LINHA [], LINHA []);
int colisaoJogador (JOGADOR *, INIMIGOS [], clock_t [], ALLEGRO_SAMPLE * , ALLEGRO_SAMPLE *);
void colisaoInimgos (INIMIGOS []);
void limitesInimigo (INIMIGOS [], LINHA [], LINHA []);
void limitesJogador (JOGADOR , LINHA [], LINHA []);
void inicio (TEMPO *);
void para (TEMPO *);
void continua (TEMPO *);
int calculaTempo (TEMPO);
void salvarJogo(JOGADOR , INIMIGOS [], LINHA [], LINHA [], int, TEMPO, FUNDO , int );
void carregarJogo(JOGADOR *, INIMIGOS [], LINHA [], LINHA [], int *, TEMPO*, FUNDO *, int *);
void opcoesMenu (int*, int*, int*, int*, int*, int, int, ALLEGRO_EVENT_QUEUE*, ALLEGRO_EVENT, ALLEGRO_SAMPLE *, ALLEGRO_FONT *);
void opcoesMorte (int*, int*, int*, int*, int, int, ALLEGRO_EVENT_QUEUE *, ALLEGRO_EVENT , ALLEGRO_SAMPLE *, ALLEGRO_FONT *);


//          MAIN
int main()
{
    //  VARIAVEIS
    ALLEGRO_EVENT eventoJogo; //Variavel para Eventos no Jogo
    ALLEGRO_EVENT_QUEUE *filaEventoJogo = NULL; //Ponteiro para Fila de Eventos no jogo
    ALLEGRO_EVENT eventoMenu; //Variavel para Eventos no Menu
    ALLEGRO_EVENT_QUEUE *filaEventoMenu = NULL; //Ponteiro para Fila de Eventos no Menu
    ALLEGRO_DISPLAY *tela = NULL;   //Ponteiro para o display
    ALLEGRO_TIMER *timer = NULL;    //Ponteiro para o timer
    ALLEGRO_BITMAP *fundoMenu = NULL;   //Ponteiro para a imagem de fundo do menu
    ALLEGRO_BITMAP *fundoJogo = NULL;   //Ponteiro para a imagem de fundo do jogo
    ALLEGRO_BITMAP *fundoMorte = NULL;  //Ponteiro para a imagem de fundo da tela de morte
    ALLEGRO_SAMPLE *musicaMorte = NULL, *somColisao = NULL,*somMorrer = NULL, *somClique = NULL; //Ponteiros para sons
    ALLEGRO_AUDIO_STREAM *musicaFundo = NULL; //Ponteiro para a musica de fundo

    JOGADOR jogador, *pJogador = &jogador;  //Variavel e ponteiro do tipo JOGADOR
    INIMIGOS inimigos[QTD_INIMIGOS];    //Vetor do tipo INIMIGOS
    LINHA vertical[QTD_LINHAS], horizontal[QTD_LINHAS]; //Vetores do tipo LINHA
    TEMPO tempo, *pTempo = &tempo; //Variavel e ponteiro do tipo PONTOS
    FUNDO coordFundo, *pCoordFundo = &coordFundo;   //Variavel e ponteiro do tipo FUNDO

    bool teclas[4] = {false, false, false, false}; //Vetor do tipo booleano para as teclas de movimento

    clock_t tempoVeneno[2]; //Vetor do tipo clock_t para calcular tempo envenenado

    int fim = 1, *pFim = &fim, menu = 1, *pMenu = &menu, jogo = 0, *pJogo = &jogo, morte = 0, pMorte = &morte; //Variaveis para cada loop
    int FPS = 50;   //Variavel para o FPS (Taxa de quadros por segundo)
    int i;  //Variavel para iteração
    int mouseX, mouseY; //Variaveis para pegar a posição do mouse
    int pause = 1, *pPause = &pause;    //Variavel e ponteiro para saber quando o jogo foi pausado
    int pontosTotal = 0, *pPontosTotal = &pontosTotal; //Variavel em que será somado os pontos.
    int novoJogo=0, *pNovoJogo = &novoJogo, jogoSalvo=0, *pJogoSalvo = &jogoSalvo; //Variavel para saber se será um novo jogo ou um jogo salvo


    //  INICIALIZACAO ALLEGRO E TELA
    if (!al_init()) //Se der erro ao inicializiar a biblioteca allegro
    {
        al_show_native_message_box(NULL, "Mensagem de Erro!","Motivo:", "Erro ao inicializar!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }

    tela = al_create_display(LARGURA_TELA, ALTURA_TELA);   //Criação do display do jogo
    al_set_window_title(tela, "AGARIO"); //Muda o nome da tela para AGARIO

    if (!tela)  //Se der erro ao inicializiar a tela
    {
        al_show_native_message_box(NULL, "Mensagem de Erro!!", "Motivo!", "Erro na Tela!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }


    //  INICIALIZACAO DE ADDONS E DISPOSITIVOS
    al_install_keyboard();  //Inicizaliza funções do Teclado
    al_install_mouse();     //Inicizaliza funções do Mouse
    al_init_primitives_addon(); //Inicizaliza funções de Desenhos
    al_init_image_addon();  //Inicizaliza funções para imagens
    al_init_font_addon();   //Inicizaliza funções de fontes
    al_init_ttf_addon();    //Inicizaliza extensoes de fontes
    al_install_audio();     //Inicizaliza funções de audio
    al_init_acodec_addon(); //Para utilizar o tipo ogg de audio
    al_reserve_samples(5);  //Reserva espaço para 5 musicas/sons
    timer = al_create_timer(1.0/FPS); //Timer, exibe FPS por segundo

    //  IMAGENS DE FUNDO
    fundoMenu = al_load_bitmap("Imagens/fundoMenu.png"); //Imagem que será usada no fundo do menu
    fundoJogo = al_load_bitmap("Imagens/fundoJogo.jpg"); //Imagem que será usada no fundo do jogo
    fundoMorte = al_load_bitmap("Imagens/fundoMorte.jpg"); //Imagem que será usada no fundo da tela de morte


    //  MUSICAS E SONS
    somColisao = al_load_sample("Sons/Som_Colisao.ogg");    //Som ao colidir
    somMorrer = al_load_sample("Sons/Som_Morrer.ogg");    //Som ao morrer
    somClique = al_load_sample("Sons/Som_Clique.ogg");  //Som ao clicar em uma opção
    musicaMorte = al_load_sample("Sons/Musica_Morte.ogg");    //Musica ao morrer
    musicaFundo = al_load_audio_stream("Sons/Musica_Fundo.ogg", 4, 1024);  //Musica de fundo

    al_attach_audio_stream_to_mixer(musicaFundo, al_get_default_mixer());    //Coloca a musica no mixer
    al_set_audio_stream_playmode(musicaFundo, ALLEGRO_PLAYMODE_LOOP);
    al_set_audio_stream_gain(musicaFundo, 0.5);      //Volume da musica de fundo


    //  CRIAÇÃO DE FONTES
    ALLEGRO_FONT *fonte15 = al_load_font("Fontes/arial.ttf", 15, NULL);    //Cria uma fonte arial de tamanho 15
    ALLEGRO_FONT *fonte20 = al_load_font("Fontes/arial.ttf", 20, NULL);    //Cria uma fonte arial de tamanho 20
    ALLEGRO_FONT *fonte30 = al_load_font("Fontes/arial.ttf", 30, NULL);    //Cria uma fonte arial de tamanho 30
    ALLEGRO_FONT *fonte40 = al_load_font("Fontes/arial.ttf", 40, NULL);    //Cria uma fonte arial de tamanho 40
    ALLEGRO_FONT *fonte30Mine = al_load_font("Fontes/Minecraft.ttf", 28, NULL);    //Cria uma fonte Minecraft de tamanho 28
    ALLEGRO_FONT *fonte70ASMAN = al_load_font("Fontes/ASMAN.ttf", 70, NULL);    //Cria uma fonte ASMAN de tamanho 70

    //  CRIAÇÃO DA FILA DE EVENTOS

    filaEventoMenu = al_create_event_queue();   //Fila de eventos do Menu


    //  REGISTRO DE SOURCES

    al_register_event_source(filaEventoMenu, al_get_mouse_event_source());        //Habilita eventos do mouse
    al_register_event_source(filaEventoMenu, al_get_display_event_source(tela));    //Habilita eventos da tela


    //  TIMER
    srand (time(NULL)); //Gerar numeros aleatorios
    al_start_timer(timer);  //Inicia o timer

    while(fim)  //LOOP DO JOGO INTEIRO
    {
        al_set_audio_stream_playing(musicaFundo, 1); //Começa a musica de fundo

        while (menu == 1)    //LOOP DO MENU
        {
            al_draw_bitmap(fundoMenu, 0, 0, NULL);      //Plano de fundo
            al_wait_for_event(filaEventoMenu, &eventoMenu);     //Espera um evento

            //FECHAR O JOGO
            if (eventoMenu.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {       //Se foi clicado no X do display ou clicado em ESC
                    jogo = 0;
                    fim = 0;
                    menu = 0;
            }
            if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_AXES)
            {
                mouseX = eventoMenu.mouse.x;
                mouseY = eventoMenu.mouse.y;
            }

            //Botões
            al_draw_filled_rounded_rectangle(290,245,510,285,20,20,al_map_rgb (255,255,255)); //Desenha um quadrado preenchido
            al_draw_text(fonte30Mine, al_map_rgb(0,0,0), 325, 253, NULL, "Iniciar Jogo");   //Imprime a mensagem na tela

            al_draw_filled_rounded_rectangle(290,300,510,340,20,20,al_map_rgb (255,255,255));   //Desenha um quadrado preenchido
            al_draw_text(fonte30Mine, al_map_rgb(0,0,0), 305, 308, NULL, "Carregar Jogo");  //Imprime a mensagem na tela

            al_draw_filled_rounded_rectangle(290,355,510,395,20,20,al_map_rgb (255,255,255));   //Desenha um quadrado preenchido
            al_draw_text(fonte30Mine, al_map_rgb(0,0,0), 355, 363, NULL, "Fechar"); //Imprime a mensagem na tela

            //Chama a função de opções do Menu.
            opcoesMenu (pNovoJogo, pJogoSalvo, pFim, pMenu, pJogo, mouseX, mouseY, filaEventoMenu, eventoMenu, somClique, fonte30Mine);

            al_flip_display();  //Coloca as imagens na tela.
        }


        //  FUNÇÕES PARA INICAR O JOGO
        if (novoJogo)   //Se foi clicado em Novo Jogo
        {
            novoJogo = 0;   //Zera para n entrar dnv se nao foi clicado
            iniciarJogador(pJogador);   //Inicia o jogador
            iniciarInimigos(inimigos);  //Inicia os inimigos
            iniciarBackground(vertical, horizontal);       //Inicia o fundo
            inicio (pTempo);   //Inicio do cronometro dos pontos
            iniciarFundo(pCoordFundo);  //Inicio da imagem de fundo
            pause = 1;
        }
        else
        {
            if (jogoSalvo)   //Se foi cliclado em Carregar Jogo
            {
                jogoSalvo = 0;  //Zera para n entrar dnv se nao foi clicado
                //Chama a função de carregar um jogo já salvo
                carregarJogo(pJogador,inimigos,vertical,horizontal,pPontosTotal, pTempo, pCoordFundo, pPause);

            }
        }

        filaEventoJogo = al_create_event_queue();     //Fila de eventos de jogo
        al_register_event_source(filaEventoJogo, al_get_keyboard_event_source());     //Habilita eventos do teclado
        al_register_event_source(filaEventoJogo, al_get_display_event_source(tela));    //Habilita eventos da tela
        al_register_event_source(filaEventoJogo, al_get_timer_event_source(timer));   //Habilita eventos do tempo


        //  LOOP PRINCIPAL
        while (jogo)    //Equanto jogo for verdadeiro
        {
            al_draw_bitmap(fundoJogo, coordFundo.x, coordFundo.y, NULL);  //Desenha a tela de fundo
            al_wait_for_event(filaEventoJogo, &eventoJogo);   //Espera um evento

    //      EVENTOS PARA FECHAR O JOGO
            if (eventoJogo.type == ALLEGRO_EVENT_DISPLAY_CLOSE )
            {       //Se foi clicado no X do display ou clicado em ESC
                jogo = 0; //Encerra o jogo
                fim = 0; //Encerra o programa
            }
            else
            {
                //Volta para o Menu
                if (eventoJogo.type == ALLEGRO_EVENT_KEY_DOWN && eventoJogo.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                {
                    jogo = 0;   //Encerra o jogo
                    menu = 1;   //Volta para o menu
                }
                else
                {
                    if (eventoJogo.type == ALLEGRO_EVENT_KEY_DOWN && eventoJogo.keyboard.keycode == ALLEGRO_KEY_P)   //PARA PAUSAR O JOGO
                    {
                        pause = !pause;
                        if (pause == 0) //Pausa o contador de tempo
                            para(pTempo);
                        else
                            continua(pTempo); //Pausa o contador de tempo

                    }
                    else
                    {
                        //  EVENTOS DE MOVIMENTO
                        if(eventoJogo.type == ALLEGRO_EVENT_KEY_DOWN) //Ao apertar uma tecla
                        {
                            switch (eventoJogo.keyboard.keycode)
                            {
                                case ALLEGRO_KEY_UP:    //Se clicar na seta para Cima
                                    teclas[CIMA] = true;    //O elemento CIMA fica true
                                    break;
                                case ALLEGRO_KEY_DOWN:  //Se clicar na seta para Baixo
                                    teclas[BAIXO] = true;   //O elemento BAIXO fica true
                                    break;
                                case ALLEGRO_KEY_RIGHT: //Se clicar na seta para Direita
                                    teclas[DIREITA] = true; //O elemento DIREITA fica true
                                    break;
                                case ALLEGRO_KEY_LEFT:  //Se clicar na seta para Esquerda
                                    teclas[ESQUERDA] = true;   //O elemento ESQUERDA fica true
                                    break;
                            }
                        }
                        else
                        {
                            if (eventoJogo.type == ALLEGRO_EVENT_KEY_UP)    //Ao desapertar uma tecla
                            {
                                switch (eventoJogo.keyboard.keycode)
                                {
                                    case ALLEGRO_KEY_UP:    //Se soltar a seta para Cima
                                        teclas[CIMA] = false;    //O elemento CIMA fica false
                                        break;
                                    case ALLEGRO_KEY_DOWN:  //Se soltar na seta para Baixo
                                        teclas[BAIXO] = false;  //O elemento BAIXO fica false
                                        break;
                                    case ALLEGRO_KEY_RIGHT: //Se soltar na seta para Direita
                                        teclas[DIREITA] = false;    //O elemento DIREITA fica false
                                        break;
                                    case ALLEGRO_KEY_LEFT:  //Se soltar na seta para Esquerda
                                        teclas[ESQUERDA] = false;   //O elemento ESQUERDA fica false
                                        break;
                                }
                            }
                            else
                            {
                                if (eventoJogo.type == ALLEGRO_EVENT_TIMER) //Quando passa algum tempo
                                {
                                    if (pause == 1) //Se nao está pausado
                                    {
                                        moverBaixo (jogador, inimigos, teclas, horizontal, pCoordFundo);
                                        moverCima (jogador, inimigos, teclas, horizontal, pCoordFundo);
                                        moverEsq (jogador, inimigos, teclas, vertical, pCoordFundo);
                                        moverDir (jogador, inimigos, teclas, vertical, pCoordFundo);
                                    }

                                    //  VERIFICAR COLISAO
                                    //Se a função retorna -1, é porque o jogador morreu
                                    if (colisaoJogador(pJogador, inimigos, tempoVeneno, somColisao, somMorrer) == -1)
                                    {
                                        al_set_audio_stream_playing(musicaFundo, 0); //Para a musica de fundo
                                        al_play_sample(musicaMorte, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);  //Toca a musica de morte
                                        al_rest(1.0);   //Espera 1 segundo
                                        jogo = 0; //Encerra o jogo
                                        morte = 1;  //Abre tela de morte

                                        for (i=0; i<4;i++)  //Coloca os movimentos como falso
                                            teclas[i] = false;
                                    }
                                    else
                                    {
                                        if (pause == 1) //Se o jogo não está pausado, movimenta os inimigos
                                        {
                                            respawnInimigos (inimigos, jogador, vertical, horizontal);
                                            inimigosPerseguidores(inimigos,jogador);
                                            inimigosAleatorios(inimigos);
                                            inimigosLinear (inimigos);
                                            colisaoInimgos(inimigos);
                                            limitesInimigo(inimigos, vertical, horizontal);
                                            limitesJogador(jogador, vertical, horizontal);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //Se o jogador está envenenado, começa o timer do veneno
            if (jogador.estado == 1)
                tempoVeneno[1] = clock();

            //Se passar 5 segundos que o jogador está envenenado, ele volta ao normal.
            if ((tempoVeneno[1] - tempoVeneno[0]) / CLOCKS_PER_SEC == 5)
            {
                jogador.estado = 0;
                jogador.cor.red = 255; jogador.cor.green = 255; jogador.cor.blue = 0;
            }

            //      DESENHOS
            desenharFundo (vertical, horizontal);
            desenharInimigos(inimigos);
            desenharJogador(jogador);

            //      EXIBE A LEGENDA
            al_draw_filled_circle(150, 15, 10, al_map_rgb(105,105,105));
            al_draw_text(fonte20, al_map_rgb(0,0,0), 160, 5, ALLEGRO_ALIGN_LEFT, "Normais");
            al_draw_filled_circle(250, 15, 10, al_map_rgb(34,139,34));
            al_draw_text(fonte20, al_map_rgb(0,0,0), 260, 5, ALLEGRO_ALIGN_LEFT, "Envenenadas");
            al_draw_filled_circle(400, 15, 10, al_map_rgb(255,0,0));
            al_draw_text(fonte20, al_map_rgb(0,0,0), 410, 5, ALLEGRO_ALIGN_LEFT, "Explosivas!");
            al_draw_text(fonte15, al_map_rgb(0,0,0), 800, 3, ALLEGRO_ALIGN_RIGHT, "P para Pausar");
            al_draw_text(fonte15, al_map_rgb(0,0,0), 800, 25, ALLEGRO_ALIGN_RIGHT, "Esc para Sair");


            if (pause == 0)     //EXIBE MENSAGEM DE QUE O JOGO ESTÁ PAUSADO
            {
                al_set_audio_stream_playing(musicaFundo, 0); //Para a musica de fundo
                al_draw_filled_rounded_rectangle(220,150,580,250,20,20,al_map_rgb (255,0,0));   //Desenha um retangulo preenchido
                al_draw_rectangle(235,160,565,205,al_map_rgb (255,255,255),3);  //Desenha um retangulo normal
                al_draw_text(fonte40, al_map_rgb(255,255,255), LARGURA_TELA/2, 160, ALLEGRO_ALIGN_CENTER, "JOGO PAUSADO");
                al_draw_text(fonte20, al_map_rgb(0,0,0), LARGURA_TELA/2, 215, ALLEGRO_ALIGN_CENTER, "S para Salvar");

                if (eventoJogo.type == ALLEGRO_EVENT_KEY_DOWN && eventoJogo.keyboard.keycode == ALLEGRO_KEY_S)   //PARA SALVAR O JOGO
                {
                    salvarJogo(jogador, inimigos, vertical, horizontal, pontosTotal, tempo, coordFundo, pause);
                    i = 1;  //Para mudar a cor da letra ao salvar
                }
                if (i==1)   //Se foi clicado em S, muda a cor da letra da frase (branco)
                {   //Muda a cor da letra para saber que já foi salvo
                    al_draw_text(fonte20, al_map_rgb(255,255,255), LARGURA_TELA/2, 215, ALLEGRO_ALIGN_CENTER, "S para Salvar");
                }
            }
            else
            {
                i=0; //Quando sai do pause, a letra volta pra a cor preta.
                al_set_audio_stream_playing(musicaFundo, 1); //Volta a musica de fundo
            }
            //Os pontos depende dos inimigos que o jogador comeu e do tempo que passou vezes 5
            pontosTotal = jogador.pontos + calculaTempo(tempo)*5;

             //      EXIBE A PONTUAÇÃO
            al_draw_textf(fonte20, al_map_rgb(0,0,0), 0, 5, ALLEGRO_ALIGN_LEFT, "Score: %d", pontosTotal);    //Exibe a pontuação na tela

            al_flip_display();  //Exibe os desenhos na tela
        }

        while (morte)   //Se o jogador morreu
        {
            al_draw_bitmap(fundoMorte, 0, 0, NULL); //Desenha plano de fundo da morte
            al_set_audio_stream_playing(musicaFundo, 0);    //Para a musica de fundo

            al_wait_for_event(filaEventoMenu, &eventoMenu); //Espera um evento

            if (eventoMenu.type == ALLEGRO_EVENT_DISPLAY_CLOSE || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {       //Se foi clicado no X do display ou clicado em ESC
                morte = 0; //Encerra tela de morte
                fim = 0;    //Encerra o programa
            }
            else
            {
                if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_AXES)
                {
                    mouseX = eventoMenu.mouse.x;    //Pega a posiçãoX do mouse
                    mouseY = eventoMenu.mouse.y;    //Pega a posiçãoY do mouse
                }

                //Mostra mensagem de morte e os pontos na tela
                al_draw_text(fonte70ASMAN, al_map_rgb(255,0,0), LARGURA_TELA/2, 100, ALLEGRO_ALIGN_CENTER, "YOU DIED");
                al_draw_textf(fonte30,al_map_rgb(255,255,255), LARGURA_TELA/2, 180, ALLEGRO_ALIGN_CENTER, "Seus pontos: %d",  pontosTotal);

                //Botões
                al_draw_filled_rounded_rectangle(95,495,360,540,20,20,al_map_rgb (255,255,255));
                al_draw_text(fonte30, al_map_rgb(0,0,0), 100, 501, NULL, "Voltar para o menu");
                al_draw_filled_rounded_rectangle(495,495,750,540,20,20,al_map_rgb (255,255,255));
                al_draw_text(fonte30, al_map_rgb(0,0,0), 500, 501, NULL, "Jogar Novamente");

                //Opções e cliques
                opcoesMorte (pNovoJogo, pMenu, pJogo, pMorte, mouseX, mouseY, filaEventoMenu, eventoMenu, somClique, fonte30);
            }

            al_flip_display();  //Mostra desenhos na tela
        }

        //  FINALIZACAO DO PROGRAMA
        if (fim == 0)
        {
            al_destroy_display(tela);   //Destruir a tela
            al_destroy_event_queue(filaEventoJogo);   //Destruir a fila de eventos
            al_destroy_event_queue(filaEventoMenu);
            al_destroy_timer(timer);        //Destruir o timer
            al_destroy_font(fonte15);       //Destruir a fonte15
            al_destroy_font(fonte20);       //Destruir a fonte20
            al_destroy_font(fonte30);       //Destruir a fonte30
            al_destroy_font(fonte40);       //Destruir a fonte40
            al_destroy_font(fonte30Mine);       //Destruir a fonte30Mine
            al_destroy_font(fonte70ASMAN);       //Destruir a fonte70ASMAN
            al_destroy_sample(somColisao);     //Destruir o som de colisao
            al_destroy_sample(musicaMorte);     //Destruir o som de morte
            al_destroy_sample(somMorrer);     //Destruir o som de morte
            al_destroy_sample(somClique);     //Destruir o som de morte
            al_destroy_audio_stream(musicaFundo); //Destruir a musica de fundo
            al_destroy_bitmap(fundoMenu);   //Destruir a imagem de fundo do Menu
            al_destroy_bitmap(fundoJogo);   //Destruir a imagem de fundo do Jogo
            al_destroy_bitmap(fundoMorte);   //Destruir a imagem de fundo da tela de Morte
        }

    }
}




//          FUNÇÕES


//  FUNÇÕES DE INICIALIZAÇÃO

//Função inicia o jogador
void iniciarJogador (JOGADOR *jogador){
    jogador->x = LARGURA_TELA/2;
    jogador->y = ALTURA_TELA/2;
    jogador->raio = 30;
    jogador->velocidade = 10;
    jogador->estado = 0;
    jogador->cor.red = 255;
    jogador->cor.green = 255;
    jogador->cor.blue = 0;
    jogador->pontos = 0;
}

//Função inicia os inimigos
void iniciarInimigos (INIMIGOS inimigos[]){
    int i, raio;

    srand(time(NULL));

    for (i=0; i < 200; i++) //Inimigos normais
    {
        raio = rand()% 30;
        inimigos[i].x = rand() % 9000;
        inimigos[i].y = rand() % 9000;
        inimigos[i].cor.red = 105;
        inimigos[i].cor.green = 105;
        inimigos[i].cor.blue = 105;
        inimigos[i].tipo = 0;
        inimigos[i].estado = 1;
        inimigos[i].direcao = rand()%8;
        inimigos[i].raio = raio;

        if (raio < 10)  //O raio minimo é 10
        {
            inimigos[i].raio = 10;
        }
    }

    for (i=200; i < 250 ; i++)  //Inimigos envenenados
    {
        raio = rand()% 30;
        inimigos[i].x = rand() % 9000;
        inimigos[i].y = rand() % 9000;
        inimigos[i].cor.red = 34;
        inimigos[i].cor.green = 139;
        inimigos[i].cor.blue = 34;
        inimigos[i].estado = 1;
        inimigos[i].tipo = 1;
        inimigos[i].raio = raio;

        if (raio < 10)
        {
            inimigos[i].raio = 10;
        }
    }

    for (i=250; i < QTD_INIMIGOS; i++)  //Inimigos explosivos
    {
        raio = rand()% 30;
        inimigos[i].x = rand() % 9000;
        inimigos[i].y = rand() % 9000;
        inimigos[i].cor.red = 255;
        inimigos[i].cor.green = 0;
        inimigos[i].cor.blue = 0;
        inimigos[i].tipo = 2;
        inimigos[i].estado = 1;
        inimigos[i].raio = raio;

        if (raio < 10)
        {
            inimigos[i].raio = 10;
        }
    }
}

//Função inicia as linhas de fundo
void iniciarBackground (LINHA vertical[], LINHA horizontal[]){
    int i, t, j, distancia = 100;

    for (i=0, j=0, t=0; i<QTD_LINHAS; i++, t+=distancia)
    {
        horizontal[i].x1 = j;
        horizontal[i].y1 = j+t;
        horizontal[i].x2 = LARGURA_TELA;

        vertical[i].x1 = j+t;
        vertical[i].y1 = j;
        vertical[i].y2 = ALTURA_TELA;
    }
}

//Função inicia a imagem de fundo
void iniciarFundo (FUNDO *fundo){
    fundo->x=-15;
    fundo->y=-10;
}

//      FUNÇÕES DE DESENHO
//Função para desenhar o jogador
void desenharJogador (JOGADOR jogador){
    al_draw_filled_circle(jogador.x, jogador.y, jogador.raio, al_map_rgb(jogador.cor.red,jogador.cor.green,jogador.cor.blue));
}

//Função para desenhar os inimigos
void desenharInimigos (INIMIGOS inimigos[]){
    int i;

    for (i=0; i<QTD_INIMIGOS; i++)
    {
        if (inimigos[i].estado == 1)    //Se o inimigo está vivo
            al_draw_filled_circle(inimigos[i].x, inimigos[i].y, inimigos[i].raio, al_map_rgb(inimigos[i].cor.red,inimigos[i].cor.green,inimigos[i].cor.blue));
    }
}

//Função para as linhas de fundo
void desenharFundo(LINHA vertical[], LINHA horizontal[]){
    int i;

    for (i=0; i<QTD_LINHAS; i++)    //Para desenhar as linhas verticais
    {
        if (i==0 || i == QTD_LINHAS-1)  //Se é a primeira linha vertical ou a ultima, desenha ela vermelha e mais grossa
        {   //Para ser o fim do mapa
            al_draw_line(vertical[i].x1, vertical[i].y1, vertical[i].x1, vertical[i].y2, al_map_rgb(255,0,0), 30);
            al_draw_line(horizontal[i].x1, horizontal[i].y1, horizontal[i].x2, horizontal[i].y1, al_map_rgb(255,0,0), 30);
        }
        else    //linhas normais
        {
            al_draw_line(vertical[i].x1, vertical[i].y1, vertical[i].x1, vertical[i].y2, al_map_rgb(0,0,0), 2);
            al_draw_line(horizontal[i].x1, horizontal[i].y1, horizontal[i].x2, horizontal[i].y1, al_map_rgb(0,0,0), 2);
        }
    }
}


//      FUNÇÕES DE MOVIMENTO

//Função para mover para baixo
void moverBaixo (JOGADOR jogador, INIMIGOS inimigos[], bool teclas[], LINHA horizontal[], FUNDO *fundo){
    int i;

    if (horizontal[QTD_LINHAS-1].y1 > ALTURA_TELA/2 + jogador.raio) //Se o jogador está dentro do limite da tela
    {
        for (i=0; i<QTD_INIMIGOS; i++)  //Movimenta os inimigos
        {
            inimigos[i].y -= teclas[BAIXO] * jogador.velocidade;
        }

        fundo->y -= teclas[BAIXO] * 0.2;    //Movimenta a imagem de fundo

    }

    for (i=0; i<QTD_LINHAS; i++)    //Movimenta as linhas do plano de fundo
    {
        horizontal[i].y1 -= teclas[BAIXO] * VEL_LINHAS;
    }
}

//Função para mover para cima
void moverCima (JOGADOR jogador, INIMIGOS inimigos[], bool teclas[], LINHA horizontal[], FUNDO *fundo){
    int i;

    if (horizontal[0].y1 < ALTURA_TELA/2 - jogador.raio)
    {
        for (i=0; i<QTD_INIMIGOS; i++)
        {
        inimigos[i].y += teclas[CIMA] * jogador.velocidade;
        }

        fundo->y += teclas[CIMA] * 0.2;

    }

    for (i=0; i<QTD_LINHAS; i++)
    {
        horizontal[i].y1 += teclas[CIMA] * VEL_LINHAS;
    }
}

//Função para mover para direita
void moverDir(JOGADOR jogador, INIMIGOS inimigos[],bool teclas[], LINHA vertical[], FUNDO *fundo){
    int i;

    if (vertical[QTD_LINHAS-1].x1 > LARGURA_TELA/2 + jogador.raio)
    {
        for (i=0; i<QTD_INIMIGOS; i++)
            inimigos[i].x -= teclas[DIREITA] * jogador.velocidade;

        fundo->x -= teclas[DIREITA] * 0.4;
    }

    for (i=0; i<QTD_LINHAS; i++)
    {
        vertical[i].x1 -= teclas[DIREITA] * VEL_LINHAS;
    }
}

//Função para mover para esquerda
void moverEsq (JOGADOR jogador, INIMIGOS inimigos[],bool teclas[], LINHA vertical[], FUNDO *fundo){
    int i;

   if (vertical[0].x1 < LARGURA_TELA/2 - jogador.raio)
    {
        for (i=0; i<QTD_INIMIGOS; i++)
            inimigos[i].x += teclas[ESQUERDA] * jogador.velocidade;

        fundo->x += teclas[ESQUERDA] * 0.4;

    }

    for (i=0; i<QTD_LINHAS; i++)
    {
        vertical[i].x1 += teclas[ESQUERDA] * VEL_LINHAS;
    }
}

//      FUNÇÕES MOVIMENTAR INIMIGOS

//Inimigos que se movem em apenas uma direção aleatoria.
//Obs: Qnd chegam no limite do mapa, elas mudam a direção aleatoriamente.
void inimigosLinear (INIMIGOS inimigos[]) {
    int i;

    //0 = Norte; 1=Sul; 2=Leste; 3=Oeste; 4=Nordeste; 5=Sudeste; 6=Sudoeste; 7=Noroeste

    for (i=100; i < 150; i++)    //50 Inimigos se movem em apenas uma direção
    {
        switch (inimigos[i].direcao)
        {
            case 0: inimigos[i].y -= 7; break;
            case 1: inimigos[i].y += 7; break;
            case 2: inimigos[i].x += 7; break;
            case 3: inimigos[i].x -= 7; break;
            case 4: inimigos[i].x += 7; inimigos[i].y -= 7; break;
            case 5: inimigos[i].x += 7; inimigos[i].y += 7; break;
            case 6: inimigos[i].x -= 7; inimigos[i].y += 7; break;
            case 7: inimigos[i].x -= 7; inimigos[i].y -= 7; break;
        }
    }
}

//Função define um inimigo explosivo como perseguidor
void inimigosPerseguidores (INIMIGOS inimigos[], JOGADOR jogador){
    int i;

    for (i=QTD_INIMIGOS-1; i < QTD_INIMIGOS; i++)  //1 inimigo explosivo persegue o jogador
    {
        if (inimigos[i].x >= jogador.x)
        {
            inimigos[i].x -= 2;
        }
        else
        {
            inimigos[i].x += 2;
        }
        if (inimigos[i].y >= jogador.y)
        {
            inimigos[i].y -= 2;
        }
        else
        {
            inimigos[i].y += 2;
        }
    }
}

//Função para inimigos se movimentarem aleatoriamente
void inimigosAleatorios(INIMIGOS inimigos[]){
    int i;

    srand(time(NULL));
    for (i=150; i < QTD_INIMIGOS-1; i++)   //150 Inimigos se movem aleatoriamente
    {
        inimigos[i].x += 7 - rand()%14; //Inimigos se movem entre -7 e 7 de velocidade
        inimigos[i].y += 7 - rand()%14; //Inimigos se movem entre -7 e 7 de velocidade
    }
}

//Função para redesenhar inimigos mortos
void respawnInimigos (INIMIGOS inimigos[], JOGADOR jogador, LINHA vertical[], LINHA horizontal[]){
    int i, novoX, novoY;

    srand(time(NULL));

    for (i=0; i < QTD_INIMIGOS; i++)
    {
        if (inimigos[i].estado == 0)    //Se o inimigo está morto
        {
            if (vertical[0].x1+1 + vertical[QTD_LINHAS-1].x1+1 > 0) //Para gerar uma nova posição dentro dos limites do jogo
            {
                novoX = rand() % ((vertical[0].x1+1) + (vertical[QTD_LINHAS-1].x1+1));  //Gera uma nova posição X
            }
            else
            {
                novoX = rand() % ((vertical[0].x1+1) + (vertical[QTD_LINHAS-1].x1+1)) * -1;  //Gera uma nova posição X
            }
            if (horizontal[0].y1+1 + horizontal[QTD_LINHAS-1].y1+1 > 0) //Para gerar uma nova posição dentro dos limites do jogo
            {
                novoY = rand() % ((horizontal[0].y1+1) + (horizontal[QTD_LINHAS-1].y1+1));  //Gera uma nova posição Y
            }
            else
            {
                novoY = rand() % ((horizontal[0].y1+1) + (horizontal[QTD_LINHAS-1].y1+1)) * -1;
            }

            if ((novoX > jogador.x + LARGURA_TELA || novoX < jogador.x - LARGURA_TELA) &&
                (novoY > jogador.x + ALTURA_TELA || novoY < jogador.y - LARGURA_TELA))
            {   //Se a nova posição do inimigo i está fora da tela do jogador
                inimigos[i].x = novoX;  //Nova posição do inimigo
                inimigos[i].y = novoY;  //Nova posição do inimigo
                inimigos[i].estado = 1; //Inimigo fica estado vivo

                if (inimigos[i].raio <100)  //Se o raio do inimigo é maior que 100, ele cresce ao renascer
                {
                    inimigos[i].raio += inimigos[i].raio/5; //O novo inimigo aumenta 1/5 do tamanho anterior
                }
                else
                {
                    if (inimigos[i].raio < 30)  //O novo raio obrigatoriamente é igual ou maior que 30
                    {
                       inimigos[i].raio = 30;
                    }
                }
            }
        }
    }
}

//      FUNÇÕES DE COLISAO
//Colisao do jogador com inimigos
int colisaoJogador (JOGADOR *jogador, INIMIGOS inimigos[], clock_t tempoVeneno[], ALLEGRO_SAMPLE *colisao , ALLEGRO_SAMPLE *morrer){
    int i;

    for (i=0; i<QTD_INIMIGOS; i++)
    {
        if ((jogador->x + jogador->raio) > inimigos[i].x + inimigos[i].raio &&
                jogador->x < (inimigos[i].x + inimigos[i].raio) &&
                (jogador->y + jogador->raio) > inimigos[i].y + inimigos[i].raio  &&
                jogador->y < (inimigos[i].y + inimigos[i].raio))
        {
            if (inimigos[i].tipo == 2 && inimigos[i].estado == 1)
            {
                al_play_sample(morrer, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                printf ("Voce explodiu!\n");
                return -1;
            }
            if (jogador->raio > inimigos[i].raio && jogador->estado==0)   //Se houve colisao e o raio do jogador eh maior e ele nao esta envenenado
            {
                if (inimigos[i].estado == 1)   //Se o inimigo está vivo
                {
                    al_play_sample(colisao, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL); //Toca som de colisao
                    inimigos[i].estado = 0; //O inimigo morre
                    jogador->pontos += inimigos[i].raio;    //Ganha pontos conforme o tamanho do inimigo

                    if (inimigos[i].tipo == 1)  //Se o inimigo é envenenado
                    {
                        if (jogador->raio > 40) //Jogador não diminui se o raio for menor que 40
                        {
                            jogador->raio -= inimigos[i].raio/4;    //O raio do jogador diminui 1/4 do raio do inimigo q comeu
                        }
                        jogador->cor.red = 34;
                        jogador->cor.green = 139;
                        jogador->cor.blue = 34;
                        jogador->estado = 1;  //Fica envenenado e nao pode comer
                        tempoVeneno[0] = clock();   //Inicia o contador do tempo envenenado

                    }
                    else    //Se o inimigo não está envenenado, o jogador cresce
                    {
                        jogador->raio += inimigos[i].raio/5;    //O raio do jogador cresce 1/5 do raio do inimigo q comeu
                    }
                }
            }
            else
            {
                if (jogador->raio < inimigos[i].raio)       //Se o raio do jogador for menor, ele morre.
                {
                    if (inimigos[i].tipo == 0 || inimigos[i].tipo == 2)
                    {
                        al_play_sample(morrer, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                        printf ("Voce foi comido!\n");
                        return -1;
                    }
                }
            }
        }
    }
}

//Colisao de inimigos com inimigos
void colisaoInimgos (INIMIGOS inimigos[]) {
    int i,j;

    for (i=0; i < QTD_INIMIGOS; i++)
    {
        for (j=0; j < QTD_INIMIGOS; j++)
        {
            if (i!=j)
            {
                if ((inimigos[i].x + inimigos[i].raio) > inimigos[j].x + inimigos[j].raio &&
                    inimigos[i].x  < (inimigos[j].x + inimigos[j].raio) &&
                    (inimigos[i].y + inimigos[i].raio) > inimigos[j].y + inimigos[j].raio  &&
                    inimigos[i].y  < (inimigos[j].y + inimigos[j].raio)) //Colisao
                {
                    if (inimigos[j].tipo == 2 || inimigos[i].tipo == 2) //Se algum dos inimigos é explosivo
                    {
                        if (inimigos[i].estado == 1 && inimigos[j].estado == 1) //E se os 2 estao vivos
                        {
                            inimigos[i].estado = 0; //Os dois morrem
                            inimigos[j].estado = 0;
                        }

                    }
                    else //Se nao sao explosivos
                    {
                        if (inimigos[i].raio >= inimigos[j].raio) //Compara o raio dos 2 inimigos
                        {
                            if (inimigos[i].estado == 1 && inimigos[j].estado == 1) //Se os 2 inimigos estao vivos
                            {
                                inimigos[j].estado = 0; //O que tem raio menor morre
                                if (inimigos[i].raio < 100) //Se o raio for menor de 100, cresce
                                {
                                    inimigos[i].raio += inimigos[j].raio/5; //Cresce 1/5 do tamanho do inimigo q comeu
                                }

                            }
                        }
                        else
                        {
                            if (inimigos[i].raio < inimigos[j].raio) //Compara o raio dos 2 inimigos
                            {
                                if (inimigos[i].estado == 1 && inimigos[j].estado == 1)  //Se os 2 inimigos estao vivos
                                {
                                   inimigos[i].estado = 0; //O que tem raio menor morre
                                }

                            }

                        }
                    }
                }
            }
        }
    }
}

//      FUNÇÕES DE LIMITAÇÃO

//Função não deixa os inimigos saírem do campo delimitado do jogo
void limitesInimigo (INIMIGOS inimigos[], LINHA vertical[], LINHA horizontal[]) {
    int i;

    srand(time(NULL));
    for (i = 0; i< QTD_INIMIGOS; i++)
    {
         //Verifica fim do mapa na lateral esquerda
        if (inimigos[i].x <= vertical[0].x1)
        {
            inimigos[i].x += inimigos[i].raio;
            if (i>=70 && i<100)
                inimigos[i].direcao = rand()%8; //Gera nova direção para os inimigos lineares
        }
        else
        {
            //Verifica fim do mapa na lateral direita
            if (inimigos[i].x >= vertical[QTD_LINHAS-1].x1)
            {
                inimigos[i].x -= inimigos[i].raio;
                if (i>=70 && i<100)
                    inimigos[i].direcao = rand()%8; //Gera nova direção para os inimigos lineares
            }
        }

        //Verifica fim do mapa superior
        if (horizontal[0].y1 >= inimigos[i].y)
        {
            inimigos[i].y += inimigos[i].raio;
            if (i>=70 && i<100)
                inimigos[i].direcao = rand()%8; //Gera nova direção para os inimigos lineares
        }
        else
        {
            //Verifica fim do mapa inferior
            if (horizontal[QTD_LINHAS-1].y1 <= inimigos[i].y)
            {
                inimigos[i].y -= inimigos[i].raio;
                if (i>=70 && i<100)
                    inimigos[i].direcao = rand()%8; //Gera nova direção para os inimigos lineares
            }
        }
    }
}

//Função não deixa o jogador saír do campo delimitado do jogo
void limitesJogador (JOGADOR jogador, LINHA vertical[], LINHA horizontal[]) {
    int i;

    if (vertical[0].x1 >= jogador.x)   //Verifica fim do mapa na lateral esquerda
    {
        for (i = 0; i < QTD_LINHAS; i++) //Se o jogador passou do limite esquerdo, as linhas verticais voltam uma posição
        {
            vertical[i].x1 -= jogador.raio;
        }
    }
    else
    {       //Verifica fim do mapa na lateral direita
        if (vertical[QTD_LINHAS-1].x1 <= jogador.x)
        {
            for (i = 0; i < QTD_LINHAS; i++) //Se o jogador passou do limite direita, as linhas verticais voltam uma posição
            {
                vertical[i].x1 += jogador.raio;
            }
        }
    }
    if (horizontal[0].y1 >= jogador.y)   //Verifica fim do mapa superior
    {
        for (i = 0; i < QTD_LINHAS; i++)    //Se o jogador passou do limite superior, as linhas horizontais voltam uma posição
        {
            horizontal[i].y1 -= jogador.raio;
        }
    }
    else
    {       //Verifica fim do mapa inferior
        if (horizontal[QTD_LINHAS-1].y1 <= jogador.y) //Se o jogador passou do limite inferior, as linhas horizontais voltam uma posição
        {
            for (i = 0; i < QTD_LINHAS; i++)
            {
                horizontal[i].y1 += jogador.raio;
            }
        }
    }
}

//      TEMPO

//Função inicia o timer do tempo
void inicio (TEMPO *tempo){
    tempo->andando = 1;
    tempo->inicio = clock();
}

//Função para o timer do tempo
void para (TEMPO *tempo){
    if (tempo->andando)
    {
        tempo->andando = 0;
        tempo->fim = clock();
    }
}

//Função continua o timer do tempo de onde parou
void continua (TEMPO *tempo){
    if (!tempo->andando)
    {
        tempo->andando = 1;
        tempo->inicio += clock() - tempo->fim;
    }
}

//Função retorna quanto tempo passou.
int calculaTempo (TEMPO tempo){
    if(tempo.andando) //Se estiver andando o tempo é o atual menos o inicio
        return (clock() - tempo.inicio) / CLOCKS_PER_SEC;
    else//caso contrario, o tempo é a diferença entre o fim e o inicio
        return (tempo.fim - tempo.inicio) / CLOCKS_PER_SEC;

}

//      ARQUIVOS

//Função para salvar um jogo
void salvarJogo(JOGADOR jogador, INIMIGOS inimigos[], LINHA vertical[], LINHA horizontal[], int pontosTotal, TEMPO tempo, FUNDO coordFundo, int pause){

    FILE *arquivo = fopen("meu_agario.bin", "wb");

    if(!arquivo)//Se houve erro ao abrir o arquivo
        printf("Erro ao criar o arquivo");

    else
    {
        fwrite(&jogador, sizeof(JOGADOR), 1, arquivo);
        fwrite(inimigos, sizeof(INIMIGOS), QTD_INIMIGOS, arquivo);
        fwrite(vertical, sizeof(LINHA), QTD_LINHAS, arquivo);
        fwrite(horizontal, sizeof(LINHA), QTD_LINHAS, arquivo);
        fwrite(&pontosTotal, sizeof(int), 1, arquivo);
        fwrite(&tempo, sizeof(TEMPO), 1, arquivo);
        fwrite(&coordFundo, sizeof(FUNDO), 1, arquivo);
        fwrite(&pause, sizeof(int), 1, arquivo);
    }

    fclose(arquivo);
}

//Função para carregar um jogo
void carregarJogo(JOGADOR *jogador, INIMIGOS inimigos[], LINHA vertical[], LINHA horizontal[], int *pPontosTotal, TEMPO *tempo,  FUNDO *coordFundo, int *pause){

    FILE *arquivo = fopen("meu_agario.bin", "rb");

    if(!arquivo)//Se houve erro ao abrir o arquivo
        printf("Erro ao criar o arquivo");

    else
    {
        fread(jogador, sizeof(JOGADOR), 1, arquivo);
        fread(inimigos, sizeof(INIMIGOS), QTD_INIMIGOS, arquivo);
        fread(vertical, sizeof(LINHA), QTD_LINHAS, arquivo);
        fread(horizontal, sizeof(LINHA), QTD_LINHAS, arquivo);
        fwrite(pPontosTotal, sizeof(int), 1, arquivo);
        fread(tempo, sizeof(TEMPO), 1, arquivo);
        fread(coordFundo, sizeof(FUNDO), 1, arquivo);
        fread(pause, sizeof(FUNDO), 1, arquivo);
    }
    fclose(arquivo);
}


void opcoesMenu (int *novoJogo, int *jogoSalvo, int *fim, int *menu, int *jogo, int mouseX, int mouseY,
                 ALLEGRO_EVENT_QUEUE *filaEventoMenu, ALLEGRO_EVENT eventoMenu, ALLEGRO_SAMPLE *somClique,
                 ALLEGRO_FONT *fonte30Mine){


    if (mouseX >= 290 && mouseX <= 510 && mouseY >= 245 && mouseY <= 285)
    {
        al_draw_filled_rounded_rectangle(290,245,510,285,20,20,al_map_rgb (0,0,255));
        al_draw_text(fonte30Mine, al_map_rgb(255,255,255), 325, 250, NULL, "Iniciar Jogo");

        if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || eventoMenu.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (eventoMenu.mouse.button & 1 || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                al_play_sample(somClique, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                *novoJogo = 1;
                *menu = 0;
                *jogo = 1;
            }
        }

    }
    else
    {
        if (mouseX >= 290 && mouseX <= 510 && mouseY >= 300 && mouseY <= 340)
        {
            al_draw_filled_rounded_rectangle(290,300,510,340,20,20,al_map_rgb (0,0,255));
            al_draw_text(fonte30Mine, al_map_rgb(255,255,255), 305, 305, NULL, "Carregar Jogo");

            if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || eventoMenu.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (eventoMenu.mouse.button & 1 || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    al_play_sample(somClique, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                    *jogoSalvo = 1;
                    *menu = 0;
                    *jogo = 1;
                }
            }
        }
        else
        {
            if (mouseX >= 290 && mouseX <= 510 && mouseY >= 355 && mouseY <= 395)
            {
                al_draw_filled_rounded_rectangle(290,355,510,395,20,20,al_map_rgb (0,0,255));
                al_draw_text(fonte30Mine, al_map_rgb(255,255,255), 355, 360, NULL, "Fechar");
                if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || eventoMenu.type == ALLEGRO_EVENT_KEY_DOWN)
                {
                    if (eventoMenu.mouse.button & 1 || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    {
                        al_play_sample(somClique, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                        *menu = 0;
                        *jogo = 0;
                        *fim = 0;
                    }
                }
            }
        }
    }
    al_flip_display();
}


void opcoesMorte (int *novoJogo, int *menu, int *jogo, int *morte, int mouseX, int mouseY,
                 ALLEGRO_EVENT_QUEUE *filaEventoMenu, ALLEGRO_EVENT eventoMenu, ALLEGRO_SAMPLE *somClique,
                 ALLEGRO_FONT *fonte30){

    if (mouseX >= 95 && mouseX <= 360 && mouseY >= 495 && mouseY <= 540)
    {
        al_draw_filled_rounded_rectangle(95,495,360,540,20,20,al_map_rgb (255,0,0));
        al_draw_text(fonte30, al_map_rgb(0,0,0), 100, 501, NULL, "Voltar para o menu");

        if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || eventoMenu.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (eventoMenu.mouse.button & 1 || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                al_stop_samples();
                al_play_sample(somClique, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                *morte = 0;
                *menu = 1;
            }
        }
    }
    else
    {
        if (mouseX >= 495 && mouseX <= 750 && mouseY >= 495 && mouseY <= 540)
        {
            al_draw_filled_rounded_rectangle(495,495,750,540,20,20,al_map_rgb (255,0,0));
            al_draw_text(fonte30, al_map_rgb(0,0,0), 500, 501, NULL, "Jogar Novamente");

            if (eventoMenu.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || eventoMenu.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (eventoMenu.mouse.button & 1 || eventoMenu.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    al_stop_samples();
                    al_play_sample(somClique, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE,NULL);
                    *morte = 0;
                    *novoJogo = 1;
                    *jogo = 1;
                }
            }
        }
    }
}

