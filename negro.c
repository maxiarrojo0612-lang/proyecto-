#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ANCHO_PANTALLA 1900
#define ALTO_PANTALLA 1150
#define TAMANO_CUADRADO 50
#define VELOCIDAD 4
#define ALTO 23
#define ANCHO 38
#define VIDA_MAXIMA 6
#define MAX_ENEMIGOS 12
#define VELOCIDAD_ENEMIGO 3
#define MAX_BALAS_EN 20
#define MAX_BALAS_PJ 4
#define RANGO_PERSECUCION 400.0f
#define RANGO_DISPARO 500.0f
#define COOLDOWN_DISPARO 2.0f
#define TIEMPO_EXPLOSION 90
#define RADIO_EXPLOSION 1
#define MAX_TOP 10
#define TOTAL_NIVELES 6
#define LARG_NOMBRE 10
#define NUM_PALANCAS 4
#define PALANCA_AZUL     0
#define PALANCA_VERDE    1
#define PALANCA_ROJA     2
#define PALANCA_AMARILLA 3
#define MAX_OBJ 7

//agregar (x2)...............2
//sonido.............3
//animación
//item de rango de balas...............2
//que solo se guarde el puntaje cuando ganas.........1
//sprite de jugador

typedef enum {
    ESTADO_MENU,
    ESTADO_CONTROLES,
    ESTADO_PUNTAJE,
    ESTADO_JUGANDO,
    ESTADO_GAME_OVER,
    ESTADO_WIN,
    ESTADO_INGRESAR_NOMBRE,
    ESTADO_PAUSA,
    ESTADO_PANTALLA_DE_CARGA,
} EstadoJuego;
typedef enum {
    EVENTO_MATAR_ENEMIGO,
    EVENTO_DESTRUIR_PARED,
    EVENTO_PASAR_NIVEL,
    EVENTO_RECIBIR_DANIO,
    EVENTO_COIN,
} EventoPuntaje;

typedef struct {
    char nombre[10];
    int consumir;
    int X,Y;
    bool activo;
    bool activada;
    int contador;
} Objeto;
typedef struct {
    float startX, startY;
    float X, Y;
    float velX, velY;
    bool activa;
    int rangoMaximo;
} municion;

typedef struct {
    Objeto items[MAX_OBJ];
} mochila;

typedef struct personaje {
    char nombreIngresado[LARG_NOMBRE];
    int vida;
    int balas;
    float X, Y;
    float velocidad;
    mochila bag;
    int direccion;
    municion listaBalas[MAX_BALAS_PJ];
    int cooldownPinches;
    float rangoDisparo;
    int puntos;
    int lenNombre;
} personaje;

typedef struct {
    float X, Y;
    float velocidad;
    bool activo;
    int tipo;
    int sentido;
    float ultimoDisparo;
    int visible;
    float ultimoCambioVisibilidad;
    float dirX, dirY;
    municion listaBalas[MAX_BALAS_EN];
} enemigos;
typedef struct {
    personaje player;
    EstadoJuego estadoActual;
    enemigos listaEnemigos[MAX_ENEMIGOS];
    bool tieneLlave;
    bool cofreAbierto;
    bool portalActivo;
    int nivelActual;
    int palancasRequeridas[NUM_PALANCAS];
    int palancasActivadas[NUM_PALANCAS];
    char pistaNivel[128];
} Juego;
typedef struct {
    char nombre[LARG_NOMBRE];
    int puntos;
} RegistroPuntaje;

ALLEGRO_BITMAP *spriteJugador = NULL;
ALLEGRO_BITMAP *spriteEnemigo = NULL;
ALLEGRO_BITMAP *spriteFantasma = NULL;
ALLEGRO_BITMAP *spritePared = NULL;
ALLEGRO_BITMAP *spritefondo = NULL;
ALLEGRO_BITMAP *spriteCaja = NULL;
ALLEGRO_BITMAP *spriteLlave = NULL;
ALLEGRO_BITMAP *spriteCofre = NULL;
ALLEGRO_BITMAP *spritePortal = NULL;
ALLEGRO_BITMAP *spriteBala = NULL;
ALLEGRO_BITMAP *spriteCoin = NULL;
ALLEGRO_BITMAP *spriteBalaPJ = NULL;
ALLEGRO_BITMAP *spriteCorazon = NULL;
ALLEGRO_BITMAP *spriteMedioCorazon = NULL;
ALLEGRO_BITMAP *spritePalanca_ROJA = NULL;
ALLEGRO_BITMAP *spritePalanca_VERDE = NULL;
ALLEGRO_BITMAP *spritePalanca_AMARILLA = NULL;
ALLEGRO_BITMAP *spritePalanca_AZUL = NULL;
ALLEGRO_BITMAP* spriteBombaNormal = NULL;
ALLEGRO_BITMAP* spriteBombaActivada = NULL;
ALLEGRO_BITMAP* spriteBombaExplotada = NULL;
ALLEGRO_BITMAP* spritePinches = NULL;
ALLEGRO_BITMAP* spriteSuper_En = NULL;

void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO], Juego *j);
void dibujarVida(int vida);
void consumibles(char mapa[ALTO][ANCHO], Juego *j);
void reiniciarJuego(char mapa[ALTO][ANCHO], Juego *j);
void cargarAssets();
void actualizarPuntaje(personaje *player, EventoPuntaje evento);
void danhoexplosion(char mapa[ALTO][ANCHO], Objeto *o, Juego *j);
void danhopinches(char mapa[ALTO][ANCHO], Juego *j);
void moverEnemigos(char mapa[ALTO][ANCHO], Juego *j);
void guardarNuevoPuntaje(int nuevosPuntos, const char *nombre);
void cargarPuntajes(RegistroPuntaje top[]);
void dispararEnemigo(enemigos *en, float playerX, float playerY);
void actualizarBalasEnemigos(char mapa[ALTO][ANCHO], Juego *j);
bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala, Juego *j);
void cargarNivelActual(char mapa[ALTO][ANCHO], Juego *j);
void avanzarSiguienteNivel(char mapa[ALTO][ANCHO], Juego *j);
bool validarRequisitosPortal(Juego *j);
void danhoEnemigos(Juego *j);
void dibujarInterfaz(personaje *p, int balas, ALLEGRO_FONT *fuente);
void asignarComportamientos( Juego *j);
void dispararPJ(char mapa[ALTO][ANCHO], Juego *j);
int buscarSlotLibre(personaje *p);
void limpiarInventario(personaje *p);
bool tieneObjeto(personaje *p, char *nombre);

