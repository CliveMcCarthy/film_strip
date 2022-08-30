# film_strip
film-strip and contact-sheet in Gtk

![Screenshot at 2022-08-30 16-35-22](https://user-images.githubusercontent.com/10423377/187561887-13458f1c-ee2c-4194-9e98-9cd60981511b.png)

	This program is a cross between a film-strip editor and a contact-sheet.
	It presents an array of still image frames into which images can be
	dragged-and-dropped from a file manager such as Mint Mate's Caja.

	The program is named steenbeck but is in no way associated with:
	https://www.steenbeck.com/ so don't call or blame those guys.

	The program's output is a simple list of the image's file names which can
	be used by other software. The program also read these sequenece text files
	(image sequence files *.isf). The files are parsed in a fairly crude manner.

	Image frames can be cut/copy/paste/deleted with the usual ctrl-x, ctrl-c,
	ctrl-v, and Del keystrokes.

	I had hoped to get drag-and-drop to work within the program but, alas,
	I couldn't figure it out. The program is designed to be used on 4K displays
	but by changing a few constants in steenback.h it can be made to work on
	smaller screens.
	
	I have published this aspect of the program to show how Gtk can be used.
	And, in a way, to thank the various open source resources I used to
	write the program. I should also thank Albrecht Dürer (1471–1528)
	for the rhinocerous -- but he is dead.

	In general, Gtk documentation sucks. The official on-line stuff seems to be
	just an auto-generated bunch of formal, object oriented, lists without any
	prototypes and code examples. It's quite inferior to the Linux documentation
	you will find such as: 'The Linux Programming Interface' by Michael Kerrisk.
