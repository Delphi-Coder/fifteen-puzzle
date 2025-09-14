#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 4
#define TILE_COUNT (GRID_SIZE * GRID_SIZE)
#define PADDING 5

typedef struct {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *tiles[TILE_COUNT];
    int positions[TILE_COUNT]; // Changed from 'char' to 'int'
    int empty_index;
    GtkWidget *moves_label;
    GtkWidget *time_label;
    guint moves_count;
    guint timer_id;
    time_t start_time;
    gboolean is_paused;
    gboolean solved;
    time_t elapsed_time;
    GtkWidget *pause_resume_item; // Reference to the menu item
} GameState;

// Function prototypes
void shuffle_tiles(GameState *game_state);
gboolean is_solvable(int *positions);
void on_tile_clicked(GtkWidget *widget, gpointer data);
gboolean check_win_condition(GameState *game_state);
void create_tiles(GameState *game_state);
void swap_tiles(GameState *game_state, int index1, int index2);
gboolean is_solved(GameState *game_state);
void on_new_game_activate(GtkMenuItem *menuitem, gpointer data);
void on_exit_activate(GtkMenuItem *menuitem, gpointer data);
void on_about_activate(GtkMenuItem *menuitem, gpointer data);
void on_pause_resume_activate(GtkMenuItem *menuitem, gpointer data);
gboolean update_timer(gpointer data);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    srand(time(NULL)); // Seed the random number generator here

    GameState *game_state = g_new0(GameState, 1); // Allocate and zero-initialize GameState

    // Create main window
    game_state->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(game_state->window), "Fifteen Puzzle");
    gtk_window_set_default_size(GTK_WINDOW(game_state->window), 250, 300);
    g_signal_connect(game_state->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create vertical box to hold menu, labels, and grid
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(game_state->window), vbox);

    // Create menu bar
    GtkWidget *menu_bar = gtk_menu_bar_new();

    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);

    GtkWidget *new_game_item = gtk_menu_item_new_with_label("New");
    g_signal_connect(new_game_item, "activate", G_CALLBACK(on_new_game_activate), game_state);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_game_item);

    game_state->pause_resume_item = gtk_menu_item_new_with_label("Pause");
    g_signal_connect(game_state->pause_resume_item, "activate", G_CALLBACK(on_pause_resume_activate), game_state);
    gtk_menu_shell_insert(GTK_MENU_SHELL(file_menu), game_state->pause_resume_item, 2); // Insert appropriately

    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_exit_activate), game_state);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);

    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_menu_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu);

    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about_activate), game_state);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_menu_item);

    // Add menu bar to vbox
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Create horizontal box to hold move count and timer labels
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Move count label
    game_state->moves_label = gtk_label_new("Moves: 0");
    gtk_box_pack_start(GTK_BOX(hbox), game_state->moves_label, FALSE, FALSE, 0);

    // Timer label
    game_state->time_label = gtk_label_new("Time: 00:00:00");
    gtk_box_pack_end(GTK_BOX(hbox), game_state->time_label, FALSE, FALSE, 0);

    // Create grid container
    game_state->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(game_state->grid), PADDING);
    gtk_grid_set_column_spacing(GTK_GRID(game_state->grid), PADDING);
    gtk_container_set_border_width(GTK_CONTAINER(game_state->grid), PADDING);
    gtk_box_pack_start(GTK_BOX(vbox), game_state->grid, TRUE, TRUE, 0);

    // Initialize game state
    game_state->moves_count = 0;
    game_state->start_time = time(NULL);
    game_state->timer_id = g_timeout_add_seconds(1, update_timer, game_state);

    // Create tiles
    create_tiles(game_state);

    // Show all widgets
    gtk_widget_show_all(game_state->window);

    // Start GTK main loop
    gtk_main();

    // Cleanup
    g_free(game_state);

    return 0;
}

// Function to update the timer every second
gboolean update_timer(gpointer data) {
    GameState *game_state = (GameState *)data;
    if (game_state->solved)
        return TRUE;
    if (!game_state->is_paused) {
        time_t current_time = time(NULL);
        game_state->elapsed_time = current_time - game_state->start_time;
    }
    int hours = game_state->elapsed_time / 3600;
    int minutes = (game_state->elapsed_time % 3600) / 60;
    int seconds = game_state->elapsed_time % 60;
    char time_text[20];
    snprintf(time_text, sizeof(time_text), "Time: %02d:%02d:%02d", hours, minutes, seconds);
    gtk_label_set_text(GTK_LABEL(game_state->time_label), time_text);
    return TRUE; // Continue calling this function
}

// Function to reset the game
void reset_game(GameState *game_state) {
    // Remove existing tiles
    for (int i = 0; i < TILE_COUNT; i++) {
        gtk_widget_destroy(game_state->tiles[i]);
    }

    // Reset moves count and timer
    game_state->moves_count = 0;
    gtk_label_set_text(GTK_LABEL(game_state->moves_label), "Moves: 0");
    game_state->start_time = time(NULL);
    game_state->elapsed_time = 0;
    // Create new tiles
    create_tiles(game_state);
    game_state->solved = FALSE;
    // Refresh the grid
    gtk_widget_show_all(game_state->grid);
}