int main() {

    Juego miJuego;
    Objeto miObjeto;
    miJuego.tieneLlave = false;
    miJuego.cofreAbierto = false;
    miJuego.portalActivo = false;
    miObjeto.activada = false;
    bool salir = false;
    bool redibujar = true;
    char mapa[ALTO][ANCHO];
    RegistroPuntaje top[MAX_TOP];
    const char* textoMostrar;
    
    srand(time(NULL));

    if (!al_init()) {
        printf("ERROR: No se pudo inicializar Allegro.\n");
        return -1;
    }

    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA);

    ALLEGRO_DISPLAY *display = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA);

    cargarAssets();

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_FONT *miFuente = al_create_builtin_font();

    ALLEGRO_FONT *fuenteTitulo = al_load_ttf_font("arial.ttf", 100, 0);
    ALLEGRO_FONT *fuenteMenu   = al_load_ttf_font("arial.ttf", 32, 0);

    if (!fuenteTitulo || !fuenteMenu) {
        return -1;
    }

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    cargarNivelActual(mapa, &miJuego);
    asignarComportamientos(&miJuego);
    al_start_timer(timer);
    ALLEGRO_BITMAP *fondoCompleto = NULL;

    if (spritefondo != NULL) {
        fondoCompleto = al_create_bitmap(ANCHO_PANTALLA, ALTO_PANTALLA); 
        al_set_target_bitmap(fondoCompleto);
        for (int i = 0; i < ALTO; i++) {
            for (int j = 0; j < ANCHO; j++) {
                al_draw_bitmap(spritefondo, j * 50, i * 50, 0);
            }
        }
        al_set_target_bitmap(al_get_backbuffer(display));
    } 

    miJuego.player.vida = 6;
    miJuego.player.balas = 6;
    miJuego.player.rangoDisparo = RANGO_DISPARO;
    int opcionSeleccionada = 0;

    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        if (evento.type == ALLEGRO_EVENT_TIMER) {
            redibujar = true;

            if (miJuego.estadoActual == ESTADO_JUGANDO) {
                ALLEGRO_KEYBOARD_STATE teclado;
                al_get_keyboard_state(&teclado);

                float nuevaX = miJuego.player.X, nuevaY = miJuego.player.Y;
                if (al_key_down(&teclado, ALLEGRO_KEY_W)) { 
                    nuevaY -= VELOCIDAD; 
                    miJuego.player.direccion = 3; 
                }
                if (al_key_down(&teclado, ALLEGRO_KEY_S)) { 
                    nuevaY += VELOCIDAD; 
                    miJuego.player.direccion = 0; 
                }
                if (al_key_down(&teclado, ALLEGRO_KEY_A)) { 
                    nuevaX -= VELOCIDAD; 
                    miJuego.player.direccion = 1; 
                }
                if (al_key_down(&teclado, ALLEGRO_KEY_D)) { 
                    nuevaX += VELOCIDAD; 
                    miJuego.player.direccion = 2; 
                }

                colision(mapa, &miJuego.player.X, &miJuego.player.Y, nuevaX, nuevaY);
                moverEnemigos(mapa, &miJuego);
                consumibles(mapa, &miJuego);
                dispararPJ(mapa, &miJuego);
                danhoEnemigos(&miJuego);
                danhoexplosion(mapa, &miObjeto, &miJuego);
                danhopinches(mapa, &miJuego);

                if (miJuego.player.vida <= 0) {
                    miJuego.estadoActual = ESTADO_GAME_OVER;
                    miJuego.player.lenNombre = 0;
                    miJuego.player.nombreIngresado[0] = '\0';
                }
            }
        }
        
        //menu y demas
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (miJuego.estadoActual == ESTADO_MENU) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_UP) {
                    opcionSeleccionada--;
                    if (opcionSeleccionada < 0) 
                        opcionSeleccionada = 3;
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                    opcionSeleccionada++;
                    if (opcionSeleccionada > 3) 
                        opcionSeleccionada = 0;
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    if (opcionSeleccionada == 0) {
                        miJuego.player.puntos = 0;
                        miJuego.nivelActual = 1;
                        asignarComportamientos(&miJuego);
                        cargarNivelActual(mapa, &miJuego);
                    }
                    else if (opcionSeleccionada == 1) {
                        miJuego.estadoActual = ESTADO_CONTROLES;
                    }
                    else if (opcionSeleccionada == 2) {
                        cargarPuntajes(top);
                        miJuego.estadoActual = ESTADO_PUNTAJE;
                    }
                    else if (opcionSeleccionada == 3) {
                        salir = true;
                    }
                }
            }
            else if (miJuego.estadoActual == ESTADO_CONTROLES || miJuego.estadoActual == ESTADO_PUNTAJE) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    miJuego.estadoActual = ESTADO_MENU;
                }
            }
            else if (miJuego.estadoActual == ESTADO_PAUSA){
                if (evento.keyboard.keycode == ALLEGRO_KEY_UP) {
                    opcionSeleccionada--;
                    if (opcionSeleccionada < 0) 
                        opcionSeleccionada = 1;
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                    opcionSeleccionada++;
                    if (opcionSeleccionada > 1) 
                        opcionSeleccionada = 0;
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    if (opcionSeleccionada == 0) {
                        miJuego.estadoActual = ESTADO_JUGANDO;
                    }
                    else if (opcionSeleccionada == 1) {
                        miJuego.estadoActual = ESTADO_MENU;
                    } 
                }
            }
            else if (miJuego.estadoActual == ESTADO_JUGANDO) {
                if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    miJuego.estadoActual = ESTADO_PAUSA;
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_F) {
                    int col = (int)((miJuego.player.X + 25) / 50.0);
                    int fil = (int)((miJuego.player.Y + 25) / 50.0);
                    if (tieneObjeto(&miJuego.player, "Bomba")) {
                        if (mapa[fil][col] == '.'){

                            mapa[fil][col] = 'B';
                            miObjeto.activada = true;
                            miObjeto.contador = TIEMPO_EXPLOSION;
                            miObjeto.X = col; 
                            miObjeto.Y = fil;
                            mapa[fil][col] = 'b';
                        }
                        for (int i = 0; i < 10; i++) {
                            if (strcmp(miJuego.player.bag.items[i].nombre, "Bomba") == 0) {
                                miJuego.player.bag.items[i].nombre[0] = '\0';
                                break;
                            }
                        }
                    }
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_E) {
            
                    int col = (int)((miJuego.player.X + 25) / 50.0);
                    int fil = (int)((miJuego.player.Y + 25) / 50.0);
                    if (mapa[fil][col] == 'X') {
                        char itemsPosibles[] = {'M', '+', 'c', 'B'};
                        int numItems = sizeof(itemsPosibles) / sizeof(itemsPosibles[0]);
                        int indiceAzar = rand() % numItems;
                        mapa[fil][col] = itemsPosibles[indiceAzar];
                    }

                    if (mapa[fil][col] == '1') { 
                        miJuego.palancasActivadas[PALANCA_AZUL] = 1;
                        mapa[fil][col] = 'a';
                    }
                    else if (mapa[fil][col] == '2') { 
                        miJuego.palancasActivadas[PALANCA_VERDE] = 1;
                        mapa[fil][col] = 'v';
                    }
                    else if (mapa[fil][col] == '3') { 
                        miJuego.palancasActivadas[PALANCA_ROJA] = 1;
                        mapa[fil][col] = 'r';
                    }
                    else if (mapa[fil][col] == '4') { 
                        miJuego.palancasActivadas[PALANCA_AMARILLA] = 1;
                        mapa[fil][col] = 'm';
                    }
                }
                if (evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                    if (miJuego.player.balas > 0) {
                        for (int i = 0; i < MAX_BALAS_PJ; i++) {
                            if (!miJuego.player.listaBalas[i].activa) {
                                miJuego.player.listaBalas[i].X = miJuego.player.X; 
                                miJuego.player.listaBalas[i].Y = miJuego.player.Y;
                                miJuego.player.listaBalas[i].startX = miJuego.player.X;
                                miJuego.player.listaBalas[i].startY = miJuego.player.Y;
                                miJuego.player.listaBalas[i].rangoMaximo = miJuego.player.rangoDisparo;
                                miJuego.player.listaBalas[i].activa = true;
                                if (miJuego.player.direccion == 2) {
                                    miJuego.player.listaBalas[i].velX = 10;
                                } 
                                else if (miJuego.player.direccion == 1) {
                                    miJuego.player.listaBalas[i].velX = -10;
                                } 
                                else {
                                    miJuego.player.listaBalas[i].velX = 0;
                                }
                                if (miJuego.player.direccion == 0) {
                                    miJuego.player.listaBalas[i].velY = 10;
                                } 
                                else if (miJuego.player.direccion == 3) {
                                    miJuego.player.listaBalas[i].velY = -10;
                                } 
                                else {
                                    miJuego.player.listaBalas[i].velY = 0;
                                }
                                miJuego.player.balas--; 
                                break;
                            }
                        }
                    }
                }
            }
        }
        //pantalla game over y win
        else if (evento.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (miJuego.estadoActual == ESTADO_GAME_OVER || miJuego.estadoActual == ESTADO_WIN) {

                if (evento.keyboard.keycode == ALLEGRO_KEY_ENTER || evento.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER) {
                    if (miJuego.player.lenNombre > 0) {
                        guardarNuevoPuntaje(miJuego.player.puntos, miJuego.player.nombreIngresado);
                        reiniciarJuego(mapa, &miJuego);
                        miJuego.player.vida = 6;
                        miJuego.player.puntos = 0;
                        miJuego.estadoActual = ESTADO_MENU;
                    }
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    reiniciarJuego(mapa, &miJuego);
                    miJuego.player.vida = 6;
                    miJuego.player.puntos = 0;
                    miJuego.estadoActual = ESTADO_MENU;
                }
                else if (evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && miJuego.player.lenNombre > 0) {
                    miJuego.player.lenNombre--;
                    miJuego.player.nombreIngresado[miJuego.player.lenNombre] = '\0';
                }
                else if (evento.keyboard.unichar >= 32 && evento.keyboard.unichar <= 122 && miJuego.player.lenNombre < 9) {
                    miJuego.player.nombreIngresado[miJuego.player.lenNombre] = evento.keyboard.unichar;
                    miJuego.player.lenNombre++;
                    miJuego.player.nombreIngresado[miJuego.player.lenNombre] = '\0';
                }
            }
        }

        // cerrar ventana
        else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            salir = true;
        }
    

        //dibujo
        if (redibujar && al_is_event_queue_empty(queue)) {
            redibujar = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

            if (miJuego.estadoActual == ESTADO_MENU) {
                al_draw_text(fuenteTitulo, al_map_rgb(255, 255, 255), 960, 100, ALLEGRO_ALIGN_CENTER, "Explorer");

                ALLEGRO_COLOR colorJugar, colorControles, colorPuntaje, colorSalir;

                if (opcionSeleccionada == 0) {
                    colorJugar = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorJugar = al_map_rgb(200, 200, 200);
                }
                if (opcionSeleccionada == 1) {
                    colorControles = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorControles = al_map_rgb(200, 200, 200);
                }
                if (opcionSeleccionada == 2) {
                    colorPuntaje = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorPuntaje = al_map_rgb(200, 200, 200);
                }

                if (opcionSeleccionada == 3) {
                    colorSalir = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorSalir = al_map_rgb(200, 200, 200);
                }

                al_draw_text(fuenteMenu, colorJugar, 960, 320, ALLEGRO_ALIGN_CENTER, "JUGAR");
                al_draw_text(fuenteMenu, colorControles, 960, 370, ALLEGRO_ALIGN_CENTER, "CONTROLES");
                al_draw_text(fuenteMenu, colorPuntaje, 960, 420, ALLEGRO_ALIGN_CENTER, "PUNTAJE");
                al_draw_text(fuenteMenu, colorSalir, 960, 470, ALLEGRO_ALIGN_CENTER, "SALIR");
            }
            else if (miJuego.estadoActual == ESTADO_CONTROLES) {
                al_draw_text(fuenteTitulo, al_map_rgb(255, 255, 255), 960, 100, ALLEGRO_ALIGN_CENTER, "CONTROLES");
                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 300, ALLEGRO_ALIGN_CENTER, "WASD: Moverse");
                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 340, ALLEGRO_ALIGN_CENTER, "ESPACIO: Disparar");
                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 380, ALLEGRO_ALIGN_CENTER, "F: Activar bomba");
                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 380, ALLEGRO_ALIGN_CENTER, "E: Interactuar");
                al_draw_text(fuenteMenu, al_map_rgb(255, 255, 0), 960, 900, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver");
            }
            else if (miJuego.estadoActual == ESTADO_PUNTAJE) {
                al_draw_text(fuenteTitulo, al_map_rgb(255, 255, 0), 960, 50, ALLEGRO_ALIGN_CENTER, "MEJORES PUNTAJES");

                int posY = 200;
                for (int i = 0; i < MAX_TOP; i++) {
                    char linea[64];
                    sprintf(linea, "%d.  %-12s  %05d PTS", i + 1, top[i].nombre, top[i].puntos);
                    al_draw_text(fuenteMenu, al_map_rgb(255, 255, 255), 960, posY, ALLEGRO_ALIGN_CENTER, linea);
                    posY += 50;
                }

                al_draw_text(fuenteMenu, al_map_rgb(150, 150, 150), 960, 900, ALLEGRO_ALIGN_CENTER, "Presiona ESC para volver");
            }

            else if (miJuego.estadoActual == ESTADO_PAUSA) {
                al_clear_to_color(al_map_rgb(15, 15, 20));
                ALLEGRO_COLOR colorJugar, colorSalir;

                if (opcionSeleccionada == 0) {
                    colorJugar = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorJugar = al_map_rgb(200, 200, 200);
                }
                if (opcionSeleccionada == 1) {
                    colorSalir = al_map_rgb(255, 255, 0);
                } 
                else {
                    colorSalir = al_map_rgb(200, 200, 200);
                }
                al_draw_textf(fuenteTitulo, al_map_rgb(255, 215, 0), 960, 180, ALLEGRO_ALIGN_CENTER, "NIVEL %d", miJuego.nivelActual);
                al_draw_text(fuenteMenu, al_map_rgb(255, 255, 255), 960, 300, ALLEGRO_ALIGN_CENTER, miJuego.pistaNivel);
                al_draw_text(fuenteMenu, colorJugar, 960, 520, ALLEGRO_ALIGN_CENTER, "CONTINUAR");
                al_draw_text(fuenteMenu, colorSalir, 960, 580, ALLEGRO_ALIGN_CENTER, "SALIR AL MENU");
            }
            
            else if (miJuego.estadoActual == ESTADO_GAME_OVER) {
                al_draw_text(fuenteTitulo, al_map_rgb(255, 0, 0), 960, 150, ALLEGRO_ALIGN_CENTER, "GAME OVER");
                
                char puntosTxt[32];
                snprintf(puntosTxt, sizeof(puntosTxt), "Puntaje Final: %d", miJuego.player.puntos);
                al_draw_text(fuenteMenu, al_map_rgb(255, 255, 255), 960, 330, ALLEGRO_ALIGN_CENTER, puntosTxt);
                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 410, ALLEGRO_ALIGN_CENTER, "Ingresa tu Nombre:");

                if (miJuego.player.lenNombre == 0) {
                    textoMostrar = "_";
                } 
                else {
                    textoMostrar = miJuego.player.nombreIngresado;
                }
                al_draw_text(fuenteMenu, al_map_rgb(0, 255, 255), 960, 460, ALLEGRO_ALIGN_CENTER, textoMostrar);
                al_draw_text(fuenteMenu, al_map_rgb(150, 150, 150), 960, 540, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para guardar | ESC para omitir");
            }
            else if (miJuego.estadoActual == ESTADO_WIN){
                al_draw_text(fuenteTitulo, al_map_rgb(255, 0, 0), 960, 150, ALLEGRO_ALIGN_CENTER, "¡FELICIDADES GANASTE!");

                char puntosTxt[32];
                snprintf(puntosTxt, sizeof(puntosTxt), "Puntaje Final: %d", miJuego.player.puntos);
                al_draw_text(fuenteMenu, al_map_rgb(255, 255, 255), 960, 330, ALLEGRO_ALIGN_CENTER, puntosTxt);

                al_draw_text(fuenteMenu, al_map_rgb(200, 200, 200), 960, 410, ALLEGRO_ALIGN_CENTER, "Ingresa tu Nombre:");

                if (miJuego.player.lenNombre == 0) {
                    textoMostrar = "";
                } 
                else {
                    textoMostrar = miJuego.player.nombreIngresado;
                }
                al_draw_text(fuenteMenu, al_map_rgb(0, 255, 255), 960, 460, ALLEGRO_ALIGN_CENTER, textoMostrar);
                al_draw_text(fuenteMenu, al_map_rgb(150, 150, 150), 960, 540, ALLEGRO_ALIGN_CENTER, "Presiona ENTER para guardar | ESC para omitir");
            }

            //dibujar mapa
            else if (miJuego.estadoActual == ESTADO_JUGANDO) {
                al_draw_bitmap(fondoCompleto, 0, 0, 0);
                dibujarMapa(mapa, &miJuego);
                actualizarBalasEnemigos(mapa, &miJuego);
                dibujarInterfaz(&miJuego.player, miJuego.player.balas, miFuente);   

                //dibujar jugador
                if (spriteJugador) {
                    al_draw_scaled_bitmap(spriteJugador, 0, 0, 50, 50, miJuego.player.X, miJuego.player.Y, 50, 50, 0);
                } 

                // Dibujar balas de enemigos
                for (int e = 0; e < MAX_ENEMIGOS; e++) {
                    if (miJuego.listaEnemigos[e].activo) {
                        for (int b = 0; b < MAX_BALAS_EN; b++) {
                            if (miJuego.listaEnemigos[e].listaBalas[b].activa) {
                                al_draw_scaled_bitmap(spriteBala, 0, 0, 50, 50, miJuego.listaEnemigos[e].listaBalas[b].X, miJuego.listaEnemigos[e].listaBalas[b].Y, 50, 50, 0);
                            }
                        }
                    }
                }

                // Dibujar enemigos
                for (int i = 0; i < MAX_ENEMIGOS; i++) {
                    enemigos *e = &miJuego.listaEnemigos[i];

                    if (e->activo) {
                        int flip;
                        if (e->tipo == 1) { 
                            if (e->visible == 1) {
                                if (e->dirX == -1) {
                                    flip = 0;
                                } 
                                else {
                                    flip = ALLEGRO_FLIP_HORIZONTAL;
                                }
                                al_draw_scaled_bitmap(spriteFantasma, 0, 0, 50, 50, e->X, e->Y, 60, 50, flip);
                            }
                        } 
                        else if (e->tipo == 0) {
                            if (e->dirX == 1) {
                                flip = 0;
                            } 
                            else {
                                flip = ALLEGRO_FLIP_HORIZONTAL;
                            }
                            al_draw_scaled_bitmap(spriteSuper_En, 0, 0, 50, 50, e->X, e->Y, 60, 50, flip);
                        }
                        else {
                            if (e->dirX == 1) {
                                flip = 0;
                            } 
                            else {
                                flip = ALLEGRO_FLIP_HORIZONTAL;
                            }
                            al_draw_scaled_bitmap(spriteEnemigo, 0, 0, 50, 50, e->X, e->Y, 60, 50, flip);
                        }
                    }
                }

                // Dibujar balas del jugador
                for (int i = 0; i < MAX_BALAS_PJ; i++) {
                    if (miJuego.player.listaBalas[i].activa) {
                        al_draw_scaled_bitmap(spriteBalaPJ, 0, 0, 50, 50, miJuego.player.listaBalas[i].X, miJuego.player.listaBalas[i].Y, 50, 50, 0);
                    }
                }
                dibujarVida(miJuego.player.vida);
            }

            al_flip_display();
        }
    }

    al_destroy_bitmap(fondoCompleto);
    al_destroy_bitmap(spriteJugador);
    al_destroy_bitmap(spriteEnemigo);
    al_destroy_bitmap(spriteSuper_En);
    al_destroy_bitmap(spriteFantasma);
    al_destroy_bitmap(spritePared);
    al_destroy_bitmap(spriteCoin);
    al_destroy_bitmap(spritefondo);
    al_destroy_bitmap(spritePalanca_ROJA);
    al_destroy_bitmap(spritePalanca_AZUL);
    al_destroy_bitmap(spritePalanca_VERDE);
    al_destroy_bitmap(spritePalanca_AMARILLA);
    al_destroy_bitmap(spriteBalaPJ);
    al_destroy_bitmap(spriteBala);
    al_destroy_bitmap(spriteBombaNormal);
    al_destroy_bitmap(spriteBombaActivada);
    al_destroy_bitmap(spriteBombaExplotada);
    al_destroy_bitmap(spritePinches);
    al_destroy_bitmap(spriteCaja);
    al_destroy_display(display);
    al_destroy_font(miFuente);
    al_destroy_font(fuenteTitulo);
    al_destroy_font(fuenteMenu);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}

