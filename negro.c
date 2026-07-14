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

#define VELOCIDAD 5
#define ALTO 23
#define ANCHO 38
#define VIDA_MAXIMA 6
#define MAX_ENEMIGOS 12
#define VELOCIDAD_ENEMIGO 1
#define COOLDOWN_DISPARO_ENEMIGO 2.0

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
municion listaBalas[20];

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
    municion municion;
} personaje;

typedef struct {
    float X, Y;
    float velocidad;
    bool activo;
    int tipo;
    int sentido;
    double ultimoDisparo;
} enemigos;

    enemigos listaEnemigos[15];
    municion listaBalas[20];
    personaje jugador;
    bool tieneLlave = false;
    bool cofreAbierto = false;
    bool portalActivo = false;
    char mapa[ALTO][ANCHO];
    bool redibujar = true;
    bool salir = false;
    enemigos enemigo;
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

void cargarMapa(char mapa[ALTO][ANCHO]);
void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY);
void dibujarMapa(char mapa[ALTO][ANCHO]);
void dibujarVida(int vida);
void reiniciarJuego(char mapa[ALTO][ANCHO]);
void cargarAssets();
void moverEnemigos(char mapa[ALTO][ANCHO]);
bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala);
void cambiarNivel(char mapa[ALTO][ANCHO], const char* nombreArchivo);
void asignarComportamientos();

