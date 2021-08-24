/*------------------------------------------------------------------------------


------------------------------------------------------------------------------*/
#include "common_header.h"
#include "lip_image_processing.h"
#include "dir_utilities.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "steenbeck.h"
#include "durer-rhinoceros-engraving-1515.xpm"

/*------------------------------------------------------------------------------
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <canberra-gtk.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

sudo apt-get install libgtk-3-dev

link options set to: $(shell pkg-config --libs gtk+-3.0);-g -lstdc++ ;-no-pie
looks like the -lstdc++ is important
------------------------------------------------------------------------------*/

FILM_STRIP film_strip_data[STRIP_COUNT + 1];
FILM_STRIP clipboard_data[1];

/*------------------------------------------------------------------------------
	drag and drop definitions
    { "text/uri-list",					TARGET_URI_LIST,	TARGET_URI_LIST	},
    { "application/x-color",			TARGET_COLOR,		TARGET_COLOR	},
    { "property/bgimage",				TARGET_BGIMAGE,		TARGET_BGIMAGE	},
    { "property/keyword",				TARGET_KEYWORD,		TARGET_KEYWORD	},
------------------------------------------------------------------------------*/
enum
{
    TARGET_PLAIN,
    TARGET_COLOR,
    TARGET_URI_LIST,
    TARGET_BGIMAGE,
    TARGET_KEYWORD,
    TARGET_BACKGROUND_RESET,
    TARGET_MATE_URI_LIST
};

static const GtkTargetEntry target_table[] =
{
    {"text/plain",	TARGET_PLAIN, TARGET_PLAIN}
};
/*------------------------------------------------------------------------------
int image_display_width		= 2000;
, image_display_height	= 100
------------------------------------------------------------------------------*/
int console_display_width	=  720, console_display_height	= 100;

