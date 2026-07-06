

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#define ANCHO_PANTALLA 1900
#define ALTO_PANTALLA 1150
#define TAMANO_CUADRADO 50

#define VELOCIDAD 6
#define ALTO 23
#define ANCHO 38
#define VIDA_MAXIMA 6
#define MAX_ENEMIGOS 15
#define VELOCIDAD_ENEMIGO 1.5

typedef struct {
    char nombre[10];
    int consumir;
} item;
typedef struct {
    float X, Y;
    bool activa;
} municion;
typedef struct {
    float X, Y;
    float velX, velY;
    bool activa;
} BalaDisparada;

BalaDisparada misBalas[10];

typedef struct {
    item items[10];
} mochila;

typedef struct {
    char nombre[10];
    int vida;
    int balas;
    float X, Y;
    float velocidad;
    mochila bag;
    int direccion;
} personaje;

typedef struct {
    float X, Y;
    int tipo;
    float velocidad;
    bool activo;
} enemigos;

enemigos listaEnemigos[15];
municion listaBalas[20];

// Variables globales
personaje jugador;
enemigos enemigo;
ALLEGRO_BITMAP *spriteJugador = NULL;
ALLEGRO_BITMAP *spriteEnemigo = NULL;
ALLEGRO_BITMAP *spritePared = NULL;
ALLEGRO_BITMAP *spritefondo = NULL;

void cargarMapa(char mapa[ALTO][ANCHO]);
void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO]);
void dibujarVida(int vida);
void reiniciarJuego(char mapa[ALTO][ANCHO]);
void cargarAssets();
void moverEnemigos(char mapa[ALTO][ANCHO]);

