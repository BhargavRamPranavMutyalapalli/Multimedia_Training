#include <gst/gst.h>

int main(int argc, char *argv[]) {
  GstElement *pipeline, *bin, *equalizer, *convert, *sink;
  GstPad *pad, *ghost_pad;
  GstBus *bus;
  GstMessage *msg;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Build the pipeline */
  pipeline = gst_parse_launch ("playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

  /* Create the elements inside the sink bin */
  equalizer = gst_element_factory_make ("equalizer-3bands", "equalizer");
  convert = gst_element_factory_make ("audioconvert", "convert");
  sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
  if (!equalizer || !convert || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Create the sink bin, add the elements and link them */
  bin = gst_bin_new ("audio_sink_bin");
  gst_bin_add_many (GST_BIN (bin), equalizer, convert, sink, NULL);
  gst_element_link_many (equalizer, convert, sink, NULL);
  gst_debug_bin_to_dot_file_with_ts(GST_BIN(bin), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
  pad = gst_element_get_static_pad (equalizer, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_debug_bin_to_dot_file_with_ts(GST_BIN(bin), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);
  //gst_debug_bin_to_dot_file_with_ts(GST_BIN(bin), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
  gst_object_unref (pad);

  /* Configure the equalizer */
  g_object_set (G_OBJECT (equalizer), "band1", (gdouble)-24.0, NULL);
  g_object_set (G_OBJECT (equalizer), "band2", (gdouble)-24.0, NULL);

  /* Set playbin's audio sink to be our sink bin */
  g_object_set (GST_OBJECT (pipeline), "audio-sink", bin, NULL);

  /* Start playing */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Free resources */
  if (msg != NULL)
    gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