char isf_file_name[PATH_MAX + 1];
char image_folder[FILENAME_MAX + 1];
/*------------------------------------------------------------------------------
	In general, the application code does NOT know about <gtk/gtk.h> but the UI
	code does know about the application's data structures such as "image.h"
------------------------------------------------------------------------------*/
GtkWidget *window, *grid;
GtkWidget *input_image = NULL, *output_image = NULL;
GtkWidget *console_text_view;
/*------------------------------------------------------------------------------

#include <gtk/gtk.h>

gboolean clicked
(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer user_data
)
{
    if(event->type == GDK_DOUBLE_BUTTON_PRESS)
        printf("double\n");

    return TRUE;
}
int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);
    GtkWidget *window;
    GtkWidget *label;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    label = gtk_label_new("label");
    gtk_container_add(GTK_CONTAINER(window), label);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(window, "button-press-event", G_CALLBACK(clicked), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_widget_show_all(window);
    gtk_main ();
    return 0;
}
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
		static const char * Durer_Rhinoceros_xpm[] = {
	//printf("height = %d scale = %4.2lf\n", height, scale);

------------------------------------------------------------------------------*/
GdkPixbuf *initial_image(int height)
{
	GdkPixbuf *rhino_pixbuf, *scaled_pixbuf;
	int image_width, image_height;
	double scale;

	//rhino_pixbuf = gdk_pixbuf_new_from_xpm_data(Durer_Rhinoceros_xpm);
	rhino_pixbuf = gdk_pixbuf_new_from_xpm_data(durer_rhinoceros_engraving_1515_xpm);
	image_width  = gdk_pixbuf_get_width(rhino_pixbuf);
	image_height = gdk_pixbuf_get_height(rhino_pixbuf);
	scale = height / (double)image_height;

	scaled_pixbuf = gdk_pixbuf_scale_simple
	(
	    rhino_pixbuf,
	    FRAME_WIDTH, //image_width * scale,
	    FRAME_HEIGHT, //height,
	    GDK_INTERP_HYPER
	);

	// un-reference should free the original pixel buffer
	g_object_unref((gpointer)rhino_pixbuf);

	return scaled_pixbuf;
}
/*------------------------------------------------------------------------------
	strcpy(image_folder, "/mnt/art_USB3/C/images");

------------------------------------------------------------------------------*/
void initialize_all_film_strip_data(void)
{
	GtkWidget *image_widget;
	GtkWidget *mode_widget;
	GtkWidget *filename_widget;

	for(int k = 0; k < STRIP_COUNT; k++)
	{
		filename_widget	= film_strip_data[k + 1].filename_widget;
		image_widget	= film_strip_data[k + 1].image_widget;
		mode_widget		= film_strip_data[k + 1].mode_widget;

	//printf("%s %d\n", __FUNC__, __LINE__);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), initial_image(FRAME_HEIGHT));
	//printf("%s %d\n", __FUNC__, __LINE__);
		gtk_entry_set_text(GTK_ENTRY(filename_widget), "...");
		gtk_entry_set_text(GTK_ENTRY(mode_widget), "0");

		film_strip_data[k + 1].frame			= k + 1;
		film_strip_data[k + 1].used				= FALSE;
		film_strip_data[k + 1].mode				= 0;	// default to full image
		*(film_strip_data[k + 1].file_name)		= '\0';
		*(film_strip_data[k + 1].dropped_text)	= '\0';
	}

	//printf("%s %d\n", __FUNC__, __LINE__);
	strcpy(image_folder, "/mnt/art_USB3/C/images");
}
/*------------------------------------------------------------------------------
	strcpy(image_folder, "/mnt/art_USB3/C/images");

------------------------------------------------------------------------------*/
void initialize_film_strip_entry(int k)
{
	GtkWidget *image_widget;
	GtkWidget *mode_widget;
	GtkWidget *filename_widget;

	filename_widget	= film_strip_data[k + 1].filename_widget;
	image_widget	= film_strip_data[k + 1].image_widget;
	mode_widget		= film_strip_data[k + 1].mode_widget;

	gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), initial_image(FRAME_HEIGHT));
	gtk_entry_set_text(GTK_ENTRY(filename_widget), "...");
	gtk_entry_set_text(GTK_ENTRY(mode_widget), "0");

	film_strip_data[k + 1].used				= FALSE;
	film_strip_data[k + 1].mode				= 0;	// default to full image
	*(film_strip_data[k + 1].file_name)		= '\0';
	*(film_strip_data[k + 1].dropped_text)	= '\0';

	strcpy(image_folder, "/mnt/art_USB3/C/images");
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void gui_update(void)
{
	// this yields execution to the GUI loop if any events are pending
	while(gtk_events_pending()) gtk_main_iteration();
}
/*------------------------------------------------------------------------------


------------------------------------------------------------------------------*/
void gui_program_error_dialog_box
(
	const char *path_file, int line, const char *function_name,
	const char *control, ...
)
{
	va_list parms;
	char text[TEXT_BUFFER_SIZE];
	const char *file;
	GtkWidget *dialog;

	va_start(parms, control);
	vsnprintf(text, TEXT_BUFFER_SIZE, control, parms);
	printf(ANSI_COLOR_RED"** error ** %-40s\n", text);
	file = (path_file);
	printf("(in file: %s at line: %d in function: %s)\n"ANSI_COLOR_RESET,
		file, line, function_name);

	dialog = gtk_message_dialog_new
	(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"%s\nin function '%s'\nin file '%s' line %d",
			text, function_name, (path_file), line
	);

	//gtk_window_set_title(GTK_WINDOW(dialog),
		//"Okay, Houston, we've had a problem here.");
	gtk_window_set_title(GTK_WINDOW(dialog),
		"Oh, shit.");
		//PROGRAM_NAME_TEXT
		//" has an error :(");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	exit(EXIT_FAILURE);
	va_end(parms);
}
/*------------------------------------------------------------------------------
	gui_warning_dialog_box(__FILE__, __LINE__, __FUNC__, "a test error");

------------------------------------------------------------------------------*/
void gui_program_warning_dialog_box
(
	const char *path_file, int line, const char *function_name,
	const char *control, ...
)
{
	va_list parms;
	char text[TEXT_BUFFER_SIZE];
	const char *file;
	GtkWidget *dialog;

	va_start(parms, control);
	vsnprintf(text, TEXT_BUFFER_SIZE, control, parms);
	printf("** warning ** %-40s\n", text);
	file = (path_file);
	printf("(in file: %s at line: %d\nin function: %s)\n", file, line, function_name);

	dialog = gtk_message_dialog_new
	(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_CLOSE,
		"%s\nin function '%s'\nin file '%s' line %d",
			text, function_name, (path_file), line
	);

	gtk_window_set_title(GTK_WINDOW(dialog), "Oh dear! ");
		//PROGRAM_NAME_TEXT
		//" has an error :(");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	va_end(parms);
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void gui_information_dialog_box(const char *control, ...)
{
	va_list parms;
	char text[TEXT_BUFFER_SIZE];
	GtkWidget *dialog;

	va_start(parms, control);
	vsnprintf(text, TEXT_BUFFER_SIZE, control, parms);
	printf("gtk_message_dialog_new='%s'", text);

	dialog = gtk_message_dialog_new
	(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_CLOSE,
		"%s", text
	);

	gtk_window_set_title(GTK_WINDOW(dialog), "information");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	va_end(parms);
}
/*------------------------------------------------------------------------------
	this is a gui version of printf that prints to _a_ Gtk console window
------------------------------------------------------------------------------*/
void gui_console_printf(GtkWidget **console_text_view, const char *control, ...)
{
	va_list parms;
	GtkTextBuffer *console_buffer;
	GtkTextMark *console_mark;
	char text[TEXT_BUFFER_SIZE];

	va_start(parms, control);
	vsnprintf(text, TEXT_BUFFER_SIZE, control, parms);
	printf("%s", text);
	console_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(*console_text_view));
	gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(console_buffer), text, -1);
	console_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(console_buffer));
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(*console_text_view), console_mark);
	gui_update();
	va_end(parms);
}
/*------------------------------------------------------------------------------
	this is a gui version of printf that prints to the Gtk console window
------------------------------------------------------------------------------*/
void gui_printf(const char *control, ...)
{
	va_list parms;
	GtkTextBuffer *console_buffer;
	GtkTextMark *console_mark;
	char text[TEXT_BUFFER_SIZE];

	va_start(parms, control);
	vsnprintf(text, TEXT_BUFFER_SIZE, control, parms);
	//vsprintf(text, control, parms);
	console_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(console_text_view));
	//printf("OK '%s' %d\n", text, __LINE__);
	gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(console_buffer), text, -1);
	//printf("OK '%s' %d\n", text, __LINE__);
	console_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(console_buffer));
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(console_text_view), console_mark);
	gui_update();
	va_end(parms);
}
/*------------------------------------------------------------------------------
	install a console window of specified dimensions
	the returned scroll window widget must be attached to the application
------------------------------------------------------------------------------*/
GtkWidget *install_console
(
	int width, int height,
	GtkWidget **console_text_view
)
{
	GtkWidget *scroll_window;
	GtkTextBuffer *console_buffer;
	PangoFontDescription *font;
	GdkRGBA rgba;

	scroll_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scroll_window), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll_window, width, height);

	*console_text_view = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scroll_window), *console_text_view);

	font = pango_font_description_from_string("Monospace 10");
	gtk_widget_override_font(*console_text_view, font);

	console_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(*console_text_view));

	// Change default color throughout the widget
	gdk_rgba_parse (&rgba, "blue");
	gtk_widget_override_color(*console_text_view, GTK_STATE_FLAG_NORMAL, &rgba);

	gtk_text_buffer_insert_at_cursor(console_buffer, "", -1);

	return scroll_window;
}
/*------------------------------------------------------------------------------
__attribute__((unused)__attribute__((unused))

https://code.woboq.org/gtk/gtk/gdk/gdkkeysyms.h.html

 GDK_KEY_Delete
 GDK_KEY_BackSpace
------------------------------------------------------------------------------*/
void keypress_function
(
	GtkWidget *widget,
	GdkEventKey *event,
	FILM_STRIP *film_strip_frame
)
{
	char widget_name[255];

	strcpy(widget_name, gtk_widget_get_name(widget));

	if(event->keyval == GDK_KEY_Delete)
	{
		gui_printf
		(
			"widget %s delete key pressed\t: 0x%02X\n",
			widget_name, event->keyval
		);
	}
	else
	if(event->state == 4)		// control keys
	{
#if 0
		gui_printf
		(
			"widget %s key pressed\t: ctrl-'%c' 0x%02X\n",
			widget_name, event->keyval, event->keyval
		);
#endif
		switch(event->keyval)
		{
			case 'x':
			// copy to clipboard
			gui_printf("cut\t\t: |%s|\n", film_strip_frame->file_name);
			strcpy(clipboard_data->file_name, film_strip_frame->file_name);
			clipboard_data->mode = film_strip_frame->mode;
			clipboard_data->used = film_strip_frame->used;
			gui_printf("clipboard\t: |%s|\n", clipboard_data->file_name);
			// delete here
			film_strip_frame->used = FALSE;
			break;

			case 'c':
			// copy to clipboard
			gui_printf("copy\t\t: |%s|\n", film_strip_frame->file_name);
			strcpy(clipboard_data->file_name, film_strip_frame->file_name);
			clipboard_data->mode = film_strip_frame->mode;
			clipboard_data->used = film_strip_frame->used;
			gui_printf("clipboard\t: |%s|\n", clipboard_data->file_name);

			break;

			case 'v':
			// paste from clipboard
			gui_printf("paste\t\t: |%s|\n", clipboard_data->file_name);
			strcpy(film_strip_frame->file_name, clipboard_data->file_name);
			film_strip_frame->mode = 	clipboard_data->mode;
			film_strip_frame->used = TRUE;

			display_strip_image
			(
				film_strip_frame->frame,
				film_strip_frame->file_name
			);

			break;

			default:
			break;
		}
	}
	else
	{
		gui_printf
		(
			"widget %s unused key pressed\t: '%c' 0x%02X\n",
			widget_name, event->keyval, event->keyval
		);
	}

	if(film_strip_frame->used)
	{
#if 0
		gui_printf
		(
			"keystroke frame\t: %s\n",
			film_strip_frame->file_name
		);
#endif
	}
	else
	{
		gui_printf
		(
			"unused frame\t: %s\n",
			film_strip_frame->file_name
		);
	}
}
/*------------------------------------------------------------------------------

typedef struct film_strip_data
{
	bool used;
	char file_name[FILENAME_DIRENT_MAX];
	GtkWidget *image_widget;
	GtkWidget *mode_widget;
	GtkWidget *filename_widget;
	char dropped_text[FILENAME_DIRENT_MAX];
	int mode;
}
FILM_STRIP;__attribute__((unused))
	// copy to clipboard
	strcpy(clipboard_data->file_name, film_strip_frame->file_name);
	clipboard_data->mode = film_strip_frame->mode;
	clipboard_data->used = film_strip_frame->used;

------------------------------------------------------------------------------*/
void frame_selected
(
	GtkWidget *widget,
	__attribute__((unused))GdkEventButton *event,
	FILM_STRIP *film_strip_frame
)
{
	gtk_widget_grab_focus(widget);

	gui_printf
	(
		"frame\t\t: %d\n",
		film_strip_frame->frame
	);

	gui_printf
	(
		"clipboard\t: %s\n",
		clipboard_data->file_name
	);
}
/*------------------------------------------------------------------------------
https://stackoverflow.com/questions/54951379/gtk-3-native-file-chooser-allowed-file-types-and-default-file-name

	GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new("Choose a file:", GTK_WINDOW(window),
                        GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE,
                         GTK_RESPONSE_OK, GTK_STOCK_CANCEL,
                         GTK_RESPONSE_CANCEL, NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.cpp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "test.cpp");
	printf("%s %d\n", __FUNC__, __LINE__);
	printf("%s %d\n", __FUNC__, __LINE__);

	printf("%s %d\n", __FUNC__, __LINE__);
	printf("%s %d\n", __FUNC__, __LINE__);
    //gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "sequence.isf");
	printf("%s %d\n", __FUNC__, __LINE__);
	//printf("selected %s\n", isf_file_name);
printf("%s %d\n", __FUNC__, __LINE__);
printf("%s %d\n", __FUNC__, __LINE__);

------------------------------------------------------------------------------*/
void load_isf_file(void)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	gint res;
	char *filename;

	dialog = gtk_file_chooser_dialog_new
	(
		"Load isf file",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel",	GTK_RESPONSE_CANCEL,
		"_Open",	GTK_RESPONSE_ACCEPT,
		NULL
	);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.isf");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	// Set initial directory. If it doesnt exist "../images" is used.
	res = gtk_file_chooser_set_current_folder
	(
		GTK_FILE_CHOOSER(dialog), image_folder
	);

	if(!res)
		  res = gtk_file_chooser_set_current_folder
		  (
			GTK_FILE_CHOOSER(dialog), "../images"
		  );

	res = gtk_dialog_run(GTK_DIALOG(dialog));