void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY) {

    float size = TAMANO_CUADRADO;
    if (nuevaX != *posX) {
        int direccion;

        if (nuevaX > *posX) {
            direccion = 1;
        } 
        else {
            direccion = -1;
        }
        float intentoX = *posX;
        while (intentoX != nuevaX) {
            intentoX += direccion;
            int left   = (int)(intentoX / size);
            int right  = (int)((intentoX + size - 0.001) / size);
            int top    = (int)(*posY / size);
            int bottom = (int)((*posY + size - 0.001) / size);
            bool colision = false;
            for (int y = top; y <= bottom; y++) {
                for (int x = left; x <= right; x++) {
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#'|| mapa[y][x] == '*') {
                        colision = true;
                    }
                }
            }
            if (colision) {
                intentoX -= direccion;
                break;
            }
        }
        *posX = intentoX;
    }
    if (nuevaY != *posY) {
        int direccion;

        if (nuevaY > *posY) {
            direccion = 1;
        } 
        else {
            direccion = -1;
        }
        float intentoY = *posY;
        while (intentoY != nuevaY) {
            intentoY += direccion;
            int left   = (int)(*posX / size);
            int right  = (int)((*posX + size - 0.001) / size);
            int top    = (int)(intentoY / size);
            int bottom = (int)((intentoY + size - 0.001) / size);
            bool colision = false;

            for (int y = top; y <= bottom; y++) {
                for (int x = left; x <= right; x++) {
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#'|| mapa[y][x] == '*') {
                        colision = true;
                    }
                }
            }
            if (colision) {
                intentoY -= direccion;
                break;
            }
        }
        *posY = intentoY;
    }
}
void actualizarPuntaje(struct personaje *player, EventoPuntaje evento) {
    if (player == NULL) return;

    switch (evento) {
        case EVENTO_MATAR_ENEMIGO:
            player->puntos += 100;
            break;

        case EVENTO_DESTRUIR_PARED:
            player->puntos += 25;
            break;

        case EVENTO_PASAR_NIVEL:
            player->puntos += 200;
            break;

        case EVENTO_COIN:
            player->puntos += 100;
            break;
        case EVENTO_RECIBIR_DANIO:
            player->puntos -= 50;
            
            if (player->puntos < 0) {
                player->puntos = 0;
            }
            break;
    }
}

