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
struct item_
{
    char nombre[10];
    int consumir;

};
typedef struct item_ item;

struct mochila_
{
    item items[10];
};
typedef struct mochila_ mochila;

struct personaje_
{
    char nombre[10];
    int vida;
    float X;
    float Y;
    float velocidad;
    mochila bag;
    float w;
    float h;
};
typedef struct personaje_ personaje;


void cargarMapa(char mapa[ALTO][ANCHO]);
void leerMapa(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO]);


int main() {

    float posX = TAMANO_CUADRADO; 
    float posY = TAMANO_CUADRADO;

    bool tecla_arriba = false, tecla_abajo = false, tecla_izq = false, tecla_der = false;
    bool salir = true;

    char mapa[ALTO][ANCHO];
    cargarMapa(mapa,&posX,&posY);

    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            putchar(mapa[i][j]);
        }
        putchar('\n');
    }

    if (!al_init()) return -1;
    al_install_keyboard();
    al_init_primitives_addon();

    al_set_new_display_option(ALLEGRO_OPENGL_MAJOR_VERSION, 2, ALLEGRO_REQUIRE);
    al_set_new_display_option(ALLEGRO_OPENGL_MINOR_VERSION, 0, ALLEGRO_REQUIRE);
    al_set_new_display_flags(ALLEGRO_WINDOWED);

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

    while (salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) salir = false;

        if (evento.type == ALLEGRO_EVENT_TIMER) {
            float nuevaX = posX;
            float nuevaY = posY;

            if (tecla_arriba) nuevaY -= VELOCIDAD;
            if (tecla_abajo)  nuevaY += VELOCIDAD;
            if (tecla_izq)    nuevaX -= VELOCIDAD;
            if (tecla_der)    nuevaX += VELOCIDAD;

            leerMapa(mapa, &posX, &posY, nuevaX, nuevaY);

            if (posX < 0) posX = 0;
            if (posY < 0) posY = 0;
            if (posX > ANCHO_PANTALLA - TAMANO_CUADRADO) posX = ANCHO_PANTALLA - TAMANO_CUADRADO;
            if (posY > ALTO_PANTALLA - TAMANO_CUADRADO) posY = ALTO_PANTALLA - TAMANO_CUADRADO;

            al_clear_to_color(al_map_rgb(10, 10, 10));
            dibujarMapa(mapa);
            al_draw_filled_rectangle(posX, posY, posX + TAMANO_CUADRADO, posY + TAMANO_CUADRADO, al_map_rgb(255, 0, 0));
            al_flip_display();
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (evento.keyboard.keycode) {
                case ALLEGRO_KEY_UP:    tecla_arriba = true; break;
                case ALLEGRO_KEY_DOWN:  tecla_abajo = true;  break;
                case ALLEGRO_KEY_LEFT:  tecla_izq = true;    break;
                case ALLEGRO_KEY_RIGHT: tecla_der = true;    break;
                case ALLEGRO_KEY_ESCAPE: salir = false;  break;
            }
        }
        else if (evento.type == ALLEGRO_EVENT_KEY_UP) {
            switch (evento.keyboard.keycode) {
                case ALLEGRO_KEY_UP:    tecla_arriba = false; break;
                case ALLEGRO_KEY_DOWN:  tecla_abajo = false;  break;
                case ALLEGRO_KEY_LEFT:  tecla_izq = false;    break;
                case ALLEGRO_KEY_RIGHT: tecla_der = false;    break;
            }
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}

void cargarMapa(char mapa[ALTO][ANCHO]) 
{
    FILE *archivofop = fopen("mapa2 copy.txt", "r");
    int i,j;
    if (!archivofop) {
        fprintf(stderr, "Error al abrir el archivo del mapa.\n");
        for (int i = 0; i < ALTO; i++)
            for (int j = 0; j < ANCHO; j++)
                mapa[i][j] = '.';
        return;
    }

    for (i = 0; i < ALTO; i++) {
        for (j = 0; j < ANCHO; j++) {
            fscanf(archivofop," %c",&mapa[i][j]);
        }
    }

    fclose(archivofop);
    return;
}


void leerMapa(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY)
{
    float size = TAMANO_CUADRADO;

   
    if (nuevaX != *posX) {
        float testX = nuevaX;
        float testY = *posY;

        float cornersX[2] = { testX, testX + size - 1 };
        float cornersY[2] = { testY, testY + size - 1 };

        bool colisionH = false;
        for (int k = 0; k < 2 && !colisionH; k++) {
            int cellX = (int)(cornersX[k] / TAMANO_CUADRADO);
            int cellY = (int)(cornersY[0] / TAMANO_CUADRADO);
            if (cellX < 0 || cellX >= ANCHO || cellY < 0 || cellY >= ALTO) 
            {
                colisionH = true;
                break;
            }
            if (mapa[cellY][cellX] == '#') { colisionH = true; break; }

            cellY = (int)(cornersY[1] / TAMANO_CUADRADO);
            if (cellY < 0 || cellY >= ALTO)
             { colisionH = true;
             break; 
            }
            if (mapa[cellY][cellX] == '#') 
            { 
                colisionH = true;
                break;
            }
        }

        if (!colisionH) {
            *posX = testX;
        }
    }

    if (nuevaY != *posY) {
        float testX = *posX;
        float testY = nuevaY;

        float cornersX[2] = { testX, testX + size - 1 };
        float cornersY[2] = { testY, testY + size - 1 };

        bool colisionV = false;
        for (int k = 0; k < 2 && !colisionV; k++) {
            int cellX = (int)(cornersX[0] / TAMANO_CUADRADO);
            int cellY = (int)(cornersY[k] / TAMANO_CUADRADO);
            if (cellX < 0 || cellX >= ANCHO || cellY < 0 || cellY >= ALTO) { colisionV = true; break; }
            if (mapa[cellY][cellX] == '#') { colisionV = true; break; }

            cellX = (int)(cornersX[1] / TAMANO_CUADRADO);
            if (cellX < 0 || cellX >= ANCHO) { colisionV = true; break; }
            if (mapa[cellY][cellX] == '#') { colisionV = true; break; }
        }

        if (!colisionV) {
            *posY = testY;
        }
    }
}


void dibujarMapa(char mapa[ALTO][ANCHO])
{
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            if (mapa[i][j] == '#') {
                float x = j * TAMANO_CUADRADO;
                float y = i * TAMANO_CUADRADO;
                al_draw_filled_rectangle(x, y, x + TAMANO_CUADRADO, y + TAMANO_CUADRADO, al_map_rgb(150, 150, 150));
            }
           // if (mapa[i][j] == '@') {

        }
    }
}

