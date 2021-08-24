/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
#pragma once
#define TEXT_BUFFER_SIZE		4097
#define STRIP_ROWS				   4
#define STRIP_COLS				   5
#define STRIP_COUNT				 (STRIP_ROWS * STRIP_COLS)
#define FRAME_WIDTH				 300
#define FRAME_HEIGHT			 300

#define ISF_TERMINATOR	99
#define ISF_GESSO		98

typedef struct film_strip_data
{
	int frame;
	bool used;
	char file_name[FILENAME_DIRENT_MAX];
	GtkWidget *image_widget;
	GtkWidget *mode_widget;
	GtkWidget *filename_widget;
	char dropped_text[FILENAME_DIRENT_MAX];
	int mode;
}
FILM_STRIP;

extern char isf_file_name[PATH_MAX + 1];
extern char image_folder[FILENAME_MAX + 1];
extern FILM_STRIP film_strip_data[STRIP_COUNT + 1];
extern FILM_STRIP clipboard_data[1];
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void gui_program_error_dialog_box
(
	const char *path_file, int line, const char *function_name,
	const char *control, ...
);
void gui_program_warning_dialog_box
(
	const char *path_file, int line, const char *function_name,
	const char *control, ...
);
void gui_information_dialog_box(const char *control, ...);
void gui_printf(const char *control, ...);
GdkPixbuf *initial_image(int height);

/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
void gui_update(void)
{
	// this yields execution to the GUI loop if any events are pending
	while(gtk_events_pending()) gtk_main_iteration();
};

------------------------------------------------------------------------------*/

char *delete_filename_from_path(const char *file);
void save_isf(char *list_filename);
int load_isf(char *list_filename);
void initialize_film_strip_data(void);
void print_film_strip_data(void);

/*------------------------------------------------------------------------------

	GtkWidget *widget,
------------------------------------------------------------------------------*/

void display_strip_image
(
	int frame,
	char *file_name
);

/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
