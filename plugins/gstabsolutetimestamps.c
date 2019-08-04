/*
 * Copyright (C) 2019 George Hawkins <https://github.com/george-hawkins>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * SECTION:element-gstabsolutetimestamps
 *
 * The absolutetimestamps element generates a mapping from relative timestamps to absolute timestamps.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 videotestsrc num-buffers=120 ! 'video/x-raw,width=1024,height=768,framerate=24/1' ! clockoverlay ! absolutetimestamps ! jpegenc ! avimux ! filesink sync=true location=out.avi
 * ]|
 * Capture a mapping from relative timestamps to absolute timestamps so that frames can later be extracted by the absolute time at which they were captured (i.e. the time recorded by clockoverlay).
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gstdio.h>

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstabsolutetimestamps.h"

GST_DEBUG_CATEGORY_STATIC (gst_absolutetimestamps_debug_category);
#define GST_CAT_DEFAULT gst_absolutetimestamps_debug_category

#define DEFAULT_FILENAME "timestamps.log"

/* prototypes */


static void gst_absolutetimestamps_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_absolutetimestamps_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_absolutetimestamps_dispose (GObject * object);

static gboolean gst_absolutetimestamps_accept_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps);
static gboolean gst_absolutetimestamps_start (GstBaseTransform * trans);
static gboolean gst_absolutetimestamps_stop (GstBaseTransform * trans);
static GstFlowReturn gst_absolutetimestamps_transform_ip (GstBaseTransform *
    trans, GstBuffer * buf);

enum
{
  PROP_0,
  PROP_LOCATION
};

/* pad templates */

static GstStaticPadTemplate gst_absolutetimestamps_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );

static GstStaticPadTemplate gst_absolutetimestamps_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAbsolutetimestamps, gst_absolutetimestamps,
    GST_TYPE_BASE_TRANSFORM,
    GST_DEBUG_CATEGORY_INIT (gst_absolutetimestamps_debug_category,
        "absolutetimestamps", 0,
        "debug category for absolutetimestamps element"));

static void
gst_absolutetimestamps_class_init (GstAbsolutetimestampsClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class =
      GST_BASE_TRANSFORM_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS (klass),
      &gst_absolutetimestamps_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS (klass),
      &gst_absolutetimestamps_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "Absolutetimestamps", "Generic", "Generate a mapping from relative timestamps to absolute timestamps",
      "George Hawkins <https://github.com/george-hawkins>");

  gobject_class->set_property = gst_absolutetimestamps_set_property;
  gobject_class->get_property = gst_absolutetimestamps_get_property;

  g_object_class_install_property (gobject_class, PROP_LOCATION,
      g_param_spec_string ("location", "File Location",
          "Location of the timestamp mapping file to write", DEFAULT_FILENAME,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gobject_class->dispose = gst_absolutetimestamps_dispose;
  base_transform_class->accept_caps =
      GST_DEBUG_FUNCPTR (gst_absolutetimestamps_accept_caps);
  base_transform_class->start =
      GST_DEBUG_FUNCPTR (gst_absolutetimestamps_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_absolutetimestamps_stop);
  base_transform_class->transform_ip =
      GST_DEBUG_FUNCPTR (gst_absolutetimestamps_transform_ip);

}

static void
gst_absolutetimestamps_init (GstAbsolutetimestamps * absolutetimestamps)
{
  absolutetimestamps->filename = g_strdup(DEFAULT_FILENAME);
  absolutetimestamps->file = NULL;
}

void
gst_absolutetimestamps_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (object);

  GST_DEBUG_OBJECT (absolutetimestamps, "set_property");

  const gchar *location;

  switch (property_id) {
    case PROP_LOCATION:
	  location = g_value_get_string (value);
      g_free (absolutetimestamps->filename); // Free the value created in gst_absolutetimestamps_init.
      absolutetimestamps->filename = g_strdup (location);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_absolutetimestamps_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (object);

  GST_DEBUG_OBJECT (absolutetimestamps, "get_property");

  switch (property_id) {
    case PROP_LOCATION:
      g_value_set_string (value, absolutetimestamps->filename);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_absolutetimestamps_dispose (GObject * object)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (object);

  GST_DEBUG_OBJECT (absolutetimestamps, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_absolutetimestamps_parent_class)->dispose (object);

  g_free (absolutetimestamps->filename);
  absolutetimestamps->filename = NULL;
}

static gboolean
gst_absolutetimestamps_accept_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (trans);

  GST_DEBUG_OBJECT (absolutetimestamps, "accept_caps");

  GstPad *pad;

  if (direction == GST_PAD_SRC)
    pad = GST_BASE_TRANSFORM_SINK_PAD (trans);
  else
    pad = GST_BASE_TRANSFORM_SRC_PAD (trans);

  return gst_pad_peer_query_accept_caps (pad, caps);
}

/* states */
static gboolean
gst_absolutetimestamps_start (GstBaseTransform * trans)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (trans);

  GST_DEBUG_OBJECT (absolutetimestamps, "start");

  absolutetimestamps->file = g_fopen (absolutetimestamps->filename, "wb");

  if (absolutetimestamps->file == NULL) {
    GST_ELEMENT_ERROR (absolutetimestamps, RESOURCE, OPEN_WRITE,
        ("Could not open file \"%s\" for writing.", absolutetimestamps->filename),
        GST_ERROR_SYSTEM);
    return FALSE;
  }

  return TRUE;
}

// In GStreamer source you often see _("...") - the underscore is defined in <gst/gst-i18n-lib.h> and
// is a shortcut for dgettext (a function for retrieving a localized version of a given string).
// In copying over snippets from gstfilesink.c I've removed the underscore usage.

static gboolean
gst_absolutetimestamps_stop (GstBaseTransform * trans)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (trans);

  GST_DEBUG_OBJECT (absolutetimestamps, "stop");

  if (absolutetimestamps->file) {
    if (fclose (absolutetimestamps->file) != 0)
      GST_ELEMENT_ERROR (absolutetimestamps, RESOURCE, CLOSE,
          ("Error closing file \"%s\".", absolutetimestamps->filename), GST_ERROR_SYSTEM);

    absolutetimestamps->file = NULL;
  }

  return TRUE;
}

static GstFlowReturn
gst_absolutetimestamps_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstAbsolutetimestamps *absolutetimestamps = GST_ABSOLUTETIMESTAMPS (trans);

  GST_DEBUG_OBJECT (absolutetimestamps, "transform_ip");

  GstClockTime timestamp = GST_BUFFER_TIMESTAMP (buf);

  if (timestamp != GST_CLOCK_TIME_NONE) {
      GTimeVal real_time;

      g_get_current_time (&real_time);

      gchar *s = g_time_val_to_iso8601 (&real_time);

      g_fprintf (absolutetimestamps->file, "%" GST_TIME_FORMAT " %s\n", GST_TIME_ARGS (timestamp), s);

      g_free (s);
  }

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  return gst_element_register (plugin, "absolutetimestamps", GST_RANK_NONE,
      GST_TYPE_ABSOLUTETIMESTAMPS);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "gst-absolutetimestamps"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "GstAbsolutetimestamps"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/george-hawkins/gst-absolutetimestamps"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    absolutetimestamps,
    "Element for generating a mapping from relative timestamps to absolute timestamps",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