// Callback for "New" menu item
void on_new_game_activate(GtkMenuItem *menuitem, gpointer data) {
    GameState *game_state = (GameState *)data;

    reset_game(game_state);
}

// Callback for "Exit" menu item
void on_exit_activate(GtkMenuItem *menuitem, gpointer data) {
    gtk_main_quit();
}

// Callback for "About" menu item
void on_about_activate(GtkMenuItem *menuitem, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(((GameState *)data)->window),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Fifteen Puzzle Game\n\nDeveloped using GTK+ 3.");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void create_tiles(GameState *game_state) {
    // Initialize positions with numbers 1 to TILE_COUNT - 1 and 0 for the blank tile
    for (int i = 0; i < TILE_COUNT - 1; i++) {
        game_state->positions[i] = i + 1;
    }
    game_state->positions[TILE_COUNT - 1] = 0; // Blank tile

    // Shuffle until a solvable and unsolved puzzle is generated
    do {
        shuffle_tiles(game_state);
    } while (!is_solvable(game_state->positions) || is_solved(game_state));

    // Create tile buttons
    for (int i = 0; i < TILE_COUNT; i++) {
        game_state->tiles[i] = gtk_button_new();
        gtk_widget_set_hexpand(game_state->tiles[i], TRUE);
        gtk_widget_set_vexpand(game_state->tiles[i], TRUE);

        // Connect the "clicked" signal to all buttons
        g_signal_connect(game_state->tiles[i], "clicked", G_CALLBACK(on_tile_clicked), game_state);

        if (game_state->positions[i] != 0) {
            char label[4];
            snprintf(label, sizeof(label), "%d", game_state->positions[i]);
            gtk_button_set_label(GTK_BUTTON(game_state->tiles[i]), label);
            gtk_widget_set_sensitive(game_state->tiles[i], TRUE);
        } else {
            // It's the blank tile
            gtk_button_set_label(GTK_BUTTON(game_state->tiles[i]), "");
            gtk_widget_set_sensitive(game_state->tiles[i], FALSE);
            game_state->empty_index = i;
        }

        // Add the tile to the grid
        gtk_grid_attach(GTK_GRID(game_state->grid), game_state->tiles[i],
                        i % GRID_SIZE, i / GRID_SIZE, 1, 1);
    }
}

gboolean is_solved(GameState *game_state) {
    for (int i = 0; i < TILE_COUNT - 1; i++) {
        if (game_state->positions[i] != i + 1) {
            return FALSE;
        }
    }
    // The last tile should be the blank tile (0)
    if (game_state->positions[TILE_COUNT - 1] != 0) {
        return FALSE;
    }
    return TRUE;
}

void shuffle_tiles(GameState *game_state) {
    int *positions = game_state->positions;
    for (int i = TILE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = positions[i];
        positions[i] = positions[j];
        positions[j] = temp;
    }
}

