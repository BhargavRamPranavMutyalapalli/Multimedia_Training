#include <gst/gst.h>

static GMainLoop *loop;
GstElement *pipeline, *appsrc, *conv, *videosink;

static gboolean push_data(gpointer user_data)
{
	static gboolean white = FALSE;
	static GstClockTime timestamp = 0;
	GstBuffer *buffer;
	guint size;
	GstFlowReturn ret;

	size = 385 * 288*2;

	buffer = gst_buffer_new_allocate (NULL, size, NULL);

	/* this makes the image black/white */
	gst_buffer_memset (buffer, 0, white ? 0x2A : 0x0, size);

	white = !white;

	GST_BUFFER_PTS (buffer) = timestamp;
	GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

	timestamp += GST_BUFFER_DURATION (buffer);

	g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
	gst_buffer_unref (buffer);

	if (ret != GST_FLOW_OK) {
		/* something wrong, stop pushing */
		g_main_loop_quit (loop);
	}
	return TRUE;
}
static void cb_need_data (GstElement *appsrc,guint unused_size,gpointer user_data)
{
	g_print("Need data requested. Pushing more data...\n");

	// Schedule the push_data function to be called
	g_idle_add(push_data, NULL);
}
void enough_data_callback(GstElement* element, gpointer user_data) {
	// Your code to handle the "enough-data" event goes here
	g_print("enough data is invoked\n");
}


gint main (gint argc,gchar *argv[])
{

	/* init GStreamer */
	gst_init (&argc, &argv);
	loop = g_main_loop_new (NULL, FALSE);

	/* setup pipeline */
	pipeline = gst_pipeline_new ("pipeline");
	appsrc = gst_element_factory_make ("appsrc", "source");
	conv = gst_element_factory_make ("videoconvert", "conv");
	//videosink = gst_element_factory_make ("xvimagesink", "videosink");
	videosink = gst_element_factory_make ("autovideosink", "videosink");

	/* setup */
	g_object_set (G_OBJECT (appsrc), "caps",
			gst_caps_new_simple ("video/x-raw",
				"format", G_TYPE_STRING, "RGB16",
				"width", G_TYPE_INT, 384,
				"height", G_TYPE_INT, 288,
				"framerate", GST_TYPE_FRACTION, 0, 1,
				NULL), NULL);
	gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
	gst_element_link_many (appsrc, conv, videosink, NULL);

	/* setup appsrc */
	g_object_set (G_OBJECT (appsrc),
			"stream-type", 0,
			"format", GST_FORMAT_TIME, NULL);
	g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
	g_signal_connect(appsrc, "enough-data", G_CALLBACK(enough_data_callback), NULL);


	/* play */
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	g_main_loop_run (loop);

	/* clean up */
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));
	g_main_loop_unref (loop);

	return 0;
}
