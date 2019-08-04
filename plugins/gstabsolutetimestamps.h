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

#ifndef _GST_ABSOLUTETIMESTAMPS_H_
#define _GST_ABSOLUTETIMESTAMPS_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_ABSOLUTETIMESTAMPS   (gst_absolutetimestamps_get_type())
#define GST_ABSOLUTETIMESTAMPS(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ABSOLUTETIMESTAMPS,GstAbsolutetimestamps))
#define GST_ABSOLUTETIMESTAMPS_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ABSOLUTETIMESTAMPS,GstAbsolutetimestampsClass))
#define GST_IS_ABSOLUTETIMESTAMPS(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ABSOLUTETIMESTAMPS))
#define GST_IS_ABSOLUTETIMESTAMPS_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ABSOLUTETIMESTAMPS))

typedef struct _GstAbsolutetimestamps GstAbsolutetimestamps;
typedef struct _GstAbsolutetimestampsClass GstAbsolutetimestampsClass;

struct _GstAbsolutetimestamps
{
  GstBaseTransform base_absolutetimestamps;

  gchar *filename;
  FILE *file;
};

struct _GstAbsolutetimestampsClass
{
  GstBaseTransformClass base_absolutetimestamps_class;
};

GType gst_absolutetimestamps_get_type (void);

G_END_DECLS

#endif