void dispararEnemigo(enemigos *en, float playerX, float playerY) {
    int b;
    float dx;
    float dy;
    float dist;
    for(b = 0; b < MAX_BALAS_EN; b++) {
        if(!en->listaBalas[b].activa) {
            en->listaBalas[b].activa = true;
            en->listaBalas[b].X = en->X;
            en->listaBalas[b].Y = en->Y;

            dx = playerX - en->X;
            dy = playerY - en->Y;
            dist = sqrt(dx*dx + dy*dy);
            
            en->listaBalas[b].velX = (dx / dist) * 5.0f;
            en->listaBalas[b].velY = (dy / dist) * 5.0f;
            
            break;
        }
    }
}
void dispararPJ(char mapa[ALTO][ANCHO], Juego *j) {
    float dx;
    float dy;
    float distanciaY;
    float distanciaX;
    int i, b;

    for (i = 0; i < 10; i++) {
        if (j->player.listaBalas[i].activa) {
        
                j->player.listaBalas[i].X += j->player.listaBalas[i].velX;
                j->player.listaBalas[i].Y += j->player.listaBalas[i].velY;
        
                distanciaX = j->player.listaBalas[i].X - j->player.listaBalas[i].startX;
                distanciaY = j->player.listaBalas[i].Y - j->player.listaBalas[i].startY;
        
                if ((distanciaX * distanciaX + distanciaY * distanciaY) > (j->player.listaBalas[i].rangoMaximo * j->player.listaBalas[i].rangoMaximo)) {
                    j->player.listaBalas[i].activa = false;
                }
                else {
                    if (esColision(mapa, j->player.listaBalas[i].X, j->player.listaBalas[i].Y, true, j)) {
                        j->player.listaBalas[i].activa = false;
                    } 
                else {
                    for (b = 0; b < MAX_ENEMIGOS; b++) {
                        if (j->listaEnemigos[b].activo) {
                            dx = j->player.listaBalas[i].X - (j->listaEnemigos[b].X) + 25;
                            dy = j->player.listaBalas[i].Y - (j->listaEnemigos[b].Y) + 25;
                            if ((dx*dx + dy*dy) < 900) {
                                j->listaEnemigos[b].activo = false;
                                actualizarPuntaje(&j->player, EVENTO_MATAR_ENEMIGO);

                                j->player.listaBalas[i].activa = false;
                            }
                        }
                    }
                }
            }
        }
    }
}
void cargarPuntajes(RegistroPuntaje top[]) {
    int i;
    FILE *archivo = fopen("puntajes.txt", "r");
    
    if (!archivo) {
        archivo = fopen("puntajes.txt", "w");
        for (i = 0; i < MAX_TOP; i++) {
            snprintf(top[i].nombre, sizeof(top[i].nombre), "---");
            top[i].puntos = 0;
            if (archivo) {
                fprintf(archivo, "%s %d\n", top[i].nombre, top[i].puntos);
            }
        }
        if (archivo) 
            fclose(archivo);
        return;
    }

    for (i = 0; i < MAX_TOP; i++) {
        if (fscanf(archivo, "%s %d", top[i].nombre, &top[i].puntos) != 2) {
            snprintf(top[i].nombre, sizeof(top[i].nombre), "---");
            top[i].puntos = 0;
        }
    }
    fclose(archivo);
}

