#include <gst/gst.h>

int main (int   argc,char *argv[])
{
	GstElement *pipeline;
	GstElement *source, *filter, *sink;

	/* init */
	gst_init (&argc, &argv);

	/* create pipeline */
	pipeline = gst_pipeline_new ("my-pipeline");

	/* create elements */
	source = gst_element_factory_make ("fakesrc", "source");
	filter = gst_element_factory_make ("identity", "filter");
	sink = gst_element_factory_make ("fakesink", "sink");

	/* must add elements to pipeline before linking them */
	gst_bin_add_many (GST_BIN (pipeline), source, filter, sink, NULL);

	/* link */
	if (!gst_element_link_many (source, filter, sink, NULL)) {
		g_warning ("Failed to link elements!");
	}
	gst_debug_bin_to_dot_file_with_ts(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

}

