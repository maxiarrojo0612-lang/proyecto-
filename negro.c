#include <allegro5/allegro.h>
#include <stdio.h>
#include <stdbool.h>

int main() {
    printf("1. Iniciando Allegro...\n");
    if (!al_init()) {
        fprintf(stderr, "Error al inicializar Allegro.\n");
        return -1;
    }

    printf("2. Creando la ventana...\n");
    ALLEGRO_DISPLAY *display = al_create_display(800, 600);
    if (!display) {
        fprintf(stderr, "Error al crear la ventana.\n");
        return -1;
    }

    printf("3. Ventana creada con éxito. Pintando de negro...\n");
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    printf("4. Entrando al bucle de eventos (esperando a que cierres la ventana)...\n");
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));

    bool corriendo = true;
    while (corriendo) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(event_queue, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            printf("5. Has cerrado la ventana. Saliendo...\n");
            corriendo = false;
        }
    }

    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
    
    return 0;
}