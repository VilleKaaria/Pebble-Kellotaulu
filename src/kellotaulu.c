#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_connection_layer, *s_weekday_layer, *s_date_layer, *s_temperature_layer, *s_city_layer;

enum WeatherKey{
    WEATHER_TEMPERATURE_KEY,
    WEATHER_CITY_KEY,
};

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Read incoming data
    static char temperature_buffer[10];
    static char city_buffer[32];
    
    Tuple *temp_tuple = dict_find(iterator, WEATHER_TEMPERATURE_KEY);
    Tuple *city_tuple = dict_find(iterator, WEATHER_CITY_KEY);

    if(temp_tuple && city_tuple) {

        snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temp_tuple->value->cstring);
        snprintf(city_buffer, sizeof(city_buffer), "%s", city_tuple->value->cstring);
    
        text_layer_set_text(s_temperature_layer, temperature_buffer);
        text_layer_set_text(s_city_layer, city_buffer);
    }
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
    // Get time in format hh:mm
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), "%H:%M" , tick_time);

    text_layer_set_text(s_time_layer, s_buffer);
}

static void handle_bluetooth(bool connected) {
    // Check bluetooth connection
    text_layer_set_text(s_connection_layer, connected ? "yhdistetty" : "ei yhteyttä");
}

static void handle_dateweekday(){
    // Get weekday number and convert to Finnish weekday. FI locale not supported.
    time_t now = time(NULL);    
    struct tm *t = localtime(&now);
    static char s_daynumber[] = "0";
    strftime(s_daynumber, sizeof(s_daynumber), "%u", t);
    switch(s_daynumber[0]) {
        case '1':
            text_layer_set_text(s_weekday_layer, "Maanantai");
            break;
        case '2':
            text_layer_set_text(s_weekday_layer, "Tiistai");
            break;
        case '3':
            text_layer_set_text(s_weekday_layer, "Keskiviikko");
            break;
        case '4':
            text_layer_set_text(s_weekday_layer, "Torstai");
            break;
        case '5':
            text_layer_set_text(s_weekday_layer, "Perjantai");
            break;
        case '6':
            text_layer_set_text(s_weekday_layer, "Lauantai");
            break;
        case '7':
            text_layer_set_text(s_weekday_layer, "Sunnuntai");
            break;
        default:
            text_layer_set_text(s_weekday_layer, "");
            break;
    }
   
    // Get date in format dd.mm.yyyy
    static char s_date_text[] = "00.00.0000";
    strftime(s_date_text, sizeof(s_date_text), "%d.%m.%Y", t); 
    text_layer_set_text(s_date_layer, s_date_text);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();  

    // Get weather update every 15 minutes
    if(tick_time->tm_min % 15 == 0) {
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        dict_write_uint8(iter, 0, 0);

        app_message_outbox_send();
    }

    // Update date and weekday at 00:00
    if(tick_time->tm_hour == 0 && tick_time->tm_min == 0) {
        handle_dateweekday();
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Date and weekday updated!");
    }
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    // Format all layers
    s_time_layer = text_layer_create(GRect(0, 50, bounds.size.w, 50));
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    s_connection_layer = text_layer_create(GRect(0, 145, bounds.size.w, 34));
    text_layer_set_text_color(s_connection_layer, GColorWhite);
    text_layer_set_background_color(s_connection_layer, GColorClear);
    text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);

    s_weekday_layer = text_layer_create(GRect(0, 105, bounds.size.w, 34));
    text_layer_set_text_color(s_weekday_layer, GColorWhite);
    text_layer_set_background_color(s_weekday_layer, GColorClear);
    text_layer_set_font(s_weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentCenter);

    s_date_layer = text_layer_create(GRect(0, 125, bounds.size.w, 34));    
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

    s_temperature_layer = text_layer_create(GRect(0, 20, bounds.size.w, 32));
    text_layer_set_text_color(s_temperature_layer, GColorWhite);
    text_layer_set_background_color(s_temperature_layer, GColorClear);
    text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);

    s_city_layer = text_layer_create(GRect(0, 0, bounds.size.w, 32));
    text_layer_set_text_color(s_city_layer, GColorWhite);
    text_layer_set_background_color(s_city_layer, GColorClear);
    text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
    
    handle_bluetooth(connection_service_peek_pebble_app_connection());
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = handle_bluetooth
    });

    handle_dateweekday();

    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_weekday_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));    
    layer_add_child(window_layer, text_layer_get_layer(s_city_layer));
}

static void main_window_unload(Window *window) {
    tick_timer_service_unsubscribe();
    connection_service_unsubscribe();
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_connection_layer);
    text_layer_destroy(s_weekday_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_city_layer);
    text_layer_destroy(s_temperature_layer);
}

static void init() {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    window_stack_push(s_main_window, true);
    
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    const int inbox_size = 128;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);    
    
    update_time();    
}

static void deinit() {
    window_destroy(s_main_window);     
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
