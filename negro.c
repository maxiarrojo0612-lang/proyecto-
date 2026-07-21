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
#define VELOCIDAD_ENEMIGO 1
#define COOLDOWN_DISPARO_ENEMIGO 2.0
#define MAX_BALAS_EN 20
#define MAX_BALAS_PJ 2
#define RANGO_PERSECUCION 1200.0f
#define RANGO_DISPARO 800.0f
#define COOLDOWN_DISPARO 1.0f

//mochila-items ..
//agregar palanca-bombas-curaciones .
//enemigos que disparen .
//con rango de deteccion para acercarse y para disparar .
//limpiar el main ..
//rotaciones de personaje y enemigos .
//spritebala .
//una letra para cada tipo de enemigo para ubicarlos en el mapa ..
//hacer colisiones devuelta ..

typedef struct {
    char nombre[10];
    int consumir;
}Objeto;
typedef struct {
    float X, Y;
    float velX, velY;
    bool activa;
} municion;

typedef struct {
    Objeto items[10];
} mochila;

typedef struct {
    char nombre[10];
    int vida;
    int balas;
    float X, Y;
    float velocidad;
    mochila bag;
    int direccion;
    municion listaBalas[MAX_BALAS_PJ];
} personaje;

typedef struct {
    float X, Y;
    float velocidad;
    bool activo;
    int tipo;
    int sentido;
    float ultimoDisparo;
    municion listaBalas[MAX_BALAS_EN];
} enemigos;
typedef struct {
    personaje player;
    enemigos listaEnemigos[MAX_ENEMIGOS];
    bool tieneLlave;
    bool cofreAbierto;
    bool portalActivo;
} Juego;
    ALLEGRO_BITMAP *spriteJugador = NULL;
    ALLEGRO_BITMAP *spriteEnemigo = NULL;
    ALLEGRO_BITMAP *spritePared = NULL;
    ALLEGRO_BITMAP *spritefondo = NULL;
    ALLEGRO_BITMAP *spriteLlave = NULL;
    ALLEGRO_BITMAP *spriteCofre = NULL;
    ALLEGRO_BITMAP *spritePortal = NULL;
    ALLEGRO_BITMAP *spriteBala = NULL;
    ALLEGRO_BITMAP *spriteCorazon = NULL;
    ALLEGRO_BITMAP *spriteMedioCorazon = NULL;
    ALLEGRO_BITMAP *spritePalanca = NULL;

void cargarMapa(char mapa[ALTO][ANCHO], Juego *j);
void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO], Juego *j);
void dibujarVida(int vida);
void reiniciarJuego(char mapa[ALTO][ANCHO],personaje *p, ALLEGRO_FONT *fuente, Juego *j);
void cargarAssets();
void moverEnemigos(char mapa[ALTO][ANCHO], Juego *j);
void dispararEnemigo(enemigos *en, float playerX, float playerY);
void actualizarBalasEnemigos(char mapa[ALTO][ANCHO], Juego *j);
bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala, Juego *j);
void cambiarNivel(char mapa[ALTO][ANCHO], const char* nombreArchivo, Juego *j);
void dibujarInterfaz(personaje *p, int balas, ALLEGRO_FONT *fuente);
void asignarComportamientos( Juego *j);
int buscarSlotLibre(personaje *p);
void dibujarInventario(personaje *p, ALLEGRO_FONT *fuente);
bool guardarEnInventario(personaje *p, char *nombre, int consumir);
bool tieneObjeto(personaje *p, char *nombre);