int main() {
    srand(time(NULL));
    if (!al_init()) return -1;
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();

    cargarAssets();
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    ALLEGRO_DISPLAY *display = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    
    bool salir = false;
    static bool espacioPresionado = false;
    float tiempoUltimoDaño = 0;
    jugador.vida = VIDA_MAXIMA;
    jugador.balas = 9;

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    char mapa[ALTO][ANCHO];
    cargarMapa(mapa);
    int contador = 0;
for(int i = 0; i < 20; i++) {
    if(listaBalas[i].activa) contador++;
}
    al_start_timer(timer);

    printf("Balas iniciales: %d\n", jugador.balas);

    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) salir = true;

        if (evento.type == ALLEGRO_EVENT_TIMER) {
            ALLEGRO_KEYBOARD_STATE teclado;
            al_get_keyboard_state(&teclado);
            if (al_key_down(&teclado, ALLEGRO_KEY_ESCAPE)) salir = true;

            float nuevaX = jugador.X;
            float nuevaY = jugador.Y;

            if (al_key_down(&teclado, ALLEGRO_KEY_W)) { 
                nuevaY -= VELOCIDAD; 
                jugador.direccion = 3; 
            }
            if (al_key_down(&teclado, ALLEGRO_KEY_S)) { 
                nuevaY += VELOCIDAD; 
                jugador.direccion = 0; 
            }
            if (al_key_down(&teclado, ALLEGRO_KEY_A)) { 
                nuevaX -= VELOCIDAD; 
                jugador.direccion = 1; 
            }
            if (al_key_down(&teclado, ALLEGRO_KEY_D)) { 
                nuevaX += VELOCIDAD; 
                jugador.direccion = 2; 
            }
            for (int i = 0; i < 10; i++) {
        if (misBalas[i].activa) {
            misBalas[i].X += misBalas[i].velX;
            misBalas[i].Y += misBalas[i].velY;

            if (misBalas[i].X < 0 || misBalas[i].X > ANCHO_PANTALLA || misBalas[i].Y < 0 || misBalas[i].Y > ALTO_PANTALLA) {
                misBalas[i].activa = false;
            }
            for (int j = 0; j < MAX_ENEMIGOS; j++) {
            if (listaEnemigos[j].activo) {
                float centroEnemigoX = listaEnemigos[j].X + 25;
                float centroEnemigoY = listaEnemigos[j].Y + 25;

                if (abs(misBalas[i].X - centroEnemigoX) < 30 && 
                    abs(misBalas[i].Y - centroEnemigoY) < 30) {
                    
                    listaEnemigos[j].activo = false;
                    misBalas[i].activa = false;      
                }
            }
        }
        }
    }
    

            colision(mapa, &jugador.X, &jugador.Y, nuevaX, nuevaY);
            moverEnemigos(mapa);

            for (int i = 0; i < 15; i++) {
                if (listaEnemigos[i].activo) {
                    float dist = sqrt(pow((jugador.X+25)-(listaEnemigos[i].X+25), 2) + pow((jugador.Y+25)-(listaEnemigos[i].Y+25), 2));
                    if (dist < 40 && al_get_time() - tiempoUltimoDaño > 1.0) {
                        jugador.vida--;
                        tiempoUltimoDaño = al_get_time();
                    }
                }
            }
            for (int i = 0; i < 20; i++) {
                if (listaBalas[i].activa) {
                    float dist = sqrt(pow(jugador.X - listaBalas[i].X, 2) + pow(jugador.Y - listaBalas[i].Y, 2));
                    if (dist < 30) { 
                        jugador.balas += 3; 
                        listaBalas[i].activa = false; 
                    }
                }
            }
            if (jugador.vida <= 0) reiniciarJuego(mapa);
        }

        if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_SPACE && !espacioPresionado) {
    espacioPresionado = true;
    if (jugador.balas > 0) {
        jugador.balas--;
        for(int i = 0; i < 10; i++) {
            if(!misBalas[i].activa) {
                misBalas[i].X = jugador.X + 40;
                misBalas[i].Y = jugador.Y + 40;
                misBalas[i].activa = true;
                misBalas[i].velX = (jugador.direccion == 2) ? 10 : (jugador.direccion == 1) ? -10 : 0;
                misBalas[i].velY = (jugador.direccion == 0) ? 10 : (jugador.direccion == 3) ? -10 : 0;
                break;
            }
        }
    }
}
        if (evento.type == ALLEGRO_EVENT_KEY_UP && evento.keyboard.keycode == ALLEGRO_KEY_SPACE) 
        espacioPresionado = false;

        al_clear_to_color(al_map_rgb(0, 0, 0));
        if (spritefondo) al_draw_bitmap(spritefondo, 0, 0, 0);
        dibujarMapa(mapa);
        
        
        
        
        if (spriteJugador) 
        al_draw_scaled_bitmap(spriteJugador, 0, 0, 50, 50, jugador.X, jugador.Y, 50, 50, 0);
        
        for (int i = 0; i < 15; i++) 
            if (listaEnemigos[i].activo) 
            al_draw_scaled_bitmap(spriteEnemigo, 0, 0, 50, 50, listaEnemigos[i].X, listaEnemigos[i].Y, 50, 50, 0);
        
        dibujarVida(jugador.vida);
        for (int i = 0; i < 20; i++) {
    if (listaBalas[i].activa) {
        al_draw_filled_circle(listaBalas[i].X + 25, listaBalas[i].Y + 25, 10, al_map_rgb(0, 0, 0));
    }
}
        for (int i = 0; i < 10; i++) {
    if (misBalas[i].activa) {
        al_draw_filled_circle(misBalas[i].X, misBalas[i].Y, 5, al_map_rgb(0, 0, 0));
    }
}
        
        al_flip_display();
    }

    al_destroy_bitmap(spriteJugador);
    al_destroy_bitmap(spriteEnemigo);
    al_destroy_bitmap(spritePared);
    al_destroy_bitmap(spritefondo);
    al_destroy_display(display);
    return 0;
}
//INGRESAR COMO PARAMETROS LAS VARIABLES DE LOS PERSONAJES
void cargarMapa(char mapa[ALTO][ANCHO]) {
    int indice = 0;
    int k = 0;
    FILE *f = fopen("mapa2 copy.txt", "r");
    if (!f) return;
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            fscanf(f, " %c", &mapa[i][j]);
            if (mapa[i][j] == '@') {
                jugador.X = j * TAMANO_CUADRADO;
                jugador.Y = i * TAMANO_CUADRADO; 
                mapa[i][j] = '.';
            }
            if (mapa[i][j] == 'E' && indice < 15) {
            listaEnemigos[indice].X = j * TAMANO_CUADRADO;
            listaEnemigos[indice].Y = i * TAMANO_CUADRADO;
            listaEnemigos[indice].velocidad = 3.0;
            listaEnemigos[indice].activo = true;
            indice++;
            mapa[i][j] = '.';
            }
            if (mapa[i][j] == 'B') {
            listaBalas[k].X = j * TAMANO_CUADRADO;
            listaBalas[k].Y = i * TAMANO_CUADRADO;
            listaBalas[k].activa = true;
            k++;
            mapa[i][j] = '.';
            }
        }
    }
    fclose(f);
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
bool esColision(char mapa[ALTO][ANCHO], float x, float y) {
    float size = 40.0; 
    float puntosX[] = {x, x + size, x, x + size};
    float puntosY[] = {y, y, y + size, y + size};

    for(int i = 0; i < 4; i++) {
        int col = (int)(puntosX[i] / TAMANO_CUADRADO);
        int fil = (int)(puntosY[i] / TAMANO_CUADRADO);
        
        if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) 
        return true;
        if (mapa[fil][col] == '#') 
        return true;
    }
    return false;
}
void dibujarMapa(char mapa[ALTO][ANCHO]) {
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            if (mapa[i][j] == '#') {
                al_draw_bitmap(spritePared, j*TAMANO_CUADRADO, i*TAMANO_CUADRADO, 0);
            }
            if (mapa[i][j] == '.') {
                al_draw_bitmap(spritefondo, j*TAMANO_CUADRADO, i*TAMANO_CUADRADO, 0);
            }
        }
    }
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

