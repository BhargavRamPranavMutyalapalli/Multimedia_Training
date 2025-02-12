/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2023 Bhargav Mutyalapalli <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-setcaps
 *
 * FIXME:Describe setcaps here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! setcaps ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <libswscale/swscale.h>

#include "gstsetcaps.h"

GST_DEBUG_CATEGORY_STATIC (gst_setcaps_debug);
#define GST_CAT_DEFAULT gst_setcaps_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_setcaps_parent_class parent_class
G_DEFINE_TYPE (GstSetcaps, gst_setcaps, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (setcaps, "setcaps", GST_RANK_NONE,
    GST_TYPE_SETCAPS);

static void gst_setcaps_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_setcaps_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_setcaps_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_setcaps_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);
static void rescale_video_frame(int in_width, int in_height, uint8_t *in_data, int target_width, int target_height, uint8_t *out_data);

/* GObject vmethod implementations */

/* initialize the setcaps's class */
static void
gst_setcaps_class_init (GstSetcapsClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_setcaps_set_property;
  gobject_class->get_property = gst_setcaps_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple (gstelement_class,
      "Setcaps",
      "FIXME:Generic",
      "FIXME:Generic Template Element", "Bhargav Mutyalapalli <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_setcaps_init (GstSetcaps * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_setcaps_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_setcaps_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
  filter->target_width = 1920;
  filter->target_height = 1080;
  filter->target_format = GST_VIDEO_FORMAT_YV12;
}

static void
gst_setcaps_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSetcaps *filter = GST_SETCAPS (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_setcaps_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstSetcaps *filter = GST_SETCAPS (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_setcaps_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstSetcaps *filter;
  gboolean ret;

  filter = GST_SETCAPS (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}
// Function to resize and scale a video frame
static void rescale_video_frame(int in_width, int in_height, uint8_t *in_data, int target_width, int target_height, uint8_t *out_data) {
    // Create the SwsContext for scaling
    struct SwsContext *sws_ctx = sws_getContext(in_width, in_height, AV_PIX_FMT_YUV420P,
        target_width, target_height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    // Perform the scaling operation
    sws_scale(sws_ctx, (const uint8_t *const *)&in_data, &in_width, 0, in_height, &out_data, &target_width);

    // Release the SwsContext
    sws_freeContext(sws_ctx);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_setcaps_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstSetcaps *filter;

  filter = GST_SETCAPS (parent);

  if (filter->silent == FALSE)
	  g_print ("I'm plugged, therefore I'm in.\n");
// Get the input video info
    GstVideoInfo in_info, out_info;
    gst_video_info_from_caps(&in_info, gst_pad_get_current_caps(filter->sinkpad));
    gst_video_info_from_caps(&out_info, gst_pad_get_current_caps(filter->srcpad));

    // Check if the input and output formats are YV12
    if (GST_VIDEO_INFO_FORMAT(&in_info) != GST_VIDEO_FORMAT_YV12 ||
        GST_VIDEO_INFO_FORMAT(&out_info) != GST_VIDEO_FORMAT_YV12) {
        GST_ELEMENT_ERROR(filter, STREAM, NOT_IMPLEMENTED, ("Unsupported video format"), (NULL));
        return GST_FLOW_ERROR;
    }

    // Get the dimensions of the input frame
    gint in_width = GST_VIDEO_INFO_WIDTH(&in_info);
    gint in_height = GST_VIDEO_INFO_HEIGHT(&in_info);

    // Create the output video info with the target dimensions
    GstVideoInfo out_info_scaled;
    gst_video_info_set_format(&out_info_scaled, filter->target_format, filter->target_width, filter->target_height);

    // Create the output buffer with the target dimensions
    GstBuffer *out_buffer = gst_buffer_new_allocate(NULL, GST_VIDEO_INFO_SIZE(&out_info_scaled), NULL);
    GstMapInfo in_map_info, out_map_info;
    gst_buffer_map(buf, &in_map_info, GST_MAP_READ);
    gst_buffer_map(out_buffer, &out_map_info, GST_MAP_WRITE);

    // Implement your resizing and scaling logic here
    rescale_video_frame(in_width, in_height, in_map_info.data, filter->target_width, filter->target_height, out_map_info.data);

    //gst_buffer_unmap(inbuf, &in_map_info);
    //gst_buffer_unmap(out_buffer, &out_map_info);




  return gst_pad_push (filter->srcpad, out_buffer);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean setcaps_init (GstPlugin * setcaps)
{
	/* debug category for filtering log messages
	 *
	 * exchange the string 'Template setcaps' with your description
	 */
	GST_DEBUG_CATEGORY_INIT (gst_setcaps_debug, "setcaps",
			0, "Template setcaps");

	return GST_ELEMENT_REGISTER (setcaps, setcaps);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstsetcaps"
#endif

/* gstreamer looks for this structure to register setcapss
 *
 * exchange the string 'Template setcaps' with your setcaps description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    setcaps,
    "setcaps",
    setcaps_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
