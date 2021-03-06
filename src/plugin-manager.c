/*
 *      plugin-manager.c
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

#include "plugin-manager.h"

G_DEFINE_TYPE(PluginManager, plugin_manager, G_TYPE_OBJECT)

struct _PluginManagerPrivate {
    guint state;
};

guint signal_eos;

static void
plugin_manager_finalize (GObject *object)
{
    PluginManager *self = PLUGIN_MANAGER (object);

    G_OBJECT_CLASS (plugin_manager_parent_class)->finalize (object);
}

static void
plugin_manager_class_init (PluginManagerClass *klass)
{
    GObjectClass *object_class;
    object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private ((gpointer) klass, sizeof (PluginManagerPrivate));

    object_class->finalize = plugin_manager_finalize;

    signal_eos = g_signal_new ("eos", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0);
}

static void
plugin_manager_init (PluginManager *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE((self), PLUGIN_MANAGER_TYPE, PluginManagerPrivate);

}

PluginManager*
plugin_manager_new ()
{
    return g_object_new (PLUGIN_MANAGER_TYPE, NULL);
}