int main() {
    Juego miJuego; 
    miJuego.tieneLlave = false;
    miJuego.cofreAbierto = false;
    miJuego.portalActivo = false;
    bool salir = false;
    bool redibujar = true;

    srand(time(NULL));
    if (!al_init()) {
        return -1;
    }
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    cargarAssets();
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA);

    ALLEGRO_DISPLAY *display = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0); // 60 FPS
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_FONT *miFuente = al_create_builtin_font();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    char mapa[ALTO][ANCHO];
    cargarMapa(mapa,&miJuego);
    asignarComportamientos(&miJuego);
    al_start_timer(timer);

    // Pre-renderizado del fondo
    ALLEGRO_BITMAP *fondoCompleto = al_create_bitmap(ANCHO_PANTALLA, ALTO_PANTALLA);
    al_set_target_bitmap(fondoCompleto);
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            al_draw_bitmap(spritefondo, j * 50, i * 50, 0);
        }
    }
    al_set_target_bitmap(al_get_backbuffer(display));

    miJuego.player.vida = 6;
    miJuego.player.balas = 6;//desvincular la cantidad de balas con el tamaño del arreglo

    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);
        // --- LÓGICA
        if (evento.type == ALLEGRO_EVENT_TIMER) {

            redibujar = true;
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
            moverEnemigos(mapa,&miJuego);
            int col = (int)((miJuego.player.X + 25) / 50.0);
            int fil = (int)((miJuego.player.Y + 25) / 50.0);
            char objeto = mapa[fil][col];
            if(mapa[fil][col] == 'B'){
                miJuego.player.balas += 3;
                mapa[fil][col] = ' ';
            }
            if (mapa[fil][col] == 'K') {
                for (int i = 0; i < 10; i++) {
                    if (miJuego.player.bag.items[i].nombre[0] == '\0') {
                        strcpy(miJuego.player.bag.items[i].nombre, "Llave");
                        mapa[fil][col] = ' '; 
                        printf("¡Recogiste la llave!\n");
                        break;
                    }
                }
            }
            if (mapa[fil][col] == 'C') {
                if (tieneObjeto(&miJuego.player, "Llave")) {
                    printf("¡Cofre abierto! Recibes la palanca.\n");
                    int slotPalanca = buscarSlotLibre(&miJuego.player);
                    if (slotPalanca != -1) {
                        strcpy(miJuego.player.bag.items[slotPalanca].nombre, "Palanca");
                        mapa[fil][col] = ' ';
                    } 
                    else {
                        printf("Inventario lleno, no puedes recoger la palanca del cofre.\n");
                    }
                } 
                else {
                    printf("Necesitas la llave.\n");
                }
            }
            if (mapa[fil][col] == 'P' && !miJuego.portalActivo) {
                if (tieneObjeto(&miJuego.player, "Palanca")) {
                    printf("¡Activando portal!\n");
                    miJuego.portalActivo = true;
                    for (int i = 0; i < 10; i++) {
                        if (strcmp(miJuego.player.bag.items[i].nombre, "Palanca") == 0) {
                            miJuego.player.bag.items[i].nombre[0] = '\0';
                            break;
                        }
                    }
                } 
                else {
                    printf("Necesitas una palanca.\n");
                }
            } 
            else if (mapa[fil][col] == 'P' && miJuego.portalActivo) {
                cambiarNivel(mapa, "mapa3.txt", &miJuego);
                printf("¡El portal está abierto! ¡Pasando de nivel!\n");
            }
            for (int i = 0; i < 12; i++) {
                if (miJuego.listaEnemigos[i].activo) {
                    float dx = miJuego.player.X - miJuego.listaEnemigos[i].X;
                    float dy = miJuego.player.Y - miJuego.listaEnemigos[i].Y;
                    float distancia = sqrt(dx*dx + dy*dy);

                    if (distancia < 30) {
                        static int framesColision = 0;
                        framesColision++;
                        if (framesColision > 30) {
                            miJuego.player.vida--;
                            framesColision = 0;
                
                            if (miJuego.player.vida <= 0) {
                                printf("Game Over\n");
                                reiniciarJuego(mapa, &miJuego.player, miFuente, &miJuego);
                            }
                        }
                    }
                }
            }if (miJuego.player.vida <= 0) {
                printf("Game Over\n");
                reiniciarJuego(mapa, &miJuego.player, miFuente, &miJuego);
            }

            // Mover y colisionar balas
            for (int i = 0; i < 10; i++) {
                if (miJuego.player.listaBalas[i].activa) {
                    miJuego.player.listaBalas[i].X +=miJuego.player.listaBalas[i].velX;
                    miJuego.player.listaBalas[i].Y += miJuego.player.listaBalas[i].velY;
                    if (esColision(mapa, miJuego.player.listaBalas[i].X, miJuego.player.listaBalas[i].Y, true, &miJuego)) miJuego.player.listaBalas[i].activa = false;
                    else {
                        for (int j = 0; j < MAX_ENEMIGOS; j++) {
                            if (miJuego.listaEnemigos[j].activo) {
                                float dx = miJuego.player.listaBalas[i].X - (miJuego.listaEnemigos[j].X + 25);
                                float dy = miJuego.player.listaBalas[i].Y - (miJuego.listaEnemigos[j].Y + 25);
                                if ((dx*dx + dy*dy) < 900) {
                                    miJuego.listaEnemigos[j].activo = false;
                                    miJuego.player.listaBalas[i].activa = false;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE || evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
            salir = true;
        }

        //DISPARO
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_SPACE) {
            if (miJuego.player.balas > 0) {
                for(int i = 0; i < MAX_BALAS_PJ; i++) {
                    if(!miJuego.player.listaBalas[i].activa) {
                        miJuego.player.listaBalas[i].X = miJuego.player.X + 25; 
                        miJuego.player.listaBalas[i].Y = miJuego.player.Y + 25;
                        miJuego.player.listaBalas[i].activa = true;
                        miJuego.player.listaBalas[i].velX = (miJuego.player.direccion == 2) ? 10 : (miJuego.player.direccion == 1) ? -10 : 0;
                        miJuego.player.listaBalas[i].velY = (miJuego.player.direccion == 0) ? 10 : (miJuego.player.direccion == 3) ? -10 : 0;
                        miJuego.player.balas--; 
                        break;
                    }
                }
            }
        }

        //DIBUJO
        if (redibujar && al_is_event_queue_empty(queue)) {
            redibujar = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(fondoCompleto, 0, 0, 0);
            dibujarMapa(mapa,&miJuego);
            actualizarBalasEnemigos(mapa,&miJuego);
            dibujarInterfaz(&miJuego.player, miJuego.player.balas, miFuente);           

            if (spriteJugador){
                al_draw_scaled_bitmap(spriteJugador, 0, 0, 50, 50, miJuego.player.X, miJuego.player.Y, 50, 50, 0);
            } 
            for (int i = 0; i < MAX_ENEMIGOS; i++) {
                if (miJuego.listaEnemigos[i].activo) {
                al_draw_scaled_bitmap(spriteEnemigo, 0, 0, 50, 50, miJuego.listaEnemigos[i].X, miJuego.listaEnemigos[i].Y, 50, 50, 0);
                for (int b = 0; b < MAX_BALAS_EN; b++) {
                if (miJuego.listaEnemigos[i].listaBalas[b].activa) {
                    al_draw_filled_circle(miJuego.listaEnemigos[i].listaBalas[b].X, miJuego.listaEnemigos[i].listaBalas[b].Y, 5, al_map_rgb(255, 255, 255));
                }
                }
                }
            }
            for (int i = 0; i < 10; i++) {
                if (miJuego.player.listaBalas[i].activa) {
                    al_draw_filled_circle(miJuego.player.listaBalas[i].X, miJuego.player.listaBalas[i].Y, 5, al_map_rgb(135, 206, 235));
                }
            }
            dibujarVida(miJuego.player.vida);
            
            al_flip_display();
        }

        
    }
    al_destroy_bitmap(fondoCompleto);
    al_destroy_bitmap(spriteJugador);
    al_destroy_bitmap(spriteEnemigo);
    al_destroy_bitmap(spritePared);
    al_destroy_bitmap(spritefondo);
    al_destroy_display(display);
    al_destroy_font(miFuente);

    return 0;

}

//INGRESAR COMO PARAMETROS LAS VARIABLES DE LOS PERSONAJES
void cargarMapa(char mapa[ALTO][ANCHO], Juego *j) {
    int indice = 0;
    FILE *f = fopen("mapa2 copy.txt", "r");
    if (!f) return;
    for (int i = 0; i < ALTO; i++) {
        for (int col = 0; col < ANCHO; col++) {
            fscanf(f, " %c", &mapa[i][col]);
            if (mapa[i][col] == '@') {
               j->player.X = col * TAMANO_CUADRADO;
               j->player.Y = i * TAMANO_CUADRADO;
            }
            if (mapa[i][col] == 'E' && indice < MAX_ENEMIGOS) {
            j->listaEnemigos[indice].X = col * TAMANO_CUADRADO;
            j->listaEnemigos[indice].Y = i * TAMANO_CUADRADO;
            j->listaEnemigos[indice].velocidad = 3.0;
            j->listaEnemigos[indice].activo = true;
            indice++;
            }
        }
    }
    fclose(f);
}

void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY) {      //VER

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
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#') {
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
                    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO || mapa[y][x] == '#') {
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
void dispararEnemigo(enemigos *en, float playerX, float playerY) {
    for(int b = 0; b < MAX_BALAS_EN; b++) {
        if(!en->listaBalas[b].activa) {
            en->listaBalas[b].activa = true;
            en->listaBalas[b].X = en->X + 25;
            en->listaBalas[b].Y = en->Y + 25;

            float dx = playerX - en->X;
            float dy = playerY - en->Y;
            float dist = sqrt(dx*dx + dy*dy);
            
            en->listaBalas[b].velX = (dx / dist) * 5.0f;
            en->listaBalas[b].velY = (dy / dist) * 5.0f;
            
            break;
        }
    }
}

void moverEnemigos(char mapa[ALTO][ANCHO], Juego *j) {
    float tiempoActual = al_get_time();
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!j->listaEnemigos[i].activo) 
            continue;
        float dx = j->player.X - j->listaEnemigos[i].X;
        float dy = j->player.Y - j->listaEnemigos[i].Y;
        float distancia = sqrt(dx*dx + dy*dy);
        if (j->listaEnemigos[i].tipo == 0) {
            if (distancia < RANGO_PERSECUCION) {
    float dirX = (dx > 0) ? 1.0 : -1.0;
    float dirY = (dy > 0) ? 1.0 : -1.0;

    if (!esColision(mapa, j->listaEnemigos[i].X + dirX, j->listaEnemigos[i].Y, false, j)) 
        j->listaEnemigos[i].X += dirX;
    
    if (!esColision(mapa, j->listaEnemigos[i].X, j->listaEnemigos[i].Y + dirY, false, j)) 
        j->listaEnemigos[i].Y += dirY;

    if (distancia < RANGO_DISPARO) {
        if (tiempoActual - j->listaEnemigos[i].ultimoDisparo >= COOLDOWN_DISPARO) {
            dispararEnemigo(&j->listaEnemigos[i], j->player.X, j->player.Y);
            j->listaEnemigos[i].ultimoDisparo = tiempoActual;
        }
    }
}
        }
    
        else if (j->listaEnemigos[i].tipo == 1) {
            // TIPO VERTICAL
            float proxY = j->listaEnemigos[i].Y + (j->listaEnemigos[i].sentido * 2.0);
            if (!esColision(mapa, j->listaEnemigos[i].X, proxY, false, j)) {
                j->listaEnemigos[i].Y = proxY;
            } 
            else {
                j->listaEnemigos[i].sentido *= -1;
            }
        } 
        else if (j->listaEnemigos[i].tipo == 2) {
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
    for (int i = 0; i < ALTO; i++) {
        for (int col = 0; col < ANCHO; col++) {
            float px = col * 50.0;
            float py = i * 50.0;
            
            switch(mapa[i][col]) {
                case '.': if (spritefondo) al_draw_bitmap(spritefondo, px, py, 0); 
                break;
                case '#': if (spritePared) al_draw_bitmap(spritePared, px, py, 0); 
                break;
                case 'K': if (spriteLlave) al_draw_bitmap(spriteLlave, px, py, 0); 
                break;
                case 'C': if (spriteCofre) al_draw_bitmap(spriteCofre, px, py, 0); 
                break;
                case 'B': al_draw_filled_circle(px, py, 5, al_map_rgb(135, 206, 235));
                break;
                case 'P': al_draw_rectangle(px, py, px + 50.0, py + 50.0, al_map_rgb(255, 255, 0), 2.0);
                break;
            }
        }
    }
}

void dibujarVida(int vida) {
    for (int i = 0; i < 3; i++) {
        int xInicial = 10;
        int yInicial = 10;
        int espaciado = 40;
        int corazonesMaximos = 5;
        
        int estado = vida - (i * 2);
        
        if (estado >= 2) {
            al_draw_bitmap(spriteCorazon, xInicial + (i * espaciado), yInicial, 0);
        } else if (estado == 1) {
            al_draw_bitmap(spriteMedioCorazon, xInicial + (i * espaciado), yInicial, 0);
        }
    }
}

void actualizarBalasEnemigos(char mapa[ALTO][ANCHO], Juego *j) {
    for (int i = 0; i < 12; i++) {
        for (int b = 0; b < MAX_BALAS_EN; b++) {
            if (j->listaEnemigos[i].listaBalas[b].activa) {

                j->listaEnemigos[i].listaBalas[b].X += j->listaEnemigos[i].listaBalas[b].velX;
                j->listaEnemigos[i].listaBalas[b].Y += j->listaEnemigos[i].listaBalas[b].velY;

                int col = (int)(j->listaEnemigos[i].listaBalas[b].X / TAMANO_CUADRADO);
                int fil = (int)(j->listaEnemigos[i].listaBalas[b].Y / TAMANO_CUADRADO);

                if (fil >= 0 && fil < ALTO && col >= 0 && col < ANCHO) {
                    if (mapa[fil][col] == '#') {
                        j->listaEnemigos[i].listaBalas[b].activa = false;
                        continue;
                    }
                } else {
                    j->listaEnemigos[i].listaBalas[b].activa = false;
                    continue;
                }

                float dx = j->player.X - j->listaEnemigos[i].listaBalas[b].X;
                float dy = j->player.Y - j->listaEnemigos[i].listaBalas[b].Y;
                float dist = sqrt(dx*dx + dy*dy);

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

    spriteEnemigo = al_load_bitmap("enemigo.png");
    al_convert_mask_to_alpha(spriteEnemigo, al_map_rgb(255, 255, 255));

    spritePared = al_load_bitmap("pared.png");
    spritefondo = al_load_bitmap("fondo.png");
    spriteLlave = al_load_bitmap("llave.png");
    spriteCofre = al_load_bitmap("cofre.png");
    spritePortal = al_load_bitmap("portal.png");
    spriteBala = al_load_bitmap("bala.png");
    spriteCorazon = al_load_bitmap("corazon.png");
    spriteMedioCorazon = al_load_bitmap("mediocorazon.png");
    spritePalanca = al_load_bitmap("palanca.png");

}

void asignarComportamientos(Juego *j) {
    int contPerseguir = 0, contVertical = 0, contHorizontal = 0;
    
    for (int i = 0; i < 12; i++) {
        bool asignado = false;
        while (!asignado) {
            int azar = rand() % 3;
            
            if (azar == 0 && contPerseguir < 5) { 
                j->listaEnemigos[i].tipo = 0; 
                contPerseguir++; 
                asignado = true; 
            }
            else if (azar == 1 && contVertical < 3) { 
                j->listaEnemigos[i].tipo = 1; 
                j->listaEnemigos[i].sentido = 1; 
                contVertical++; 
                asignado = true; 
            }
            else if (azar == 2 && contHorizontal < 4) { 
                j->listaEnemigos[i].tipo = 2; 
                j->listaEnemigos[i].sentido = 1; 
                contHorizontal++; 
                asignado = true; 
            }
        }
    }
}

bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala, Juego *j) {
    if (esBala) {
        int col = (int)(x / 50.0); 
        int fil = (int)(y / 50.0);
        if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) 
        return true;
        
        char celda = mapa[fil][col];
        if (celda == '#' || (celda == 'C' && !j->cofreAbierto)) {
            return true;
        }
        return false;
    } 
    else {
        float margenIzq = 8.0;
        float margenDer = 8.0;
        float margenArriba = 5.0;
        float margenAbajo = 2.0;
        float tamañoImagen = 48.0;

        float px[] = {x + margenIzq, x + tamañoImagen - margenDer, x + margenIzq, x + tamañoImagen - margenDer};
        float py[] = {y + margenArriba, y + margenArriba, y + tamañoImagen - margenAbajo, y + tamañoImagen - margenAbajo};

        for(int i = 0; i < 4; i++) {
            int col = (int)(px[i] / 50.0);
            int fil = (int)(py[i] / 50.0);

            if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) 
                return true;

            char celda = mapa[fil][col];
            
            if (celda == '#' || (celda == 'C' && !j->cofreAbierto)) {
                return true;
            }
        }
    }
    return false;
}
void dibujarInterfaz(personaje *p, int balas, ALLEGRO_FONT *fuente) {
    int inicioX = 150;
    int inicioY = 10;

    for (int i = 0; i < 3; i++) {
        int posX = inicioX + (i * 60);
        al_draw_filled_rectangle(posX, inicioY, posX + 50, inicioY + 50, al_map_rgb(50, 50, 50));
        al_draw_rectangle(posX, inicioY, posX + 50, inicioY + 50, al_map_rgb(200, 200, 200), 2);
    }
    for (int i = 0; i < 10; i++) {
        if (strcasecmp(p->bag.items[i].nombre, "Llave") == 0) {
            al_draw_bitmap(spriteLlave, inicioX + 5, inicioY + 5, 0); 
        }
        if (strcasecmp(p->bag.items[i].nombre, "Palanca") == 0) {
            al_draw_bitmap(spritePalanca, inicioX + 65, inicioY + 5, 0);
        }
    }
    al_draw_textf(fuente, al_map_rgb(255, 255, 255), inicioX + 200, inicioY + 15, 0, "Balas: %d", balas);
}
int buscarSlotLibre(personaje *p) {
    for (int i = 0; i < 10; i++) {
        if (p->bag.items[i].nombre[0] == '\0') return i;
    }
    return -1;
}
void limpiarInventario(personaje *p) {
    for (int i = 0; i < 10; i++) {
        p->bag.items[i].nombre[0] = '\0';
    }
}
void dibujarInventario(personaje *p, ALLEGRO_FONT *fuente) {
    for (int i = 0; i < 10; i++) { 
        if (p->bag.items[i].nombre[0] != '\0') {
            al_draw_textf(fuente, al_map_rgb(255, 255, 255), 10, 50 + (i * 20), 0, "Slot %d: %s", i+1, p->bag.items[i].nombre);
        }
    }    
}
bool guardarEnInventario(personaje *p, char *nombre, int consumir) {
    for (int i = 0; i < 10; i++) {
        if (p->bag.items[i].nombre[0] == '\0') {
            strcpy(p->bag.items[i].nombre, nombre);
            p->bag.items[i].consumir = consumir;
            return true;
        }
    }
    return false;
}
bool tieneObjeto(personaje *p, char *nombreObjeto) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(p->bag.items[i].nombre, nombreObjeto) == 0) {
            return true; // ¡Lo encontramos!
        }
    }
    return false; // No está en la mochila
}
void reiniciarJuego(char mapa[ALTO][ANCHO],personaje *p, ALLEGRO_FONT *fuente, Juego *j) {
    p->vida = VIDA_MAXIMA;
    p->balas = 6;
    cargarMapa(mapa, j);
    limpiarInventario(p);
    al_rest(0.5); 
    j->portalActivo = false;
}