void guardarNuevoPuntaje(int puntosActuales, const char *nombreJugador) {
    int j;
    int i;
    RegistroPuntaje top[MAX_TOP + 1];
    cargarPuntajes(top);
    
    snprintf(top[MAX_TOP].nombre, sizeof(top[MAX_TOP].nombre), "%s", nombreJugador);
    top[MAX_TOP].puntos = puntosActuales;

    for (i = 0; i < MAX_TOP; i++) {
        for (j = i + 1; j <= MAX_TOP; j++) {
            if (top[j].puntos > top[i].puntos) {
                RegistroPuntaje aux = top[i];
                top[i] = top[j];
                top[j] = aux;
            }
        }
    }

    FILE *archivo = fopen("puntajes.txt", "w");
    if (archivo) {
        for (i = 0; i < MAX_TOP; i++) {
            fprintf(archivo, "%s %d\n", top[i].nombre, top[i].puntos);
        }
        fclose(archivo);
    }
}

void moverEnemigos(char mapa[ALTO][ANCHO], Juego *j) {

    int i;
    float tiempoActual;
    float dx;
    float dy;
    float distancia;

    tiempoActual = al_get_time();
    for (i = 0; i < MAX_ENEMIGOS; i++) {

        dx = j->player.X - j->listaEnemigos[i].X;
        dy = j->player.Y - j->listaEnemigos[i].Y;
        distancia = sqrt(dx*dx + dy*dy);

        if (!j->listaEnemigos[i].activo) {
            continue;
        }
        if (j->listaEnemigos[i].tipo == 0) { // TIPO PERSECUCIÓN 
            if (distancia < RANGO_PERSECUCION) {

                if (dx > 0){
                    j->listaEnemigos[i].dirX = 1;
                }
                else
                    j->listaEnemigos[i].dirX = -1;
                if (dy > 0){
                    j->listaEnemigos[i].dirY = 1;
                }
                else
                    j->listaEnemigos[i].dirY = -1;

                if (!esColision(mapa, j->listaEnemigos[i].X + j->listaEnemigos[i].dirX, j->listaEnemigos[i].Y, false, j)) 
                    j->listaEnemigos[i].X += j->listaEnemigos[i].dirX;
    
                if (!esColision(mapa, j->listaEnemigos[i].X, j->listaEnemigos[i].Y + j->listaEnemigos[i].dirY, false, j)) 
                    j->listaEnemigos[i].Y += j->listaEnemigos[i].dirY;

                if (distancia < RANGO_DISPARO) {
                    if (tiempoActual - j->listaEnemigos[i].ultimoDisparo >= COOLDOWN_DISPARO) {
                        dispararEnemigo(&j->listaEnemigos[i], j->player.X, j->player.Y);
                        j->listaEnemigos[i].ultimoDisparo = tiempoActual;
                    }
                }
            }
        }
        else if (j->listaEnemigos[i].tipo == 1) { // TIPO FANTASMA 
            if (tiempoActual - j->listaEnemigos[i].ultimoCambioVisibilidad >= 2.0) {
                j->listaEnemigos[i].visible = !j->listaEnemigos[i].visible;
                j->listaEnemigos[i].ultimoCambioVisibilidad = tiempoActual;
            }
            if (distancia < RANGO_PERSECUCION) {

                if (dx > 0){
                    j->listaEnemigos[i].dirX = 1;                
                }
                else
                    j->listaEnemigos[i].dirX = -1;
                if (dy > 0){
                    j->listaEnemigos[i].dirY = 1;
                }
                else
                    j->listaEnemigos[i].dirY = -1;

                j->listaEnemigos[i].X += j->listaEnemigos[i].dirX;
                j->listaEnemigos[i].Y += j->listaEnemigos[i].dirY;
            }
        }
    
        else if (j->listaEnemigos[i].tipo == 2) {
            // TIPO VERTICAL
            float proxY = j->listaEnemigos[i].Y + (j->listaEnemigos[i].sentido * 2.0);
            if (!esColision(mapa, j->listaEnemigos[i].X, proxY, false, j)) {
                j->listaEnemigos[i].Y = proxY;
            } 
            else {
                j->listaEnemigos[i].sentido *= -1;
            }
        } 
        else if (j->listaEnemigos[i].tipo == 3) {
            // TIPO HORIZONTAL
            float proxX = j->listaEnemigos[i].X + (j->listaEnemigos[i].sentido * 2.0);
            if (!esColision(mapa, proxX, j->listaEnemigos[i].Y, false, j)) {
                j->listaEnemigos[i].X = proxX;
            } else {
                j->listaEnemigos[i].sentido *= -1;
            }
        }

    }
}