gboolean is_solvable(int *positions) {
    int inversions = 0;

    // Count inversions
    for (int i = 0; i < TILE_COUNT; i++) {
        for (int j = i + 1; j < TILE_COUNT; j++) {
            if (positions[i] != 0 && positions[j] != 0 && positions[i] > positions[j]) {
                inversions++;
            }
        }
    }

    // Find the row of the blank tile from the bottom
    int blank_row_from_bottom = 0;
    for (int i = 0; i < TILE_COUNT; i++) {
        if (positions[i] == 0) {
            int blank_row = i / GRID_SIZE; // Row from top starting at 0
            blank_row_from_bottom = GRID_SIZE - blank_row; // Row from bottom starting at 1
            break;
        }
    }

    if (GRID_SIZE % 2 == 1) { // Odd grid size
        return inversions % 2 == 0;
    } else { // Even grid size
        if ((blank_row_from_bottom % 2 == 0 && inversions % 2 == 1) ||
            (blank_row_from_bottom % 2 == 1 && inversions % 2 == 0)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

void on_tile_clicked(GtkWidget *widget, gpointer data) {
    GameState *game_state = (GameState *)data;

    if (game_state->is_paused || game_state->solved) {
        return;
    }

    int index = -1;
    // Find the index of the clicked tile
    for (int i = 0; i < TILE_COUNT; i++) {
        if (game_state->tiles[i] == widget) {
            index = i;
            break;
        }
    }

    //printf("Index %d clicked.",index);
    if (index == -1 || game_state->positions[index] == 0) {
        // Clicked on an invalid tile or the blank tile
        return;
    }

    int empty_index = game_state->empty_index;
    int row = index / GRID_SIZE;
    int col = index % GRID_SIZE;
    int empty_row = empty_index / GRID_SIZE;
    int empty_col = empty_index % GRID_SIZE;

    // Check if the clicked tile is in the same row or column as the empty space
    if (row == empty_row && col != empty_col) {
        // Move tiles horizontally
        if (col < empty_col) {
            // Clicked tile is to the left of the empty tile
            // Move tiles to the left (empty tile moves left)
            for (int i = empty_col; i > col; i--) {
                swap_tiles(game_state, row * GRID_SIZE + i, row * GRID_SIZE + i - 1);
            }
        } else {
            // Clicked tile is to the right of the empty tile
            // Move tiles to the right (empty tile moves right)
            for (int i = empty_col; i < col; i++) {
                swap_tiles(game_state, row * GRID_SIZE + i, row * GRID_SIZE + i + 1);
            }
        }
        game_state->moves_count++;
    } else if (col == empty_col && row != empty_row) {
        // Move tiles vertically
        if (row < empty_row) {
            // Clicked tile is above the empty tile
            // Move tiles up (empty tile moves up)
            for (int i = empty_row; i > row; i--) {
                swap_tiles(game_state, i * GRID_SIZE + col, (i - 1) * GRID_SIZE + col);
            }
        } else {
            // Clicked tile is below the empty tile
            // Move tiles down (empty tile moves down)
            for (int i = empty_row; i < row; i++) {
                swap_tiles(game_state, i * GRID_SIZE + col, (i + 1) * GRID_SIZE + col);
            }
        }
        game_state->moves_count++;
    } else {
        // Not in the same row or column
        return;
    }

    // Update moves count label
    char moves_text[20];
    snprintf(moves_text, sizeof(moves_text), "Moves: %u", game_state->moves_count);
    gtk_label_set_text(GTK_LABEL(game_state->moves_label), moves_text);

    // Check if the player has won
    if (is_solved(game_state)) {
        int total_time = game_state->elapsed_time;
        int hours = total_time / 3600;
        int minutes = (total_time % 3600) / 60;
        int seconds = total_time % 60;
        game_state->solved = TRUE;
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(game_state->window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Congratulations! You solved the puzzle!\n\nTotal Moves: %u\nTotal Time: %02d:%02d:%02d",
            game_state->moves_count,
            hours, minutes, seconds
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}


void swap_tiles(GameState *game_state, int index1, int index2) {
    // Swap the positions in the positions array
    int temp_position = game_state->positions[index1];
    game_state->positions[index1] = game_state->positions[index2];
    game_state->positions[index2] = temp_position;

    // Update the empty index if necessary
    if (game_state->positions[index1] == 0) {
        game_state->empty_index = index1;
    } else if (game_state->positions[index2] == 0) {
        game_state->empty_index = index2;
    }

    // Update the labels and sensitivity of the buttons at index1 and index2
    GtkWidget *button1 = game_state->tiles[index1];
    GtkWidget *button2 = game_state->tiles[index2];

    int pos1 = game_state->positions[index1];
    int pos2 = game_state->positions[index2];

    if (pos1 == 0) {
        // Index1 is now the blank tile
        gtk_button_set_label(GTK_BUTTON(button1), "");
        gtk_widget_set_sensitive(button1, FALSE);
    } else {
        char label[4];
        snprintf(label, sizeof(label), "%d", pos1);
        gtk_button_set_label(GTK_BUTTON(button1), label);
        gtk_widget_set_sensitive(button1, TRUE);
    }

    if (pos2 == 0) {
        // Index2 is now the blank tile
        gtk_button_set_label(GTK_BUTTON(button2), "");
        gtk_widget_set_sensitive(button2, FALSE);
    } else {
        char label[4];
        snprintf(label, sizeof(label), "%d", pos2);
        gtk_button_set_label(GTK_BUTTON(button2), label);
        gtk_widget_set_sensitive(button2, TRUE);
    }
}

gboolean check_win_condition(GameState *game_state) {
    return is_solved(game_state);
}

void on_pause_resume_activate(GtkMenuItem *menuitem, gpointer data) {
    GameState *game_state = (GameState *)data;
    game_state->is_paused = !game_state->is_paused;

    if (game_state->is_paused) {
        // Stop the timer
        if (game_state->timer_id != 0) {
            g_source_remove(game_state->timer_id);
            game_state->timer_id = 0;
        }
        // Change menu item label to "Resume"
        gtk_menu_item_set_label(GTK_MENU_ITEM(game_state->pause_resume_item), "Resume");
        // Disable the grid (prevent moves)
        gtk_widget_set_sensitive(game_state->grid, FALSE);
    } else {
        // Update start_time to account for paused duration
        game_state->start_time = time(NULL) - game_state->elapsed_time;
        // Restart the timer
        game_state->timer_id = g_timeout_add_seconds(1, update_timer, game_state);
        // Change menu item label to "Pause"
        gtk_menu_item_set_label(GTK_MENU_ITEM(game_state->pause_resume_item), "Pause");
        // Enable the grid
        gtk_widget_set_sensitive(game_state->grid, TRUE);
    }
}
