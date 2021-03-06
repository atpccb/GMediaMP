/*
 *      entry.c
 *
 *      Copyright 2009 Brett Mravec <brett.mravec@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "entry.h"

G_DEFINE_TYPE(Entry, entry, G_TYPE_OBJECT)

struct _EntryPrivate {
    GHashTable *table;

    guint id;
    gchar *location;

    EntryType type;
    EntryState state;
};

static guint signal_state;

static void
entry_finalize (GObject *object)
{
    Entry *self = ENTRY (object);

    g_hash_table_unref (self->priv->table);

    G_OBJECT_CLASS (entry_parent_class)->finalize (object);
}

static void
entry_class_init (EntryClass *klass)
{
    GObjectClass *object_class;
    object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private ((gpointer) klass, sizeof (EntryPrivate));

    object_class->finalize = entry_finalize;

    signal_state = g_signal_new ("state-changed", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__UINT,
        G_TYPE_NONE, 0, G_TYPE_UINT);
}

static void
entry_init (Entry *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE((self), ENTRY_TYPE, EntryPrivate);

    self->priv->table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    self->priv->location = "";
    self->priv->state = ENTRY_STATE_NONE;
}

Entry*
_entry_new (guint id)
{
    Entry *self = g_object_new (ENTRY_TYPE, NULL);

    self->priv->id = id;

    return self;
}

const gchar*
entry_get_location (Entry *self)
{
    return entry_get_tag_str (self, "location");
}

gchar*
entry_get_art (Entry *self)
{
    GFile *location = g_file_new_for_path (entry_get_location (self));
    GFile *parent = g_file_get_parent (location);
    gchar *ppath = g_file_get_path (parent);
    GDir *dir = g_dir_open (ppath, 0, NULL);
    const gchar *file;
    gchar *ret = NULL;

    while (dir && (file = g_dir_read_name (dir))) {
        if (g_str_has_suffix (file, ".jpg")) {
            ret = g_strdup_printf ("%s/%s", ppath, file);
            break;
        }

        if (g_str_has_suffix (file, ".bmp")) {
            ret = g_strdup_printf ("%s/%s", ppath, file);
            break;
        }

        if (g_str_has_suffix (file, ".png")) {
            ret = g_strdup_printf ("%s/%s", ppath, file);
            break;
        }
    }

    if (dir) {
        g_dir_close (dir);
    }

    g_object_unref (G_OBJECT (location));
    g_object_unref (G_OBJECT (parent));
    g_free (ppath);

    if (ret) {
        return ret;
    } else {
        return g_strdup (SHARE_DIR "/imgs/rhythmbox-missing-artwork.svg");
    }
}

void
_entry_set_location (Entry *self, const gchar *location)
{
    _entry_set_tag_str (self, "location", location);
}

guint
entry_get_id (Entry *self)
{
    return self->priv->id;
}

void
_entry_set_id (Entry *self, guint id)
{
    self->priv->id = id;
}

EntryState
entry_get_state (Entry *self)
{
    return self->priv->state;
}

void
entry_set_state (Entry *self, EntryState state)
{
    self->priv->state = state;

    g_signal_emit (G_OBJECT (self), signal_state, 0);
}

gchar*
entry_get_state_string (Entry *self)
{
    switch (self->priv->state) {
        case ENTRY_STATE_PLAYING:
            return GTK_STOCK_MEDIA_PLAY;
        case ENTRY_STATE_PAUSED:
            return GTK_STOCK_MEDIA_PAUSE;
        case ENTRY_STATE_MISSING:
            return GTK_STOCK_DIALOG_ERROR;
        default:
            return NULL;
    }
}

void
_entry_set_tag_str (Entry *self, const gchar *tag, const gchar *value)
{
    g_hash_table_insert (self->priv->table, g_strdup (tag), value ? g_strdup (value) : g_strdup (""));
}

void
_entry_set_tag_int (Entry *self, const gchar *tag, gint value)
{
    gchar *val = g_strdup_printf ("%d", value);

    g_hash_table_insert (self->priv->table, g_strdup (tag), val);
}

const gchar*
entry_get_tag_str (Entry *self, const gchar *tag)
{
    return g_hash_table_lookup (self->priv->table, tag);
}

gint
entry_get_tag_int (Entry *self, const gchar *tag)
{
    gchar *val = (gchar*) g_hash_table_lookup (self->priv->table, tag);
    return val ? atoi (val) : 0;
}

EntryType
entry_get_media_type (Entry *self)
{
    return self->priv->type;
}

void
_entry_set_media_type (Entry *self, EntryType type)
{
    self->priv->type = type;
}

guint
entry_get_key_value_pairs (Entry *self, gchar ***keys, gchar ***vals)
{
    guint size = g_hash_table_size (self->priv->table);
    gchar **tkeys = g_new0 (gchar*, size+1);
    gchar **tvals = g_new0 (gchar*, size+1);
    gint i;

    GList *lkeys = g_hash_table_get_keys (self->priv->table);
    for (i = 0; i < size; i++)
        tkeys[i] = g_strdup ((gchar*) g_list_nth_data (lkeys, i));
    g_list_free (lkeys);

    GList *lvals = g_hash_table_get_values (self->priv->table);
    for (i = 0; i < size; i++)
        tvals[i] = g_strdup ((gchar*) g_list_nth_data (lvals, i));
    g_list_free (lvals);

    *keys = tkeys;
    *vals = tvals;

    return size;
}

gchar**
entry_get_kvs (Entry *self)
{
    GList *keys, *vals, *ki, *vi;
    guint i, size;
    gchar **kvs;

    size = g_hash_table_size (self->priv->table);

    kvs = g_new0 (gchar*, 2 * size + 1);

    keys = g_hash_table_get_keys (self->priv->table);
    vals = g_hash_table_get_values (self->priv->table);
    for (ki = keys, vi = vals, i = 0; ki && vi; ki = ki->next, vi = vi->next) {
        kvs[i++] = g_strdup ((gchar*) ki->data);
        kvs[i++] = g_strdup ((gchar*) vi->data);
    }

    g_list_free (keys);
    g_list_free (vals);

    return kvs;
}