void reiniciarJuego(char mapa[ALTO][ANCHO]) {
    jugador.vida = VIDA_MAXIMA;
    jugador.balas = 9;
    cargarMapa(mapa);
    al_rest(0.5); 
}
void cargarAssets() {
    spriteJugador = al_load_bitmap("jugador.png");
    al_convert_mask_to_alpha(spriteJugador, al_map_rgb(255, 255, 255));

    spriteEnemigo = al_load_bitmap("enemigo.png");
    al_convert_mask_to_alpha(spriteEnemigo, al_map_rgb(255, 255, 255));

    spritePared = al_load_bitmap("pared.png");
    spritefondo = al_load_bitmap("fondo.png");
}
void moverEnemigos(char mapa[ALTO][ANCHO]) {
    float proxX;
    float proxY;
    float RANGO_DETECCION = 800.0;

    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!listaEnemigos[i].activo) 
            continue;

        float distX = jugador.X - listaEnemigos[i].X;
        float distY = jugador.Y - listaEnemigos[i].Y;
        float distanciaJugador = sqrt(distX * distX + distY * distY);
        if (listaEnemigos[i].X < jugador.X) proxX += VELOCIDAD_ENEMIGO;
else if (listaEnemigos[i].X > jugador.X) proxX -= VELOCIDAD_ENEMIGO;

if (listaEnemigos[i].Y < jugador.Y) proxY += VELOCIDAD_ENEMIGO;
else if (listaEnemigos[i].Y > jugador.Y) proxY -= VELOCIDAD_ENEMIGO;

        if (distanciaJugador < RANGO_DETECCION) {
            float proxX = listaEnemigos[i].X;
            float proxY = listaEnemigos[i].Y;

            if (listaEnemigos[i].X < jugador.X) proxX += listaEnemigos[i].velocidad;
            else if (listaEnemigos[i].X > jugador.X) proxX -= listaEnemigos[i].velocidad;

            if (listaEnemigos[i].Y < jugador.Y) proxY += listaEnemigos[i].velocidad;
            else if (listaEnemigos[i].Y > jugador.Y) proxY -= listaEnemigos[i].velocidad;

            for (int j = 0; j < MAX_ENEMIGOS; j++) {
                if (i == j || !listaEnemigos[j].activo) continue;

                float dx = listaEnemigos[i].X - listaEnemigos[j].X;
                float dy = listaEnemigos[i].Y - listaEnemigos[j].Y;
                float dist = sqrt(dx*dx + dy*dy);

                if (dist < 40) {
                    proxX += (dx / dist) * 1.0;
                    proxY += (dy / dist) * 1.0;
                }
            }

            if (!esColision(mapa, proxX, listaEnemigos[i].Y)) 
                listaEnemigos[i].X = proxX;
            if (!esColision(mapa, listaEnemigos[i].X, proxY)) 
                listaEnemigos[i].Y = proxY;
        }
    }
}