void cambiarNivel(char mapa[ALTO][ANCHO], const char* nombreArchivo, Juego *j) {
    int indice = 0;
    for(int i = 0; i < 15; i++) 
    j->listaEnemigos[i].activo = false;
    for(int i = 0; i < 20; i++) 
    j->player.listaBalas[i].activa = false;
    
    FILE *f = fopen(nombreArchivo, "r");
    if (!f) { 
        printf("Error: No se pudo abrir el mapa.\n"); 
        return; 
    }
    for (int i = 0; i < ALTO; i++) {
        for (int col = 0; col < ANCHO; col++) {
            fscanf(f, " %c", &mapa[i][col]);
            if (mapa[i][col] == '@') { 
                j->player.X = col * 50; 
                j->player.Y = i * 50; 
                mapa[i][col] = '.'; 
            }
            if (mapa[i][col] == 'E' && indice < 15) {
            j->listaEnemigos[indice].X = col * TAMANO_CUADRADO;
            j->listaEnemigos[indice].Y = i * TAMANO_CUADRADO;
            j->listaEnemigos[indice].velocidad = 3.0;
            j->listaEnemigos[indice].activo = true;
            indice++;
            mapa[i][col] = '.';
            }
        }
    }
    fclose(f);
    j->portalActivo = false;
    j->cofreAbierto = false;
    j->tieneLlave = false;
}