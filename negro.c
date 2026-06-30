#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ANCHO_PANTALLA 1900
#define ALTO_PANTALLA 1150
#define TAMANO_CUADRADO 50
#define VELOCIDAD 8
#define ALTO 23
#define ANCHO 38
#define VIDA_MAXIMA 6

struct item_ {
    char nombre[10];
    int consumir;
};
typedef struct item_ item;

struct mochila_ {
    item items[10];
};
typedef struct mochila_ mochila;

struct personaje_ {
    char nombre[10];
    int vida;
    float X;
    float Y;
    float velocidad;
    mochila bag;
    float w;
    float h;
    float inicioX, inicioY, finX, finY;
    bool yendoAlFin;
};
typedef struct personaje_ personaje;

personaje jugador;
personaje enemigo;

void cargarMapa(char mapa[ALTO][ANCHO]);
void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO]);
void dibujarVida(int vida);
void reiniciarJuego(char mapa[ALTO][ANCHO]);

int main() {
    float posX = TAMANO_CUADRADO; 
    float posY = TAMANO_CUADRADO;
    float tiempoUltimoDaño = 0;
    jugador.vida = VIDA_MAXIMA;

    bool salir = true;

    char mapa[ALTO][ANCHO];
    cargarMapa(mapa);

    if (!al_init()) return -1;
    al_install_keyboard();
    al_init_primitives_addon();

    ALLEGRO_DISPLAY *display = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA);
    if (!display) {
        fprintf(stderr, "Error al crear display.\n");
        return -1;
    }

    al_set_window_title(display, "Juego - Colisiones con mapa");

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    ALLEGRO_KEYBOARD_STATE teclado;

    while (salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) salir = false;
        if (evento.type == ALLEGRO_EVENT_TIMER) {
            float nuevaX = jugador.X;
            float nuevaY = jugador.Y;

            al_get_keyboard_state(&teclado);

            if (al_key_down(&teclado, ALLEGRO_KEY_W)) nuevaY -= VELOCIDAD;
            if (al_key_down(&teclado, ALLEGRO_KEY_S)) nuevaY += VELOCIDAD;
            if (al_key_down(&teclado, ALLEGRO_KEY_A)) nuevaX -= VELOCIDAD;
            if (al_key_down(&teclado, ALLEGRO_KEY_D)) nuevaX += VELOCIDAD;
            if (al_key_down(&teclado, ALLEGRO_KEY_ESCAPE)) salir = false;

            colision(mapa, &jugador.X, &jugador.Y, nuevaX, nuevaY);
            if (jugador.vida <= 0) {
                reiniciarJuego(mapa);
            }

            if (enemigo.yendoAlFin) {
                if (enemigo.X < enemigo.finX) enemigo.X += 2;
                else enemigo.yendoAlFin = false;
            } else {
                if (enemigo.X > enemigo.inicioX) enemigo.X -= 2;
                else enemigo.yendoAlFin = true;
            }
            float distX = (jugador.X + 25) - (enemigo.X + 25);
            float distY = (jugador.Y + 25) - (enemigo.Y + 25);
            float distancia = sqrt(distX * distX + distY * distY);

            if (distancia < 40) {
                float tiempoActual = al_get_time();
                
                if (tiempoActual - tiempoUltimoDaño > 1.0) {
                    if (jugador.vida > 0) {
                        jugador.vida -= 1;
                        tiempoUltimoDaño = tiempoActual;
                    }
                }
            }

            al_clear_to_color(al_map_rgb(10, 10, 10));
            dibujarMapa(mapa);
            al_draw_filled_rectangle(jugador.X, jugador.Y, jugador.X + TAMANO_CUADRADO, jugador.Y + TAMANO_CUADRADO, al_map_rgb(255, 0, 0));
            al_draw_filled_circle(enemigo.X + 25, enemigo.Y + 25, 20, al_map_rgb(0, 255, 0));
            dibujarVida(jugador.vida);
            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}

void cargarMapa(char mapa[ALTO][ANCHO]) {
    FILE *archivofop = fopen("mapa2 copy.txt", "r");
    if (!archivofop) {
        fprintf(stderr, "Error al abrir el archivo del mapa.\n");
        return;
    }

    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            fscanf(archivofop," %c",&mapa[i][j]);
            if (mapa[i][j] == '@') {
                jugador.X = j * TAMANO_CUADRADO;
                jugador.Y = i * TAMANO_CUADRADO;
            }
            if (mapa[i][j] == 'E') {
                enemigo.X = j * TAMANO_CUADRADO;
                enemigo.Y = i * TAMANO_CUADRADO;
                enemigo.inicioX = j * TAMANO_CUADRADO;
                enemigo.finX = (j + 5) * TAMANO_CUADRADO;
                enemigo.yendoAlFin = true;
            }
        }
    }
    fclose(archivofop);
}

void dibujarVida(int vida) {
    for (int i = 0; i < 3; i++) {
        float x = 20 + i * 50;
        float y = 20;
        al_draw_filled_circle(x, y, 15, al_map_rgb(50, 50, 50));
        int estado = vida - (i * 2);
        if (estado >= 2) {
            al_draw_filled_circle(x, y, 10, al_map_rgb(255, 0, 0));
        } else if (estado == 1) {
            al_draw_filled_rectangle(x - 10, y - 10, x, y + 10, al_map_rgb(255, 0, 0));
        }
    }
}

void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY) {
    float size = TAMANO_CUADRADO;

    if (nuevaX != *posX) {
        int direccion = (nuevaX > *posX) ? 1 : -1;
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
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#') colision = true;
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
        int direccion = (nuevaY > *posY) ? 1 : -1;
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
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#') colision = true;
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

void dibujarMapa(char mapa[ALTO][ANCHO]) {
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            if (mapa[i][j] == '#') {
                float x = j * TAMANO_CUADRADO;
                float y = i * TAMANO_CUADRADO;
                al_draw_filled_rectangle(x, y, x + TAMANO_CUADRADO, y + TAMANO_CUADRADO, al_map_rgb(150, 150, 150));
            }
        }
    }
}
void reiniciarJuego(char mapa[ALTO][ANCHO]) {
    jugador.vida = VIDA_MAXIMA;
    cargarMapa(mapa);
}