void dibujarMapa(char mapa[ALTO][ANCHO], Juego *j) {
    int i;
    int col;
    float px;
    float py;
    for (i = 0; i < ALTO; i++) {
        for (col = 0; col < ANCHO; col++) {
            px = col * 50.0;
            py = i * 50.0;
            
            switch(mapa[i][col]) {
                case '.':
                    if (spritefondo) 
                        al_draw_bitmap(spritefondo, px, py, 0); 
                break;
                case '#':
                    if (spritePared) 
                        al_draw_bitmap(spritePared, px, py, 0); 
                break;
                case '*':
                    if (spritePared) 
                        al_draw_bitmap(spritePared, px, py, 0); 
                break;
                case 'K':
                    if (spriteLlave) 
                        al_draw_bitmap(spriteLlave, px, py, 0); 
                break;
                case 'C':
                    if (spriteCofre) 
                        al_draw_bitmap(spriteCofre, px, py, 0); 
                break;
                case 'M':
                    if (spriteBalaPJ) 
                    al_draw_bitmap(spriteBalaPJ, px, py, 0);
                break;
                case 'P': 
                    if (spritePortal) 
                        al_draw_bitmap(spritePortal, px, py, 0);
                break;
                case '+': 
                    if (spriteCorazon) 
                        al_draw_bitmap(spriteCorazon, px, py, 0);
                break;
                case 'B': 
                    if (spriteBombaNormal) 
                        al_draw_bitmap(spriteBombaNormal, px, py, 0);
                break;
                case 'b': 
                    if (spriteBombaActivada) 
                        al_draw_bitmap(spriteBombaActivada, px, py, 0);
                break;
                case 'o': 
                    if (spriteBombaExplotada) 
                        al_draw_bitmap(spriteBombaExplotada, px, py, 0);
                break;
                case 'p': 
                    if (spritePinches) 
                        al_draw_bitmap(spritePinches, px, py, 0); 
                break;
                case 'c': 
                    if (spriteCoin) 
                        al_draw_bitmap(spriteCoin, px, py, 0); 
                break;
                case '1': 
                    al_draw_bitmap(spritePalanca_AZUL, px, py, 0); 
                    break;
                case '2': 
                    al_draw_bitmap(spritePalanca_VERDE, px, py, 0); 
                    break;
                case '3': 
                    al_draw_bitmap(spritePalanca_ROJA, px, py, 0); 
                    break;
                case '4': 
                    al_draw_bitmap(spritePalanca_AMARILLA, px, py, 0); 
                    break;
                case 'a': 
                    al_draw_scaled_bitmap(spritePalanca_AZUL, 0, 0, 50, 50, px, py, 50, 50, ALLEGRO_FLIP_HORIZONTAL); 
                    break;
                case 'v': 
                    al_draw_scaled_bitmap(spritePalanca_VERDE, 0, 0, 50, 50, px, py, 50, 50, ALLEGRO_FLIP_HORIZONTAL); 
                    break;
                case 'r': 
                    al_draw_scaled_bitmap(spritePalanca_ROJA, 0, 0, 50, 50, px, py, 50, 50, ALLEGRO_FLIP_HORIZONTAL); 
                    break;
                case 'm': 
                    al_draw_scaled_bitmap(spritePalanca_AMARILLA, 0, 0, 50, 50, px, py, 50, 50, ALLEGRO_FLIP_HORIZONTAL); 
                    break;
                case 'X':
                    al_draw_scaled_bitmap(spriteCaja, 0, 0, 50, 50, px, py, 50, 50, 0);
                    break;
            }
        }
    }
}

void dibujarVida(int vida) {
    int i;
    int xInicial;
    int yInicial;
    int espaciado;
    int corazonesMaximos;
    int estado;

    for (i = 0; i < 3; i++) {
        xInicial = 10;
        yInicial = 10;
        espaciado = 40;
        corazonesMaximos = 5;
        estado = vida - (i * 2);
        
        if (estado >= 2) {
            al_draw_bitmap(spriteCorazon, xInicial + (i * espaciado), yInicial, 0);
        } else if (estado == 1) {
            al_draw_bitmap(spriteMedioCorazon, xInicial + (i * espaciado), yInicial, 0);
        }
    }
}
void consumibles(char mapa[ALTO][ANCHO], Juego *j) {
    int i;
    int slotPalanca;
    int col = ((j->player.X + 25) / 50.0);
    int fil = ((j->player.Y + 25) / 50.0);
    if (mapa[fil][col] == 'M') {
        j->player.balas += 3;
        mapa[fil][col] = ' ';
    }
    if (mapa[fil][col] == '+') {
        j->player.vida += 2;
        mapa[fil][col] = ' ';
    }
    if (mapa[fil][col] == 'c') {
        actualizarPuntaje(&j->player, EVENTO_COIN);
        mapa[fil][col] = ' ';
    }
    if (mapa[fil][col] == 'K') {
        for (i = 0; i < MAX_OBJ; i++) {
            if (j->player.bag.items[i].nombre[0] == '\0') {
                strcpy(j->player.bag.items[i].nombre, "Llave");
                mapa[fil][col] = ' '; 
                break;
            }
        }
    }
    if (mapa[fil][col] == 'B') {
        if (!tieneObjeto(&j->player, "Bomba")) {

            for (i = 0; i < MAX_OBJ; i++) {
                if (j->player.bag.items[i].nombre[0] == '\0') {
                    strcpy(j->player.bag.items[i].nombre, "Bomba");
                    mapa[fil][col] = ' '; 
                    break;
                }
            }
        }
    }
    if (mapa[fil][col] == 'C') {
        if (tieneObjeto(&j->player, "Llave")) {
            
                j->player.rangoDisparo += 200.0;

            mapa[fil][col] = '.';
        }
    } 
    if (mapa[fil][col] == 'P') {
        if (validarRequisitosPortal(j)) {
            j->portalActivo = true;
            avanzarSiguienteNivel(mapa, j);
            asignarComportamientos(j);
        }
    }
}

void actualizarBalasEnemigos(char mapa[ALTO][ANCHO], Juego *j) {
    int i;
    int b;
    int col;
    int fil;
    float dx;
    float dy;
    float dist;

    for (i = 0; i < MAX_ENEMIGOS; i++) {
        for (b = 0; b < MAX_BALAS_EN; b++) {
            if (j->listaEnemigos[i].listaBalas[b].activa) {

                j->listaEnemigos[i].listaBalas[b].X += j->listaEnemigos[i].listaBalas[b].velX;
                j->listaEnemigos[i].listaBalas[b].Y += j->listaEnemigos[i].listaBalas[b].velY;

                col = (int)(j->listaEnemigos[i].listaBalas[b].X / TAMANO_CUADRADO);
                fil = (int)(j->listaEnemigos[i].listaBalas[b].Y / TAMANO_CUADRADO);

                if (fil >= 0 && fil < ALTO && col >= 0 && col < ANCHO) {
                    if (mapa[fil][col] == '#') {
                        j->listaEnemigos[i].listaBalas[b].activa = false;
                        continue;
                    }
                    if (mapa[fil][col] == '*') {
                        j->listaEnemigos[i].listaBalas[b].activa = false;
                        continue;
                    }
                } 
                else {
                    j->listaEnemigos[i].listaBalas[b].activa = false;
                    continue;
                }

                dx = j->player.X - j->listaEnemigos[i].listaBalas[b].X;
                dy = j->player.Y - j->listaEnemigos[i].listaBalas[b].Y;
                dist = sqrt(dx*dx + dy*dy);

                if (dist < 35.0f) {
                    j->player.vida -= 1;
                    j->listaEnemigos[i].listaBalas[b].activa = false;
                }
            }
        }
    }
}

