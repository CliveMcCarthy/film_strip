/*------------------------------------------------------------------------------


------------------------------------------------------------------------------*/
#include "common_header.h"
#include "lip_image_processing.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "steenbeck.h"

/*------------------------------------------------------------------------------
#include "durer-rhinoceros-engraving-1515.xpm"
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
extern FILM_STRIP film_strip_data[STRIP_COUNT + 1];
#define STRIP_COUNT				  20
#define TEXT_BUFFER_SIZE		4097
#define FRAME_WIDTH				 300
#define FRAME_HEIGHT			 300

#define ISF_TERMINATOR	99
#define ISF_GESSO		98
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------

	strn$cpy(path, ++char_ptr, PATH_MAX);
	//printf("%s = %s\n", file_path, file);
char *delete_filename_from_path(const char *file)
{
	char *char_ptr;
	static char path[PATH_MAX + 1];

	strn$cpy(path, file, PATH_MAX);
	char_ptr = path;

	while(*char_ptr != '\0')	// find the end
	{
		char_ptr++;
	}

	// delete everything until forward or back slash or start of the path
	while(*char_ptr != '\\' && char_ptr >= path)
	{
		*char_ptr = '\0';
		char_ptr--;
	}

	return path;
}
------------------------------------------------------------------------------*/
void print_film_strip_data(void)
{
	for(int i = 1; i <= STRIP_COUNT; i++)
	{
		if(film_strip_data[i].used)
		{
			printf
			(
				"%s\n",
				film_strip_data[i].file_name
			);
		}
	}
}
/*------------------------------------------------------------------------------
	typical delimiters[] = "\t\n\v\f\r, #"	same as isspace but with ,# too
------------------------------------------------------------------------------*/
void parse_parameter(char **input_txt_ptr, char *delimiters, char *parsed_text)
{
	char *output_ptr, *input_ptr;

	// skip leading whitespace
	while((isspace(**input_txt_ptr) && **input_txt_ptr != '\0'))
	{
		(*input_txt_ptr)++;
	}

	output_ptr	= parsed_text;
	while(**input_txt_ptr != '\0')
	{
		*output_ptr = **input_txt_ptr;
		output_ptr++;
		(*input_txt_ptr)++;

		if(strchr(delimiters, **input_txt_ptr) != NULL)
		{
			*output_ptr = '\0';
			break;
		}
	}

	// find the end of the parsed text
	input_ptr = parsed_text;
	while(*input_ptr != '\0') input_ptr++;
	input_ptr--;	// step back past the '\0'

	// back scan setting all whitespace to '\0'
	while(isspace(*input_ptr) && input_ptr >= parsed_text)
	{
		*input_ptr = '\0';
	}

	//printf("parsed_parameter\t: |%s|\n", parsed_text);
}
/*------------------------------------------------------------------------------

		//input_ptr--;
	//parse_parameter(&input_txt_ptr, " \n", comment);
	//printf("\n");
			printf("parsing\t: |%s|\n", input_txt_ptr);
 || *input_txt_ptr == '\"'
------------------------------------------------------------------------------*/
void isf_parse_sequence_entry
(
	char *raw_line, char *frame, char *mode, char *file_name, char *comment
)
{
	char *input_txt_ptr, *output_txt_ptr;
	char buffer_line[4096];

	// initialize the buffer string
	CLEAR(buffer_line);
	*mode = *file_name = *comment = '\0';

	input_txt_ptr	= raw_line;

	// truncate the raw_line at the first comment character
	input_txt_ptr	= raw_line;
	output_txt_ptr	= buffer_line;
	while(*input_txt_ptr != '\0')
	{
		if(*input_txt_ptr == '#')
		{
			*output_txt_ptr = '\0';
			break;
		}

		*output_txt_ptr++ = *input_txt_ptr++;
	}

	input_txt_ptr	= buffer_line;

	if(*input_txt_ptr != '\0')	// don't parse an empty line
	{
		parse_parameter(&input_txt_ptr, " \n", frame);
		parse_parameter(&input_txt_ptr, " \n", mode);

		// spaces are permitted in filenames
		parse_parameter(&input_txt_ptr, "\n", file_name);
	}
}
/*------------------------------------------------------------------------------

	CLEAR(frame);
	CLEAR(mode);
	CLEAR(file_name);
------------------------------------------------------------------------------*/
int load_isf
(
	char *list_filename
)
{
	char frame[255], mode[255], file_name[255];
	char comment[255];
	char raw_line[PATH_MAX + 1];
	FILE *fp_isf;
	int entry_count = 0, frame_number = 0;
	GtkWidget *mode_widget, *filename_widget;

	char drive[5];
	char path[PATH_MAX + 1];
	char relative_path[PATH_MAX + 1];
	char ext[5];

	//initialize_film_strip_data();

	if((fp_isf = fopen(list_filename, "rt")) == NULL)
	{
		gui_program_error_dialog_box(__FILE__, __LINE__, __FUNC__,
			"can't open image_sequence file: '%s'", list_filename);
	}
	else
	{
		// get the path to the isf
		file_name_split(list_filename, drive, path, file_name, ext);

		while(!feof(fp_isf))
		{
			fgets(raw_line, 512, fp_isf);
			isf_parse_sequence_entry
			(
				raw_line, frame, mode, file_name, comment
			);

			frame_number = atoi(frame);

			if(frame_number == 0
			|| frame_number == ISF_TERMINATOR
			|| *file_name == '\0'
			|| frame_number > STRIP_COUNT)
			{
				// the line is empty or a comment
				// or is out of bounds
			}
			else
			{
#if 0
				printf
				(
					"isf entry\t: %2d %2d %s\n",
					frame_number, atoi(mode), file_name
				);
#endif

				film_strip_data[frame_number].mode = atoi(mode);

				file_name_split	// get the filename and extension
				(
					file_name,
					drive, relative_path, file_name, ext
				);

				file_name_merge
				(
					film_strip_data[frame_number].file_name,
					drive, path, file_name, ext
				);
#if 0
				printf
				(
					"processing %2d |%s|\n",
					frame_number,
					film_strip_data[frame_number].file_name
				);
#endif

				filename_widget = film_strip_data[frame_number].filename_widget;
				gtk_entry_set_text
				(
					GTK_ENTRY(filename_widget),
					film_strip_data[frame_number].file_name
				);

				mode_widget = film_strip_data[frame_number].mode_widget;
				gtk_entry_set_text
				(
					GTK_ENTRY(mode_widget), (const gchar *)mode
				);

				display_strip_image
				(
					frame_number,
					film_strip_data[frame_number].file_name
				);

			}
		}

		fclose(fp_isf);
	}
	return entry_count;
}
/*------------------------------------------------------------------------------
__attribute__((unused))
	//sprintf(file_path_name, "%s/sequence.isf", image_folder);
	sprintf(file_path_name, "%s/%s", image_folder, list_filename);
	char file_path_name[5000];
	gui_printf
	(
		"sequence file %s\n", file_path_name
	);

	gui_printf
	(
		"** sequence file %s\n", list_filename
	);

------------------------------------------------------------------------------*/
void save_isf(char *list_filename)
{
	FILE *fp_isf;
	char *prefix_ptr;

	if((fp_isf = fopen(list_filename, "wt")) == NULL)
	{
		gui_program_error_dialog_box(__FILE__, __LINE__, __FUNC__,
			"can't open image_sequence file:\n\t%s",
			list_filename);
	}
	else
	{
		gui_printf
		(
			"Saving sequence file %s\n", list_filename
		);

		fprintf
		(
			fp_isf,
			"#\n"
			"#\n"
			"# Steenbeck sequence\n"
			"# %s\n"
			"#\n"
			"#\n"
			"#\n",
			list_filename
		);

		for(int i = 1; i <= STRIP_COUNT; i++)
		{
			if(film_strip_data[i].used)
			{
				prefix_ptr = strstr(film_strip_data[i].file_name, "images");

				if(prefix_ptr != NULL)
				{
					fprintf
					(
						fp_isf,
						"%2d %2d ../%s # %2d\n",
						i,
						film_strip_data[i].mode,
						prefix_ptr,
						i
					);

					gui_printf
					(
						"%2d %2d ../%s # %2d\n",
						i,
						film_strip_data[i].mode,
						prefix_ptr,
						i
					);
				}
				else
				{
					gui_program_warning_dialog_box
					(
						__FILE__, __LINE__, __FUNC__,
						"image %s is not in the master image collection",
						film_strip_data[i].file_name
					);
				}
			}
		}

		fprintf
		(
			fp_isf,
			"#\n"
			"%d %d ./common/Philips_PM5644_3480x2160.tif\n"
			"#\n"
			"# end of sequence\n",
			ISF_TERMINATOR, ISF_TERMINATOR
		);

		fclose(fp_isf);
	}
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
