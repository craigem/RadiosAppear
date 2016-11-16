#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
//static TextLayer *s_weather_layer;

static BitmapLayer *s_background_layer, *s_bt_icon_layer, *s_battery_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap, *s_battery_icon_bitmap;

// Declare globally
static GFont s_time_font;
//static GFont s_weather_font;

static int s_battery_level;

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;

  // Hide the battery icon by default
  layer_set_hidden(bitmap_layer_get_layer(s_battery_icon_layer), s_battery_level >= 30);

  // This vibrate on every check. Need to set this to only vibrate once.
  // notify the user
  //if(s_battery_level <= 30) {
    // Issue a vibrating alert
  //  vibes_double_pulse();
  //}
}

//static void inbox_received_callback( DictionaryIterator *iterator, void *context) {
  // Store incoming information
//  static char temperature_buffer[8];
//  static char conditions_buffer[32];
//  static char weather_layer_buffer[32];

  // Read tuples for data
//  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
//  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all data is available, use it
//  if(temp_tuple && conditions_tuple) {
//    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
//    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
//    conditions_tuple->value->cstring);

    // Assemble full string and display
//    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
//    text_layer_set_text(s_weather_layer, weather_layer_buffer);
//  }
//}

//static void inbox_dropped_callback(AppMessageResult reason, void *context) {
//  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
//}

//static void outbox_failed_callback( DictionaryIterator *iterator, AppMessageResult reason, void *context) {
//  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
//}

//static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
//  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
//}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
    "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
   if(tick_time->tm_min % 30 == 0) {
     // Begin dictionary
     DictionaryIterator *iter;
     app_message_outbox_begin(&iter);

     // Add a key-value pair
     dict_write_uint8(iter, 0, 0);

     // Send the message!
     app_message_outbox_send();
   }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto he layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
    GRect(PBL_IF_ROUND_ELSE(59, 41), PBL_IF_ROUND_ELSE(70, 64), 65, 33));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorRed);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_23));

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create temperature layer
//  s_weather_layer = text_layer_create(
//    GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25));

  // Style the text
//  text_layer_set_background_color(s_weather_layer, GColorClear);
//  text_layer_set_text_color(s_weather_layer, GColorWhite);
//  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentRight);
//  text_layer_set_text(s_weather_layer, "Loading...");

  // Create the second custom font, apply it and add to Window
//  s_weather_font = fonts_load_custom_font(
//    resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
//  text_layer_set_font(s_weather_layer, s_weather_font);
//  layer_add_child(window_get_root_layer(window),
//    text_layer_get_layer(s_weather_layer));

  // Create battery icon Layer
  s_battery_icon_layer = bitmap_layer_create(GRect(58, 45, 30, 30));
  s_battery_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_ICON);
  bitmap_layer_set_bitmap(s_battery_icon_layer, s_battery_icon_bitmap);

  // Add to window
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_icon_layer));

  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

  // Create the Bitmaplayer to display the GBitmap
  s_bt_icon_layer = bitmap_layer_create(
    GRect(PBL_IF_ROUND_ELSE(75, 58), 5, 30, 30));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

  // Show the correct state of the Bluetooth connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_load_custom_font(s_time_layer);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy Bitmaplayer
  bitmap_layer_destroy(s_background_layer);

  // Destroy weather elements
//  text_layer_destroy(s_weather_layer);
//  fonts_load_custom_font(s_weather_font);

  // Destroy battery level
  gbitmap_destroy(s_battery_icon_bitmap);
  bitmap_layer_destroy(s_battery_icon_layer);

  // Destroy Bluetooth indicator
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background colour
  window_set_background_color(s_main_window, GColorBlack);

  //Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
//  app_message_register_inbox_received(inbox_received_callback);
//  app_message_register_inbox_dropped(inbox_dropped_callback);
//  app_message_register_outbox_failed(outbox_failed_callback);
//  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
//  const int inbox_size = 128;
//  const int outbox_size = 128;
//  app_message_open(inbox_size, outbox_size);

  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);

  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit() {
  // Destroy window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