void cargarAssets() {
    spriteJugador = al_load_bitmap("jugador.png");
    al_convert_mask_to_alpha(spriteJugador, al_map_rgb(255, 255, 255));
    if(!spriteJugador){
        printf("spriteJugador no se pudo cargar\n");
    }

    spriteEnemigo = al_load_bitmap("enemigo.png");
    if(!spriteEnemigo){
        printf("spriteEnemigo no se pudo cargar\n");
    }
    al_convert_mask_to_alpha(spriteEnemigo, al_map_rgb(255, 255, 255));

    spriteFantasma = al_load_bitmap("fantasma.png");
    if(!spriteFantasma){
        printf("spriteFantasma no se pudo cargar\n");
    }
    spritePared = al_load_bitmap("pared.png");
    if(!spritePared){
        printf("spritePared no se pudo cargar\n");
    }
    spritefondo = al_load_bitmap("fondo.png");
    if(!spritefondo){
        printf("spritefondo no se pudo cargar\n");
    }
    spriteLlave = al_load_bitmap("llave.png");
    if(!spriteLlave){
        printf("spriteLlave no se pudo cargar\n");
    }
    spriteCofre = al_load_bitmap("cofre.png");
    if(!spriteCofre){
        printf("spriteCofre no se pudo cargar\n");
    }
    spritePortal = al_load_bitmap("portal.png");
    if(!spritePortal){
        printf("spritePortal no se pudo cargar\n");
    }
    spriteBala = al_load_bitmap("bala.png");
    if(!spriteBala){
        printf("spriteBala no se pudo cargar\n");
    }
    spriteCorazon = al_load_bitmap("corazon.png");
    if(!spriteCorazon){
        printf("spriteCorazon no se pudo cargar\n");
    }
    spriteMedioCorazon = al_load_bitmap("mediocorazon.png");
    if(!spriteMedioCorazon){
        printf("spriteMedioCorazon no se pudo cargar\n");
    }
    spritePalanca_AMARILLA = al_load_bitmap("palanca_amarilla.png");
    if(!spritePalanca_AMARILLA){
        printf("spritePalanca no se pudo cargar\n");
    }
    spritePalanca_VERDE = al_load_bitmap("palanca_verde.png");
    if(!spritePalanca_VERDE){
        printf("spritePalanca no se pudo cargar\n");
    }
    spritePalanca_AZUL = al_load_bitmap("palanca_azul.png");
    if(!spritePalanca_AZUL){
        printf("spritePalanca no se pudo cargar\n");
    }
    spritePalanca_ROJA = al_load_bitmap("palanca_roja.png");
    if(!spritePalanca_ROJA){
        printf("spritePalanca no se pudo cargar\n");
    }
    spriteBalaPJ = al_load_bitmap("balapj.png");
    if(!spriteBalaPJ){
        printf("spriteBalaPJ no se pudo cargar\n");
    }
    spriteBombaNormal = al_load_bitmap("bombanormal.png");
    if(!spriteBombaNormal){
        printf("spriteBombaNormal no se pudo cargar\n");
    }
    spriteBombaActivada = al_load_bitmap("bomba_activa.png");
    if(!spriteBombaActivada){
        printf("spriteBombaActivada no se pudo cargar\n");
    }
    spriteBombaExplotada = al_load_bitmap("bomba_explosion.png");
    if(!spriteBombaExplotada){
        printf("spriteBombaExplotada no se pudo cargar\n");
    }
    spritePinches = al_load_bitmap("pinches.png");
    if(!spritePinches){
        printf("spritePinches no se pudo cargar\n");
    }
    spriteSuper_En = al_load_bitmap("super_En.png");
    if(!spriteSuper_En){
        printf("spriteSuper_En no se pudo cargar\n");
    }
    spriteCoin = al_load_bitmap("coin.png");
    if(!spriteCoin){
        printf("spriteCoin no se pudo cargar\n");
    }
    spriteCaja = al_load_bitmap("caja.png");
    if(!spriteCaja){
        printf("spriteCaja no se pudo cargar\n");
    }
}

void danhoexplosion(char mapa[ALTO][ANCHO], Objeto *o, Juego *j) {
    int f;
    int c;
    int i;
    float rangoPixel;
    float bombaPixelX;
    float bombaPixelY;
    float distanciaX;
    float distanciaY;
    float distanciaEnemigoX;
    float distanciaEnemigoY;

    if (o->activada == true && o->contador > 0) {
        o->contador--;
        if (o->contador == 0) {
            rangoPixel = 50.0 * RADIO_EXPLOSION + 25.0;
            al_draw_filled_circle(o->X, o->Y, rangoPixel, al_map_rgb(255,255,0));
            for (f = o->Y - RADIO_EXPLOSION; f <= o->Y + RADIO_EXPLOSION; f++) {
                for (c = o->X - RADIO_EXPLOSION; c <= o->X + RADIO_EXPLOSION; c++) {
                    if (f >= 0 && f < ALTO && c >= 0 && c < ANCHO) {
                        if (mapa[f][c] == '#') {
                            actualizarPuntaje(&j->player, EVENTO_DESTRUIR_PARED);
                            mapa[f][c] = '.';
                        }    
                        if (f == o->Y && c == o->X) {
                            mapa[f][c] = 'o';
                        }
                    }
                }
            }
            bombaPixelX = o->X * 50.0 + 25.0;
            bombaPixelY = o->Y * 50.0 + 25.0;
            distanciaX = j->player.X - bombaPixelX;
            distanciaY = j->player.Y - bombaPixelY;

            if (distanciaX >= -50 && distanciaX <= 50 && distanciaY >= -50 && distanciaY <= 50) {
                j->player.vida -= 3;
            }
            for (i = 0; i < MAX_ENEMIGOS; i++) {

                if (j->listaEnemigos[i].activo == true) { 
            
                    distanciaEnemigoX = j->listaEnemigos[i].X - bombaPixelX;
                    distanciaEnemigoY = j->listaEnemigos[i].Y - bombaPixelY;            
                    if (distanciaEnemigoX >= -rangoPixel && distanciaEnemigoX <= rangoPixel && distanciaEnemigoY >= -rangoPixel && distanciaEnemigoY <= rangoPixel) {
                        actualizarPuntaje(&j->player, EVENTO_MATAR_ENEMIGO);

                        j->listaEnemigos[i].activo = false;
                    }
                }
            }
            o->activada = false;
        }
    }
}

void danhopinches(char mapa[ALTO][ANCHO], Juego *j) {
    int colJugador;
    int filaJugador;

    if (j->player.cooldownPinches > 0) {
        j->player.cooldownPinches--;
    }
    filaJugador = ((j->player.Y + 25) / 50.0);
    colJugador  = ((j->player.X + 25) / 50.0);

    if (filaJugador >= 0 && filaJugador < ALTO && colJugador >= 0 && colJugador < ANCHO) {
        if (mapa[filaJugador][colJugador] == 'p') {  
      
            if (j->player.cooldownPinches == 0) {
                j->player.vida -= 1;
                actualizarPuntaje(&j->player, EVENTO_RECIBIR_DANIO);
                j->player.cooldownPinches = 60;
            }
        }
    }
}

void asignarComportamientos(Juego *j) {
    int i;
    int azar;
    int contPerseguir = 0;
    int contVertical = 0;
    int contHorizontal = 0;
    int contFantasma = 0;
    bool asignado;
    
    for (i = 0; i < MAX_ENEMIGOS; i++) {
        asignado = false;
        while (!asignado) {
            azar = rand() % 4;
            
            if (azar == 0 && contPerseguir < 4) { 
                j->listaEnemigos[i].tipo = 0; 
                contPerseguir++; 
                asignado = true; 
            }
            else if (azar == 1 && contFantasma < 2) { 
                j->listaEnemigos[i].tipo = 1; 
                contFantasma++; 
                asignado = true; 
            }
            else if (azar == 2 && contVertical < 3) { 
                j->listaEnemigos[i].tipo = 2; 
                j->listaEnemigos[i].sentido = 1; 
                contVertical++; 
                asignado = true; 
            }
            else if (azar == 3 && contHorizontal < 3) { 
                j->listaEnemigos[i].tipo = 3; 
                j->listaEnemigos[i].sentido = 1; 
                contHorizontal++; 
                asignado = true; 
            }
        }
    }
}

bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala, Juego *j) {
    int i;
    int col;
    int fil;
    char celda;
    float margenIzq;
    float margenDer;
    float margenArriba;
    float margenAbajo;
    float tamañoImagen;
    
    if (esBala) {
        col = (x / 50.0); 
        fil = (y / 50.0);
        if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) {
            return true;
        }
        
        celda = mapa[fil][col];
        if (celda == '#' || celda == '*' || (celda == 'C' && !j->cofreAbierto)) {
            return true;
        }
        return false;
    } 
    else {
        margenIzq = 8.0;
        margenDer = 8.0;
        margenArriba = 5.0;
        margenAbajo = 2.0;
        tamañoImagen = 48.0;

        float px[4] = {x + margenIzq, x + tamañoImagen - margenDer, x + margenIzq, x + tamañoImagen - margenDer};
        float py[4] = {y + margenArriba, y + margenArriba, y + tamañoImagen - margenAbajo, y + tamañoImagen - margenAbajo};

        for(i = 0; i < 4; i++) {
            col = (px[i] / 50.0);
            fil = (py[i] / 50.0);

            if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) 
                return true;

            celda = mapa[fil][col];
            
            if (celda == '#' || celda == '*' || (celda == 'C' && !j->cofreAbierto)) {
                return true;
            }
        }
    }
    return false;
}