printf
(
	"%s %d https://gitlab.gnome.org/GNOME/gtk/-/issues/2509\n",
	__FUNC__, __LINE__
);

	if(res == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);

		strn$cpy(isf_file_name, filename, FILENAME_MAX);

		g_free(filename);
	}

	gtk_widget_destroy(dialog);

	gui_printf("Loading %s\n", isf_file_name);

	load_isf(isf_file_name);
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void save_as_isf_file(void)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	char *isf_file_name;
	gint res;

	dialog = gtk_file_chooser_dialog_new
	(
		"Save isf file",
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL
	);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.isf");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	res = gtk_file_chooser_set_current_folder
	(
		GTK_FILE_CHOOSER(dialog), image_folder
	);

	if(!res)
	{
		// Set initial directory. If it doesnt exist "../images" is used.
		res = gtk_file_chooser_set_current_folder
		(
			GTK_FILE_CHOOSER(dialog), "../images"
		);

		printf
		(
			"failed to set image sequence folder: %s\n", image_folder
		);
	}

#if 0
	gtk_file_chooser_set_do_overwrite_confirmation
	(
		GTK_FILE_CHOOSER(dialog), TRUE
	);
#endif

	if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		isf_file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
		save_isf(isf_file_name);
	// make a copy in the images root folder with a generic name
		save_isf("/mnt/art_USB3/C/images/sequence.isf");
		g_free(isf_file_name);
	}

	gtk_widget_destroy (dialog);
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void save_isf_file(void)
{
	save_isf(isf_file_name);
	// make a copy in the images root folder with a generic name
	save_isf("/mnt/art_USB3/C/images/sequence.isf");
}
/*------------------------------------------------------------------------------

	printf("%s %d\n", __FUNC__, __LINE__);
	printf("%s %d\n", __FUNC__, __LINE__);
	printf("%s %d\n", __FUNC__, __LINE__);
	printf("%s %d\n", __FUNC__, __LINE__);
------------------------------------------------------------------------------*/
void set_image_folder(void)
{
	GtkWidget *dialog;
    gboolean ret = 0;
    int dlg_ret = 0;
    gchar *path = NULL;
	char command[5000];

	initialize_all_film_strip_data();	// new folder so clear all existing data

	dialog = gtk_file_chooser_dialog_new
	(
		"Select image folder",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		"_Cancel",	GTK_RESPONSE_CANCEL,
		"_Open",	GTK_RESPONSE_ACCEPT,
		NULL
	);

	// Set initial directory. If it doesnt exist "../" is used.
	ret = gtk_file_chooser_set_current_folder
	(
		GTK_FILE_CHOOSER(dialog), image_folder
	);

	if(!ret)
	{
		  ret = gtk_file_chooser_set_current_folder
		  (
			GTK_FILE_CHOOSER(dialog), "../"		// set to root ?
		  );
	}

    dlg_ret = gtk_dialog_run(GTK_DIALOG(dialog));

    if(dlg_ret == GTK_RESPONSE_ACCEPT)
    {
		path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		strn$cpy(image_folder, path, FILENAME_MAX);
    }

    if(dlg_ret == GTK_RESPONSE_CANCEL)
    {
		gui_printf("Selection: Canceled.\n");
    }
	else
	{
		gui_printf
		(
			"Image sequence folder: %s\n", image_folder
		);

		sprintf(command, "caja %s", image_folder);
		system(command);
	}

    gtk_widget_destroy(dialog);
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void gui_print_film_strip_data(void)
{
	for(int i = 1; i <= STRIP_COUNT; i++)
	{
		if(film_strip_data[i].used)
		{
			gui_printf
			(
				"%-60s in sequence number %2d\n",
				film_strip_data[i].file_name, i
			);
		}
	}
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void mode_entry_input(GtkWidget *widget)
{
	char widget_name[10];
	int strip_location;
	const gchar *entry_text;
	int mode;

	strcpy(widget_name, gtk_widget_get_name(widget));
	strip_location = atoi(widget_name);

	entry_text = gtk_entry_get_text(GTK_ENTRY(widget));
	mode = atoi(entry_text);

	film_strip_data[strip_location].mode = mode;
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void clean_up_uri_filename(char *raw_file_name, char *file_name)
{
	char *srce_ptr, *dest_ptr;

#if 0
	printf
	(
		"raw file name has been dropped:\n\t[%s]\n",
		raw_file_name
	);
#endif

	srce_ptr = raw_file_name + strlen("file://");	// skip the "file://"
	dest_ptr = file_name;

	while(*srce_ptr != '\n' && *srce_ptr != '\0')
	{
		if(*srce_ptr == '%')	// filename has space %20 ???
		{
			// likely %20
			*dest_ptr++ = ' ';			// insert a space
			srce_ptr += strlen("%20");	// move on past %20
		}
		else
		{
			*dest_ptr++ = *srce_ptr++;
		}
	}

	*dest_ptr = '\0';	// terminate the file_name
}
/*------------------------------------------------------------------------------
	call back when drop takes place

	https://docs.gtk.org/gtk3/method.SelectionData.get_text.html
	http://www.manpagez.com/html/gdk3/gdk3-3.14.5/gdk3-Drag-and-Drop.php
	https://developer.gimp.org/api/2.0/gtk/GtkWidget.html#gtk-widget-set-name

------------------------------------------------------------------------------*/
void drag_image_received
(
	GtkWidget          *widget,
	__attribute__((unused))GdkDragContext     *context,
	__attribute__((unused))gint                x,
	__attribute__((unused))gint                y,
	GtkSelectionData   *selection_data,
	__attribute__((unused))guint               info,
	__attribute__((unused))guint               time
)
{
	char *uri_filename, file_name[FILENAME_DIRENT_MAX];
	char widget_name[10];
	int frame;

	strcpy(widget_name, gtk_widget_get_name(widget));
	frame = atoi(widget_name);

	uri_filename = (char *)gtk_selection_data_get_text(selection_data);

	strcpy
	(
		film_strip_data[frame].dropped_text,
		uri_filename
	);

	clean_up_uri_filename(uri_filename, file_name);

	display_strip_image(frame, file_name);
}
/*------------------------------------------------------------------------------

https://cpp.hotexamples.com
/examples/-/-/gtk_selection_data_set
/cpp-gtk_selection_data_set-function-examples.html

------------------------------------------------------------------------------*/
void source_drag_image
(
	GtkWidget          *widget,
	__attribute__((unused))GdkDragContext     *context,
	GtkSelectionData   *selection_data,
	__attribute__((unused))guint               info,
	__attribute__((unused))guint               time,
	__attribute__((unused))gpointer            data
)
{
	char widget_name[10];
	int strip_location;
	guchar *dropped_text;

	strcpy(widget_name, gtk_widget_get_name(widget));
	strip_location = atoi(widget_name);
	dropped_text = (guchar *)film_strip_data[strip_location].dropped_text;

	gtk_selection_data_set
	(
		selection_data,
		gtk_selection_data_get_target(selection_data),
		8,	// bytes
		dropped_text, sizeof(dropped_text)
	);
}
/*------------------------------------------------------------------------------

		//image_strip[k] = gtk_image_new_from_pixbuf(initial_image(FRAME_HEIGHT));
------------------------------------------------------------------------------*/
#if 0
int zzmain(int argc, char *argv[])
{
	GtkWidget *save_isf_button, *load_isf_button, *image_folder_button;
	GtkWidget *image_strip[STRIP_COUNT];
	GtkWidget *mode_entry[STRIP_COUNT];
	GtkWidget *filename[STRIP_COUNT];
	GtkWidget *event_box[STRIP_COUNT];
	GtkWidget *console;
	int j = 0, k = 0;
	char widget_name[STRIP_COUNT];

	printf("Gtk+ version %d.%d\n", gtk_major_version, gtk_minor_version);
	gtk_init(&argc, &argv);
	window	= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	grid	= gtk_grid_new();
#if 1
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);
	gtk_container_add(GTK_CONTAINER(window), grid);

	image_folder_button	= gtk_button_new_with_label("Select image folder");
	load_isf_button		= gtk_button_new_with_label("Load Steenbeck isf");
	save_isf_button		= gtk_button_new_with_label("Save Steenbeck isf");
	console = install_console(FRAME_WIDTH, FRAME_WIDTH * 2.0, &console_text_view);

	printf("%s %d\n", __FUNC__, __LINE__);
	for(k = 0; k < STRIP_COUNT; k++)
	{
		event_box[k]	= gtk_event_box_new();
		image_strip[k]	= gtk_image_new();
		mode_entry[k]	= gtk_entry_new();
		filename[k]		= gtk_entry_new();

		//gtk_widget_add_events(image_strip[k], GDK_BUTTON_PRESS_MASK);
		//gtk_container_add(GTK_CONTAINER(event_box[k]), image_strip[k]);
		gtk_container_add(GTK_CONTAINER(grid), image_strip[k]);
		gtk_widget_add_events(event_box[k], GDK_BUTTON_PRESS_MASK);

		sprintf(widget_name, "%2d", k + 1);
		gtk_widget_set_name(image_strip[k], widget_name);
		gtk_widget_set_size_request(image_strip[k], FRAME_WIDTH, FRAME_HEIGHT);

		gtk_widget_set_name(mode_entry[k], widget_name);
		gtk_editable_set_editable(GTK_EDITABLE(mode_entry[k]), TRUE);
		// input text is center justified
		gtk_entry_set_alignment(GTK_ENTRY(mode_entry[k]), 0.5);
		//gtk_entry_set_text(GTK_ENTRY(mode_entry[k]), "mode");

		gtk_widget_set_name(filename[k], widget_name);
		gtk_editable_set_editable(GTK_EDITABLE(filename[k]), FALSE);
		// input text is center justified
		gtk_entry_set_alignment(GTK_ENTRY(filename[k]), 0.5);
		gtk_entry_set_text(GTK_ENTRY(filename[k]), "file name");

		film_strip_data[k + 1].image_widget		= image_strip[k];
		film_strip_data[k + 1].mode_widget		= mode_entry[k];
		film_strip_data[k + 1].filename_widget	= filename[k];

		//initialize_film_strip_entry(k + 1);
	}
#endif
	printf("%s %d\n", __FUNC__, __LINE__);

	initialize_all_film_strip_data();

//---------------- grid layout

	// four rows of five columns is what we have
	// there are three distinct items to be attached
	// that is where the 3's come in
#define ITEMS 3
	for(j = 0; j < STRIP_ROWS; j++)
	for(k = 0; k < STRIP_COLS; k++)	// STRIP_COLS = stride
	{
#if 0
		gtk_grid_attach
		(
			GTK_GRID(grid),
			image_strip[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 1,
			1, 1
		);
	//printf("%s %d\n", __FUNC__, __LINE__);
#endif

		gtk_grid_attach
		(
			GTK_GRID(grid),
			filename[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 2,
			1, 1
		);

		gtk_grid_attach
		(
			GTK_GRID(grid),
			mode_entry[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 3,
			1, 1
		);
	}
	printf("%s %d\n", __FUNC__, __LINE__);

										// left, top, span-width, span-height
	gtk_grid_attach(GTK_GRID(grid), console,				 7, 1, 3, 11);
#if 1
	gtk_grid_attach(GTK_GRID(grid), image_folder_button,	 7, 12, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), load_isf_button,		 8, 12, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), save_isf_button,		 9, 12, 1, 1);

#endif
//---------------- callback signal connections

	g_signal_connect
	(
		save_as_isf_button,
		"clicked",
		G_CALLBACK(save_as_isf_file),
		NULL
	);

	g_signal_connect
	(
		load_isf_button,
		"clicked",
		G_CALLBACK(load_isf_file),
		NULL
	);

	g_signal_connect
	(
		image_folder_button,
		"clicked",
		G_CALLBACK(set_image_folder),
		NULL
	);

	g_signal_connect
	(
		window,
		"destroy",
		G_CALLBACK(gtk_main_quit),
		NULL
	);

	for(k = 0; k < STRIP_COUNT; k++)
	{
		g_signal_connect			// now in Glib not GTK
		(
			image_strip[k],
			"drag_data_received",
			G_CALLBACK(drag_image_received),
			NULL
		);

		g_signal_connect
		(
			image_strip[k],
			"drag_data_get",
			G_CALLBACK(source_drag_image),
			NULL
		);

		g_signal_connect
		(
			event_box[k],
			"button_press_event",
			G_CALLBACK(frame_selected),
			&film_strip_data[k]
		);

		g_signal_connect
		(
			mode_entry[k],
			"changed",
			G_CALLBACK(mode_entry_input),
			NULL
		);
//---------------- final setup for drop destination

	for(k = 0; k < STRIP_COUNT; k++)
	{
		gtk_drag_dest_set
		(
			GTK_WIDGET(image_strip[k]),
			GTK_DEST_DEFAULT_ALL,
			target_table,
			G_N_ELEMENTS(target_table),
			GDK_ACTION_COPY
		);
	}

	gtk_widget_show_all(window);
	gui_printf("Gtk+ version %d.%d\n", gtk_major_version, gtk_minor_version);
	gtk_main();
	return EXIT_SUCCESS;
}
#endif
/*------------------------------------------------------------------------------

		//image_strip[k] = gtk_image_new_from_pixbuf(initial_image(FRAME_HEIGHT));
	GtkWidget *Steenbeck_isf_button;
	GtkWidget *input_directory_button, *output_directory_button;

		gtk_entry_set_width_chars(GTK_ENTRY(mode_entry[k]), 7);
		gtk_entry_set_alignment(GTK_ENTRY(mode_entry[k]), 1);

------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	GtkWidget *save_isf_button, *save_as_isf_button;
	GtkWidget *load_isf_button, *image_folder_button;
	GtkWidget *image_strip[STRIP_COUNT];
	GtkWidget *mode_entry[STRIP_COUNT];
	GtkWidget *filename[STRIP_COUNT];
	GtkWidget *event_box[STRIP_COUNT];
	GtkWidget *console;
	int k, j;
	char widget_name[255];

	printf("Gtk+ version %d.%d\n", gtk_major_version, gtk_minor_version);

	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_NONE);

	gtk_container_set_border_width(GTK_CONTAINER(window), 5);

	grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
	gtk_container_add(GTK_CONTAINER(window), grid);

	image_folder_button	= gtk_button_new_with_label("Select image folder");
	load_isf_button		= gtk_button_new_with_label("Load Steenbeck isf");
	save_as_isf_button	= gtk_button_new_with_label("Save as Steenbeck isf");
	save_isf_button		= gtk_button_new_with_label("Save Steenbeck isf");
	console = install_console(FRAME_WIDTH, 3 * FRAME_WIDTH, &console_text_view);

	for(k = 0; k < STRIP_COUNT; k++)
	{
		event_box[k]	= gtk_event_box_new();
		image_strip[k]	= gtk_image_new();
		mode_entry[k]	= gtk_entry_new();
		filename[k]		= gtk_entry_new();

		sprintf(widget_name, "%2d", k + 1);

		gtk_widget_set_name(event_box[k], widget_name);
		gtk_widget_set_name(image_strip[k], widget_name);
		gtk_container_add(GTK_CONTAINER(event_box[k]), image_strip[k]);
		gtk_widget_set_size_request(image_strip[k], FRAME_WIDTH, FRAME_HEIGHT);

		gtk_widget_set_name(mode_entry[k], widget_name);
		gtk_editable_set_editable(GTK_EDITABLE(mode_entry[k]), TRUE);
		// input text is center justified
		gtk_entry_set_alignment(GTK_ENTRY(mode_entry[k]), 0.5);

		gtk_editable_set_editable(GTK_EDITABLE(filename[k]), FALSE);
		// input text is center justified
		gtk_entry_set_alignment(GTK_ENTRY(filename[k]), 0.5);

		film_strip_data[k + 1].image_widget		= image_strip[k];
		film_strip_data[k + 1].mode_widget		= mode_entry[k];
		film_strip_data[k + 1].filename_widget	= filename[k];
	}

	initialize_all_film_strip_data();

//---------------- grid layout

	// four rows of five columns is what we have
	// there are three distinct items to be attached
	// that is where the 3's come in
#define ITEMS 3
	for(j = 0; j < STRIP_ROWS; j++)
	for(k = 0; k < STRIP_COLS; k++)	// STRIP_COLS = stride
	{
#if 1
		gtk_grid_attach
		(
			GTK_GRID(grid),
			event_box[k + j * STRIP_COLS],
			//image_strip[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 1,
			1, 1
		);
	//printf("%s %d\n", __FUNC__, __LINE__);
#endif

		gtk_grid_attach
		(
			GTK_GRID(grid),
			filename[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 2,
			1, 1
		);

		gtk_grid_attach
		(
			GTK_GRID(grid),
			mode_entry[k + j * STRIP_COLS],
			k + 1, j * ITEMS + 3,
			1, 1
		);
	}
										// left, top, span width, span height
	gtk_grid_attach(GTK_GRID(grid), console,				 7, 1,  4, 11);
	gtk_grid_attach(GTK_GRID(grid), image_folder_button,	 7, 12, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), load_isf_button,		 8, 12, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), save_isf_button,		 9, 12, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), save_as_isf_button,		10, 12, 1, 1);

#if 1
#endif

//---------------- callback signal connections

	g_signal_connect
	(
		save_isf_button,
		"clicked",
		G_CALLBACK(save_isf_file),
		NULL
	);

	g_signal_connect
	(
		save_as_isf_button,
		"clicked",
		G_CALLBACK(save_as_isf_file),
		NULL
	);

	g_signal_connect
	(
		load_isf_button,
		"clicked",
		G_CALLBACK(load_isf_file),
		NULL
	);

	g_signal_connect
	(
		image_folder_button,
		"clicked",
		G_CALLBACK(set_image_folder),
		NULL
	);

	g_signal_connect
	(
		window,
		"destroy",
		G_CALLBACK(gtk_main_quit),
		NULL
	);

	for(k = 0; k < STRIP_COUNT; k++)
	{
		g_signal_connect
		(
			image_strip[k],
			"drag-data-received",
			G_CALLBACK(drag_image_received),
			NULL
		);

		g_signal_connect
		(
			image_strip[k],
			"drag-data-get",
			G_CALLBACK(source_drag_image),
			NULL
		);

		g_signal_connect
		(
			mode_entry[k],
			"changed",
			G_CALLBACK(mode_entry_input),
			NULL
		);
#if 1
		g_signal_connect
		(
			event_box[k],
			"button-press-event",
			G_CALLBACK(frame_selected),
			&film_strip_data[k + 1]
		);
#endif
		g_signal_connect
		(
			event_box[k],
			"key-press-event",
			G_CALLBACK(keypress_function),
			&film_strip_data[k + 1]
		);
	}


//---------------- final setup for drop destination
	for(k = 0; k < STRIP_COUNT; k++)
	{
		gtk_drag_dest_set
		(
			GTK_WIDGET(image_strip[k]),
			GTK_DEST_DEFAULT_ALL,
			target_table,
			G_N_ELEMENTS(target_table),
			GDK_ACTION_COPY
		);
	}

	gtk_widget_show_all(window);

	for(k = 0; k < STRIP_COUNT; k++)
	{
		gtk_widget_set_can_focus
		(
			event_box[k],
			TRUE
		);
	}

	gui_printf("Gtk+ version %d.%d\n", gtk_major_version, gtk_minor_version);
	gtk_main();
	return EXIT_SUCCESS;
}
/*------------------------------------------------------------------------------
	http://ftp.math.utah.edu/u/ma/hohn/linux/gnome/developer.gnome.org/doc/
	tutorials/gnome-libs/x1003.html


------------------------------------------------------------------------------*/
