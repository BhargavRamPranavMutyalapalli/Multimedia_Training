#include <stdio.h>
#include <gst/gst.h>

int main (int   argc,char *argv[])
{
	const gchar *nano_str;
	guint major, minor, micro, nano;

	gst_init (&argc, &argv);

	gst_version (&major, &minor, &micro, &nano);



	if (nano == 1)
		nano_str = "(CVS)";
	else if (nano == 2)
		nano_str = "(Prerelease)";
	else
		nano_str = "";

	printf ("This program is linked against GStreamer %d.%d.%d %s\n",major, minor, micro, nano_str);
	printf("The major:%d\n",GST_VERSION_MAJOR);
	printf("The minor:%d\n",GST_VERSION_MINOR);
	printf("The micro:%d\n",GST_VERSION_MICRO);
	gst_deinit();
	return 0;
}