void danhoEnemigos(Juego *j) {
    int e;
    bool hayColision;

    if (j->player.cooldownPinches > 0) {
        j->player.cooldownPinches--;
    }
    for (e = 0; e < MAX_ENEMIGOS; e++) {
        if (j->listaEnemigos[e].activo) {
            
            hayColision = (j->player.X < j->listaEnemigos[e].X + 50 && j->player.X + 50 > j->listaEnemigos[e].X && j->player.Y < j->listaEnemigos[e].Y + 50 && j->player.Y + 50 > j->listaEnemigos[e].Y);

            if (hayColision && j->player.cooldownPinches == 0) {
                j->player.vida -= 1;
                actualizarPuntaje(&j->player, EVENTO_RECIBIR_DANIO);
                j->player.cooldownPinches = 90;                
                break; 
            }
        }
    }
}

void dibujarInterfaz(personaje *p, int balas, ALLEGRO_FONT *fuente) {
    int i;
    int inicioX = 150;
    int inicioY = 10;

    for (i = 0; i < MAX_OBJ; i++) {
        int posX = inicioX + (i * 60);
        al_draw_filled_rectangle(posX, inicioY, posX + 50, inicioY + 50, al_map_rgb(50, 50, 50));
        al_draw_rectangle(posX, inicioY, posX + 50, inicioY + 50, al_map_rgb(200, 200, 200), 2);
    }
    for (i = 0; i < MAX_OBJ; i++) {
        if (strcasecmp(p->bag.items[i].nombre, "Llave") == 0) {
            al_draw_bitmap(spriteLlave, inicioX + 5, inicioY + 5, 0);
            al_draw_textf(fuente, al_map_rgb(255, 255, 255), inicioX + 5, inicioY + 55, 0, "Llave");
        }
        if (strcasecmp(p->bag.items[i].nombre, "Bomba") == 0) {
            al_draw_bitmap(spriteBombaNormal, inicioX + 115, inicioY + 5, 0);
            al_draw_textf(fuente, al_map_rgb(255, 255, 255), inicioX + 125, inicioY + 55, 0, "Bomba");
        }
    }
    al_draw_textf(fuente, al_map_rgb(255, 255, 255), inicioX + 425, inicioY + 15, 0, "Balas: %d", balas);
    al_draw_textf(fuente, al_map_rgb(255, 255, 255), inicioX + 425, inicioY + 35, 0, "Puntaje: %d", p->puntos);
}

int buscarSlotLibre(personaje *p) {
    int i;
    for (i = 0; i < MAX_OBJ; i++) {
        if (p->bag.items[i].nombre[0] == '\0') 
        {
            return i;
        }
    }
    return -1;
}

void limpiarInventario(personaje *p) {
    int i;
    for (i = 0; i < MAX_OBJ; i++) {
        p->bag.items[i].nombre[0] = '\0';
    }
}

bool tieneObjeto(personaje *p, char *nombreObjeto) {
    int i;
    for (i = 0; i < MAX_OBJ; i++) {
        if (strcmp(p->bag.items[i].nombre, nombreObjeto) == 0) {
            return true;
        }
    }
    return false;
}
void reiniciarJuego(char mapa[ALTO][ANCHO], Juego *j) {
    fflush(stdout);

    j->player.vida = 6;
    j->player.balas = 6;
    j->player.rangoDisparo = RANGO_DISPARO;

    j->nivelActual = 1;
    j->tieneLlave = false;
    j->cofreAbierto = false;
    j->portalActivo = false;
    limpiarInventario(&j->player);
    cargarNivelActual(mapa, j);
}

bool validarRequisitosPortal(Juego *j) {
    for (int i = 0; i < NUM_PALANCAS; i++) {
        if (j->palancasRequeridas[i] == 1 && j->palancasActivadas[i] == 0) {
            return false;
        }
    }
    return true;
}

void avanzarSiguienteNivel(char mapa[ALTO][ANCHO], Juego *j) {
    j->nivelActual++;

    if (j->nivelActual > TOTAL_NIVELES) {
        printf("¡Felicidades! Has completado todos los niveles.\n");
        j->estadoActual = ESTADO_WIN;
    } 
    else {
        printf("Cargando el nivel %d...\n", j->nivelActual);
        actualizarPuntaje(&j->player, EVENTO_PASAR_NIVEL);
        cargarNivelActual(mapa, j);
        limpiarInventario(&j->player);
    }
}

void cargarNivelActual(char mapa[ALTO][ANCHO], Juego *j) {
    int i, col;
    int indice = 0;
    char nombreArchivo[32];
    char lineaBuffer[512];
    
    snprintf(nombreArchivo, sizeof(nombreArchivo), "mapa%d.txt", j->nivelActual);

    FILE *f = fopen(nombreArchivo, "r");
    if (!f) { 
        return; 
    }

    if (fgets(lineaBuffer, sizeof(lineaBuffer), f) != NULL) {
        sscanf(lineaBuffer, "%d %d %d %d", &j->palancasRequeridas[0], &j->palancasRequeridas[1], &j->palancasRequeridas[2], &j->palancasRequeridas[3]);
    }

    if (fgets(j->pistaNivel, sizeof(j->pistaNivel), f) != NULL) {
        size_t len = strlen(j->pistaNivel);
        if (len > 0 && j->pistaNivel[len - 1] == '\n') {
            j->pistaNivel[len - 1] = '\0';
        }
        if (len > 1 && j->pistaNivel[len - 2] == '\r') {
            j->pistaNivel[len - 2] = '\0';
        }
    }

    for (int p = 0; p < NUM_PALANCAS; p++) 
        j->palancasActivadas[p] = 0;
    for (i = 0; i < MAX_ENEMIGOS; i++) 
        j->listaEnemigos[i].activo = false;
    for (i = 0; i < MAX_BALAS_PJ; i++) 
        j->player.listaBalas[i].activa = false;

    for (i = 0; i < ALTO; i++) {
        if (fgets(lineaBuffer, sizeof(lineaBuffer), f) == NULL) {
            break;
        }

        for (col = 0; col < ANCHO; col++) {
            mapa[i][col] = lineaBuffer[col];

            if (mapa[i][col] == '1' && j->palancasRequeridas[PALANCA_AZUL] == 0)
                 mapa[i][col] = '.';
            if (mapa[i][col] == '2' && j->palancasRequeridas[PALANCA_VERDE] == 0)
                mapa[i][col] = '.';
            if (mapa[i][col] == '3' && j->palancasRequeridas[PALANCA_ROJA] == 0)
                mapa[i][col] = '.';
            if (mapa[i][col] == '4' && j->palancasRequeridas[PALANCA_AMARILLA] == 0)
                mapa[i][col] = '.';

            if (mapa[i][col] == '@') { 
                j->player.X = col * TAMANO_CUADRADO; 
                j->player.Y = i * TAMANO_CUADRADO; 
                mapa[i][col] = '.'; 
            }

            if (mapa[i][col] == 'E' || mapa[i][col] == 'F' || mapa[i][col] == 'I' || mapa[i][col] == 'A') {
                if (indice < MAX_ENEMIGOS) {
                    j->listaEnemigos[indice].X = col * TAMANO_CUADRADO;
                    j->listaEnemigos[indice].Y = i * TAMANO_CUADRADO;
                    j->listaEnemigos[indice].velocidad = 3.0;
                    j->listaEnemigos[indice].activo = true;

                    if (mapa[i][col] == 'E') j->listaEnemigos[indice].tipo = 0;
                    else if (mapa[i][col] == 'F') {
                        j->listaEnemigos[indice].tipo = 1;
                        j->listaEnemigos[indice].visible = 1;
                        j->listaEnemigos[indice].ultimoCambioVisibilidad = 0;
                    } 
                    else if (mapa[i][col] == 'I') j->listaEnemigos[indice].tipo = 2;
                    else if (mapa[i][col] == 'A') j->listaEnemigos[indice].tipo = 3;

                    indice++;
                    mapa[i][col] = '.';
                }
            }
        }
    }

    fclose(f);

    j->portalActivo = false;
    j->cofreAbierto = false;
    j->tieneLlave = false;
    j->estadoActual = ESTADO_PAUSA; 
}