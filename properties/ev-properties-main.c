/*
 * Copyright (C) 2000, 2001 Eazel Inc.
 * Copyright (C) 2003  Andrew Sobala <aes@gnome.org>
 * Copyright (C) 2005  Bastien Nocera <hadess@hadess.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * The Ev project hereby grant permission for non-gpl compatible GStreamer
 * plugins to be used and distributed together with GStreamer and Ev. This
 * permission are above and beyond the permissions granted by the GPL license
 * Ev is covered by.
 *
 * Monday 7th February 2005: Christian Schaller: Add excemption clause.
 * See license_change file for details.
 *
 */

#include "ev-properties-view.h"
#include "ev-document-factory.h"

#include <config.h>
#include <string.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtklabel.h>
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>

static GType epp_type = 0;
static void property_page_provider_iface_init
	(NautilusPropertyPageProviderIface *iface);
static GList *ev_properties_get_pages
	(NautilusPropertyPageProvider *provider, GList *files);

static void
ev_properties_plugin_register_type (GTypeModule *module)
{
	const GTypeInfo info = {
		sizeof (GObjectClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) NULL,
		NULL,
		NULL,
		sizeof (GObject),
		0,
		(GInstanceInitFunc) NULL
	};
	const GInterfaceInfo property_page_provider_iface_info = {
		(GInterfaceInitFunc)property_page_provider_iface_init,
		NULL,
		NULL
	};

	epp_type = g_type_module_register_type (module, G_TYPE_OBJECT,
			"EvPropertiesPlugin",
			&info, 0);
	g_type_module_add_interface (module,
			epp_type,
			NAUTILUS_TYPE_PROPERTY_PAGE_PROVIDER,
			&property_page_provider_iface_info);
}

static void
property_page_provider_iface_init (NautilusPropertyPageProviderIface *iface)
{
	iface->get_pages = ev_properties_get_pages;
}

static GList *
ev_properties_get_pages (NautilusPropertyPageProvider *provider,
			 GList *files)
{
	GError *error = NULL;
	EvDocument *document;
	GList *pages = NULL;
	NautilusFileInfo *file;
	char *uri = NULL;
	GtkWidget *page, *label;
	NautilusPropertyPage *property_page;
	
	/* only add properties page if a single file is selected */
	if (files == NULL || files->next != NULL)
		goto end;
	file = files->data;

	/* okay, make the page */
	uri = nautilus_file_info_get_uri (file);
	document = ev_document_factory_get_document (uri, &error);

	if (error) {
		g_error_free (error);
		goto end;
	}
	
	if (!document)
		goto end;

	label = gtk_label_new (_("Document"));
	page = ev_properties_view_new ();
	ev_properties_view_set_info (EV_PROPERTIES_VIEW (page),
				     ev_document_get_info (document));
	gtk_widget_show (page);
	property_page = nautilus_property_page_new ("document-properties",
			label, page);
	g_object_unref (document);

	pages = g_list_prepend (pages, property_page);

end:
	g_free (uri);
	return pages;
}

/* --- extension interface --- */
void
nautilus_module_initialize (GTypeModule *module)
{
	ev_properties_plugin_register_type (module);
	ev_properties_view_register_type (module);

	/* set up translation catalog */
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

void
nautilus_module_shutdown (void)
{
}

void
nautilus_module_list_types (const GType **types,
                            int          *num_types)
{
	static GType type_list[1];

	type_list[0] = epp_type;
	*types = type_list;
	*num_types = G_N_ELEMENTS (type_list);
}

