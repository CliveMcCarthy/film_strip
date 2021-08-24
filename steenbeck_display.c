/*------------------------------------------------------------------------------

void gui_update(void)
{
	// this yields execution to the GUI loop if any events are pending
	while(gtk_events_pending()) gtk_main_iteration();
};

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
	https://rosettacode.org/wiki/Convert_decimal_number_to_rational

	f : number to convert.
	num, denom: returned parts of the rational.

	md: max denominator value.  Note that machine floating point number
	has a finite resolution (10e-16 ish for 64 bit double), so specifying
	a "best match with minimal error" is often wrong, because one can
	always just retrieve the significand and return that divided by
	2**52, which is in a sense accurate, but generally not very useful:
	1.0/7.0 would be "2573485501354569/18014398509481984", for example.
------------------------------------------------------------------------------*/
void rational_approx(double f, int64_t md, int64_t *num, int64_t *denom)
{
	/*  a: continued fraction coefficients. */
	int64_t a, h[3] = { 0, 1, 0 }, k[3] = { 1, 0, 0 };
	int64_t x, d, n = 1;
	int i, neg = 0;

	if (md <= 1) { *denom = 1; *num = (int64_t) f; return; }

	if (f < 0) { neg = 1; f = -f; }

	while (f != floor(f))
	{
		n <<= 1; f *= 2;
	}
	d = f;

	/* continued fraction and check denominator each step */
	for (i = 0; i < 64; i++)
	{
		a = n ? d / n : 0;
		if (i && !a) break;

		x = d; d = n; n = x % n;

		x = a;
		if (k[1] * a + k[0] >= md)
		{
			x = (md - k[0]) / k[1];
			if (x * 2 >= a || k[1] >= md)
				i = 65;
			else
				break;
		}

		h[2] = x * h[1] + h[0]; h[0] = h[1]; h[1] = h[2];
		k[2] = x * k[1] + k[0]; k[0] = k[1]; k[1] = k[2];
	}
	*denom = k[1];
	*num = neg ? -h[1] : h[1];
}
/*------------------------------------------------------------------------------
		crop to a square image and scale to frame size
------------------------------------------------------------------------------*/
IMAGE *format_image_for_film_strip_data(IMAGE *image)
{
	IMAGE *scaled;
	int x1, y1, x2, y2;
	double aspect;
	int64_t num, denom;

	aspect = (double)image->length / image->width;
	rational_approx(aspect, 20, &num, &denom);

#if 0
	printf
	(
		"%-40s pre-format\n\tw=%4d h=%4d "
		"aspect=%0.3lf %-16s %2ldx%ld\n",
		image->name,
		image->width, image->length, aspect,
		lip_image_orientation_string(image->orientation),
		denom, num
	);
#endif

	switch(image->orientation)
	{
		case HORIZONTAL: 	// crop to square but don't rotate
		if(image->width < image->length)
		{
			// crop to the top
			x1 = 0;
			x2 = image->width - 1;

			y1 = 0;
			y2 = image->width;
		}
		else
		{
			// crop to the middle
			x1 = (image->width - image->length) / 2;
			x2 = (image->width + image->length) / 2;
			y1 = 0;
			y2 = image->length - 1;
		}

		lip_crop_image_coordinates(image, x1, y1, x2, y2, NULL);
		break;

		case ROTATE_90_CW:
		x1 = 0;
		y1 = 0;
		x2 = image->length - 1;
		y2 = image->length - 1;
		lip_crop_image_coordinates(image, x1, y1, x2, y2, NULL);

		lip_rotate_image_style
		(
			image, ROTATE_90_CW, NULL
		);
		break;

		case ROTATE_270_CW:
		x1 = image->width - image->length;
		y1 = 0;
		x2 = image->width  - 1;
		y2 = image->length - 1;
		lip_crop_image_coordinates(image, x1, y1, x2, y2, NULL);

#if 0
	printf
	(
		"%-40s post-crop format w=%4d h=%4d aspect=%0.3lf\n",
		image->name,
		image->width, image->length,
		(double)image->length / image->width
	);
#endif

		lip_rotate_image_style
		(
			image, ROTATE_270_CW, NULL
		);
		break;

		default:	// these are not from a camera
		gui_program_warning_dialog_box(__FILE__, __LINE__, __FUNC__,
			"image %s\n\thas odd orientation %d",
				image->name, image->orientation);
		break;
	}

	if(image->width < FRAME_WIDTH || image->length < FRAME_HEIGHT)
	{
		gui_program_warning_dialog_box(__FILE__, __LINE__, __FUNC__,
			"image %s\n\tis very small",
			image->name);
	}

	lip_image_resample
	(
		image,
		FRAME_WIDTH,
		FRAME_HEIGHT,
		&scaled
	);

#if 0
	printf
	(
		"%-40s post-format w=%4d h=%4d aspect=%0.3lf\n",
		image->name,
		scaled->width, scaled->length,
		(double)scaled->length / scaled->width
	);
#endif

	return scaled;
}
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
void display_strip_image
(
	int frame,
	char *file_name
)
{
	IMAGE *image = NULL, *formatted = NULL;
	GdkPixbuf *pixbuf_image;
	GtkWidget *image_widget;
	GtkWidget *filename_widget;
	int pending;

#if 0
	printf
	(
		"showing %-40s in %2d\n",
		file_name,
		frame
	);
#endif

	image_widget	= film_strip_data[frame].image_widget;
	filename_widget	= film_strip_data[frame].filename_widget;

	image = lip_read_image
	(
		file_name
	);

	if(image != NULL)
	{
		formatted = format_image_for_film_strip_data(image);

		pixbuf_image = gdk_pixbuf_new_from_data
		(
			formatted->buffer,
			GDK_COLORSPACE_RGB,
			TRUE, 8,			// has alpha (RGBA) RGB is 8 bits per channel
			FRAME_WIDTH,
			FRAME_HEIGHT,
			formatted->width * 4,	// row stride byte count

			NULL,
			NULL
		);

		gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), pixbuf_image);
		pending = 0;

		while(gtk_events_pending())
		{
			//printf("gtk_events_pending() %d\n", pending++);
			//sleep(1);	// 10ms
			gtk_main_iteration();
			usleep(10000);	// 10ms
		}

		film_strip_data[frame].used = TRUE;
		strcpy(film_strip_data[frame].file_name, file_name);

		gtk_entry_set_alignment(GTK_ENTRY(filename_widget), 0.5);	// center
		gtk_entry_set_text
		(
			GTK_ENTRY(filename_widget),
			image->name
		);

#if 0
		gui_printf
		(
			"displaying %-40s %-40s in %s frame %2d\n",
			film_strip_data[frame].file_name,
			file_name,
			gtk_widget_get_name(image_widget),
			frame
		);
#endif

		lip_delete_image(image);
		lip_delete_image(formatted);
	}
	else
	{
		gui_program_warning_dialog_box
		(
			__FILE__, __LINE__, __FUNC__,
			"can't read image:\n\t%s", file_name
		);
	}
}
/*------------------------------------------------------------------------------
			//gtk_main_iteration();

------------------------------------------------------------------------------*/
