/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2023 Bhargav Mutyalapalli <<user@hostname.org>>
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
 * SECTION:element-verify
 *
 * FIXME:Describe verify here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! verify ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/base.h>
#include <gst/controller/controller.h>

#include "gstverify.h"

GST_DEBUG_CATEGORY_STATIC (gst_verify_debug);
#define GST_CAT_DEFAULT gst_verify_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
};

/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_verify_parent_class parent_class
G_DEFINE_TYPE (GstVerify, gst_verify, GST_TYPE_BASE_TRANSFORM);
GST_ELEMENT_REGISTER_DEFINE (verify, "verify", GST_RANK_NONE,
    GST_TYPE_VERIFY);

static void gst_verify_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_verify_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_verify_transform_ip (GstBaseTransform *
    base, GstBuffer * outbuf);
static GstCaps * gst_verify_transform_caps (GstBaseTransform * base,GstPadDirection direction, GstCaps * caps, GstCaps * filter);
/* GObject vmethod implementations */

/* initialize the verify's class */
static void
gst_verify_class_init (GstVerifyClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *trans_class;
  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_verify_set_property;
  gobject_class->get_property = gst_verify_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  gst_element_class_set_details_simple (gstelement_class,
      "Verify",
      "Generic/Filter",
      "FIXME:Generic Template Filter", "Bhargav Mutyalapalli <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_verify_transform_ip);
  trans_class = GST_BASE_TRANSFORM_CLASS (klass);
  trans_class->transform_caps =GST_DEBUG_FUNCPTR (gst_verify_transform_caps);
  /* debug category for fltering log messages
   *
   * FIXME:exchange the string 'Template verify' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_verify_debug, "verify", 0,
      "Template verify");
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_verify_init (GstVerify * filter)
{
  filter->silent = FALSE;
}

static void
gst_verify_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVerify *filter = GST_VERIFY (object);

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
gst_verify_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstVerify *filter = GST_VERIFY (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_verify_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstVerify *filter = GST_VERIFY (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");

  /* FIXME: do something interesting here.  This simply copies the source
   * to the destination. */

  return GST_FLOW_OK;
}

static GstCaps * gst_verify_transform_caps (GstBaseTransform * base,GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *retir=NULL;
  if(direction == GST_PAD_SRC)
  {
  g_print("SRC direction IS INVOKED\n");
  gchar *str1=gst_caps_to_string(caps);
  g_print("The caps in src:%s\n",str1);
  gchar *str2=gst_caps_to_string(filter);
  g_print("The filter in src:%s\n",str2);
  }
  if(direction == GST_PAD_SINK)
  {
  g_print("SINK direction IS INVOKED\n");
  gchar *str1=gst_caps_to_string(caps);
  g_print("The caps in src:%s\n",str1);
  gchar *str2=gst_caps_to_string(filter);
  g_print("The filter in src:%s\n",str2);
  }
  return caps;
}
/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
verify_init (GstPlugin * verify)
{
  return GST_ELEMENT_REGISTER (verify, verify);
}

/* gstreamer looks for this structure to register verifys
 *
 * FIXME:exchange the string 'Template verify' with you verify description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    verify,
    "verify",
    verify_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