int main() {
    srand(time(NULL));
    if (!al_init()) return -1;
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();

    cargarAssets();
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
    al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA);

    ALLEGRO_DISPLAY *display = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0); // 60 FPS
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    char mapa[ALTO][ANCHO];
    cargarMapa(mapa);
    asignarComportamientos();
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

    bool salir = false;
    bool redibujar = true;
    jugador.vida = 6;
    jugador.balas = 6;

    while (!salir) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        

        // --- LÓGICA
        if (evento.type == ALLEGRO_EVENT_TIMER) {

            redibujar = true;
            ALLEGRO_KEYBOARD_STATE teclado;
            printf("pos x: %f - pos y:%f\n", jugador.X, jugador.Y);
            al_get_keyboard_state(&teclado);

            float nuevaX = jugador.X, nuevaY = jugador.Y;
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
            
            colision(mapa, &jugador.X, &jugador.Y, nuevaX, nuevaY);
            moverEnemigos(mapa);
            int col = (int)((jugador.X + 25) / 50.0);
            int fil = (int)((jugador.Y + 25) / 50.0);
            char objeto = mapa[fil][col];
            if(mapa[col][fil] == 'B'){
                        jugador.balas += 3;
                        mapa[col][fil] = ' ';
                    }
            if (objeto == 'K') { 
                tieneLlave = true;
                mapa[fil][col] = ' '; 
            }
            else if (objeto == 'C' && tieneLlave) { 
                cofreAbierto = true; 
                portalActivo = true; 
                mapa[fil][col] = ' '; 
            }
            else if (objeto == 'P' && portalActivo) {
                printf("¡Cambiando de nivel!\n");
                cambiarNivel(mapa, "mapa3.txt");
            }
            for (int i = 0; i < 15; i++) {
                if (listaEnemigos[i].activo) {
                    float dx = jugador.X - listaEnemigos[i].X;
                    float dy = jugador.Y - listaEnemigos[i].Y;
                    float distancia = sqrt(dx*dx + dy*dy);

                    if (distancia < 30) {
                        static int framesColision = 0;
                        framesColision++;
                        if (framesColision > 30) {
                            jugador.vida--;
                            framesColision = 0;
                
                            if (jugador.vida <= 0) {
                                printf("Game Over\n");
                                reiniciarJuego(mapa);
                            }
                        }
                    }
                }
            }

            // Mover y colisionar balas
            for (int i = 0; i < 10; i++) {
                if (misBalas[i].activa) {
                    misBalas[i].X += misBalas[i].velX;
                    misBalas[i].Y += misBalas[i].velY;
                    if (esColision(mapa, misBalas[i].X, misBalas[i].Y, true)) misBalas[i].activa = false;
                    else {
                        for (int j = 0; j < MAX_ENEMIGOS; j++) {
                            if (listaEnemigos[j].activo) {
                                float dx = misBalas[i].X - (listaEnemigos[j].X + 25);
                                float dy = misBalas[i].Y - (listaEnemigos[j].Y + 25);
                                if ((dx*dx + dy*dy) < 900) {
                                    listaEnemigos[j].activo = false;
                                    misBalas[i].activa = false;
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
            if (jugador.balas > 0) {
                for(int i = 0; i < 10; i++) {
                    if(!misBalas[i].activa) {
                        misBalas[i].X = jugador.X + 25; misBalas[i].Y = jugador.Y + 25;
                        misBalas[i].activa = true;
                        misBalas[i].velX = (jugador.direccion == 2) ? 10 : (jugador.direccion == 1) ? -10 : 0;
                        misBalas[i].velY = (jugador.direccion == 0) ? 10 : (jugador.direccion == 3) ? -10 : 0;
                        jugador.balas--; break;
                    }
                }
            }
        }

        //DIBUJO
        if (redibujar && al_is_event_queue_empty(queue)) {
            redibujar = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(fondoCompleto, 0, 0, 0);
            dibujarMapa(mapa);
            for (int i = 0; i < 20; i++) {
                if (listaBalas[i].activa) {
                    al_draw_filled_circle(listaBalas[i].X + 25, listaBalas[i].Y + 25, 8, al_map_rgb(0, 0, 0));
                }
            }

            if (spriteJugador){
                al_draw_scaled_bitmap(spriteJugador, 0, 0, 50, 50, jugador.X, jugador.Y, 50, 50, 0);
            } 
            for (int i = 0; i < 15; i++) {
                if (listaEnemigos[i].activo) {
                al_draw_scaled_bitmap(spriteEnemigo, 0, 0, 50, 50, listaEnemigos[i].X, listaEnemigos[i].Y, 50, 50, 0);
                }
            }
            for (int i = 0; i < 10; i++) {
                if (misBalas[i].activa) {
                    al_draw_filled_circle(misBalas[i].X, misBalas[i].Y, 5, al_map_rgb(0, 0, 0));
                }
            }
            dibujarVida(jugador.vida);
            
            al_flip_display();
        }

        
    }
    al_destroy_bitmap(fondoCompleto);
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
            }
            if (mapa[i][j] == 'E' && indice < 15) {
            listaEnemigos[indice].X = j * TAMANO_CUADRADO;
            listaEnemigos[indice].Y = i * TAMANO_CUADRADO;
            listaEnemigos[indice].velocidad = 3.0;
            listaEnemigos[indice].activo = true;
            indice++;
            }
            if (mapa[i][j] == 'B') {
            listaBalas[k].X = j * TAMANO_CUADRADO;
            listaBalas[k].Y = i * TAMANO_CUADRADO;
            listaBalas[k].activa = true;
            k++;
            }

            float px = j * 50.0; 
            float py = i * 50.0;
        }
    }
    fclose(f);
}

void colision(char mapa[ALTO][ANCHO], float *posX, float *posY, float nuevaX, float nuevaY) {

    float size = TAMANO_CUADRADO;
    if (nuevaX != *posX) {
        int direccion = (nuevaX > *posX) ? 1 : -1;//VER
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

void moverEnemigos(char mapa[ALTO][ANCHO]) {
    for (int i = 0; i < 12; i++) {
        if (!listaEnemigos[i].activo) continue;

        if (listaEnemigos[i].tipo == 0) {
    float dx = (jugador.X > listaEnemigos[i].X) ? 1.0 : (jugador.X < listaEnemigos[i].X ? -1.0 : 0.0);
    float dy = (jugador.Y > listaEnemigos[i].Y) ? 1.0 : (jugador.Y < listaEnemigos[i].Y ? -1.0 : 0.0);

    if (!esColision(mapa, listaEnemigos[i].X + dx, listaEnemigos[i].Y + dy, false)) {
            listaEnemigos[i].X += dx;
            listaEnemigos[i].Y += dy;
            }
            else if (!esColision(mapa, listaEnemigos[i].X + dx, listaEnemigos[i].Y, false)) {
                listaEnemigos[i].X += dx;
            }
            else if (!esColision(mapa, listaEnemigos[i].X, listaEnemigos[i].Y + dy, false)) {
                listaEnemigos[i].Y += dy;
            }
        }
    
        else if (listaEnemigos[i].tipo == 1) {
            // TIPO VERTICAL
            float proxY = listaEnemigos[i].Y + (listaEnemigos[i].sentido * 2.0);
            if (!esColision(mapa, listaEnemigos[i].X, proxY, false)) {
                listaEnemigos[i].Y = proxY;
            } 
            else {
                listaEnemigos[i].sentido *= -1;
            }
        } 
        else if (listaEnemigos[i].tipo == 2) {
            // TIPO HORIZONTAL
            float proxX = listaEnemigos[i].X + (listaEnemigos[i].sentido * 2.0);
            if (!esColision(mapa, proxX, listaEnemigos[i].Y, false)) {
                listaEnemigos[i].X = proxX;
            } else {
                listaEnemigos[i].sentido *= -1;
            }
        }
    }
}

void dibujarMapa(char mapa[ALTO][ANCHO]) {
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            float px = j * 50.0;
            float py = i * 50.0;
            

            switch(mapa[i][j]) {
                case '.': if (spritefondo) al_draw_bitmap(spritefondo, px, py, 0); 
                break;
                case '#': if (spritePared) al_draw_bitmap(spritePared, px, py, 0); 
                break;
                case 'K': if (spriteLlave) al_draw_bitmap(spriteLlave, px, py, 0); 
                break;
                case 'C': if (spriteCofre) al_draw_bitmap(spriteCofre, px, py, 0); 
                break;
                case 'P': if (portalActivo && spritePortal) al_draw_bitmap(spritePortal, px, py, 0); 
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

void reiniciarJuego(char mapa[ALTO][ANCHO]) {
    jugador.vida = VIDA_MAXIMA;
    jugador.balas = 9;
    cargarMapa(mapa);
    al_rest(0.5); 
    portalActivo = false;
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
    //spriteBala = al_load_bitmap("bala.png");
    spriteCorazon = al_load_bitmap("corazon.png");
    spriteMedioCorazon = al_load_bitmap("mediocorazon.png");
}

void asignarComportamientos() {
    int contPerseguir = 0, contVertical = 0, contHorizontal = 0;
    
    for (int i = 0; i < 12; i++) {
        bool asignado = false;
        while (!asignado) {
            int azar = rand() % 3;
            
            if (azar == 0 && contPerseguir < 5) { 
                listaEnemigos[i].tipo = 0; 
                contPerseguir++; 
                asignado = true; 
            }
            else if (azar == 1 && contVertical < 3) { 
                listaEnemigos[i].tipo = 1; 
                listaEnemigos[i].sentido = 1; 
                contVertical++; 
                asignado = true; 
            }
            else if (azar == 2 && contHorizontal < 4) { 
                listaEnemigos[i].tipo = 2; 
                listaEnemigos[i].sentido = 1; 
                contHorizontal++; 
                asignado = true; 
            }
        }
    }
}

bool esColision(char mapa[ALTO][ANCHO], float x, float y, bool esBala) {
    if (esBala) {
        int col = (int)(x / 50.0); 
        int fil = (int)(y / 50.0);
        if (col < 0 || col >= ANCHO || fil < 0 || fil >= ALTO) 
        return true;
        
        char celda = mapa[fil][col];
        if (celda == '#' || (celda == 'C' && !cofreAbierto)) {
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
            
            if (celda == '#' || (celda == 'C' && !cofreAbierto)) {
                return true;
            }
        }
    }
    return false;
}

void cambiarNivel(char mapa[ALTO][ANCHO], const char* nombreArchivo) {
    int indice = 0;
    int k = 0;
    for(int i = 0; i < 15; i++) 
    listaEnemigos[i].activo = false;
    for(int i = 0; i < 20; i++) 
    listaBalas[i].activa = false;
    
    FILE *f = fopen(nombreArchivo, "r");
    if (!f) { 
        printf("Error: No se pudo abrir el mapa.\n"); 
        return; 
    }
    for (int i = 0; i < ALTO; i++) {
        for (int j = 0; j < ANCHO; j++) {
            fscanf(f, " %c", &mapa[i][j]);
            if (mapa[i][j] == '@') { 
                jugador.X = j * 50; 
                jugador.Y = i * 50; 
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
    portalActivo = false;
    cofreAbierto = false;
    tieneLlave = false;
}
