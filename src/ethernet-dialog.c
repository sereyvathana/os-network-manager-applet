// SPDX-License-Identifier: GPL-2.0+
/* NetworkManager Applet -- allow user control over networking
 *
 * Dan Williams <dcbw@redhat.com>
 *
 * Copyright 2008 Novell, Inc.
 * Copyright 2008 - 2014 Red Hat, Inc.
 */

#include "nm-default.h"


#include "ethernet-dialog.h"
#include "wireless-security.h"
#include "applet-dialogs.h"
#include "eap-method.h"

static void
stuff_changed_cb (WirelessSecurity *sec, gpointer user_data)
{
	GtkWidget *button = GTK_WIDGET (user_data);

	gtk_widget_set_sensitive (button, wireless_security_validate (sec, NULL));
}

static void
dialog_set_network_name (NMConnection *connection, GtkEntry *entry)
{
	NMSettingConnection *setting;

	setting = nm_connection_get_setting_connection (connection);

	gtk_widget_set_sensitive (GTK_WIDGET (entry), FALSE);
	gtk_entry_set_text (entry, nm_setting_connection_get_id (setting));
}

static WirelessSecurity *
dialog_set_security (NMConnection *connection,
                     GtkBuilder *builder,
                     GtkBox *box)
{
	GList *children;
	GList *iter;
	WirelessSecurity *security;

	security = (WirelessSecurity *) ws_wpa_eap_new (connection, FALSE, TRUE, NULL);

	/* Remove any previous wireless security widgets */
	children = gtk_container_get_children (GTK_CONTAINER (box));
	for (iter = children; iter; iter = iter->next)
		gtk_container_remove (GTK_CONTAINER (box), GTK_WIDGET (iter->data));
	g_list_free (children);

	gtk_box_pack_start (box, wireless_security_get_widget (security), TRUE, TRUE, 0);

	return security;
}

GtkWidget *
nma_ethernet_dialog_new (NMConnection *connection)
{
	GtkBuilder *builder;
	GtkWidget *dialog;
	GError *error = NULL;
	WirelessSecurity *security;

	builder = gtk_builder_new ();

	if (!gtk_builder_add_from_resource (builder, "/org/freedesktop/network-manager-applet/8021x.ui", &error)) {
		g_warning ("Couldn't load builder resource: %s", error->message);
		g_error_free (error);
		applet_missing_ui_warning_dialog_show ();
		g_object_unref (builder);
		return NULL;
	}

	dialog = (GtkWidget *) gtk_builder_get_object (builder, "8021x_dialog");
	if (!dialog) {
		g_warning ("Couldn't find wireless_dialog widget.");
		applet_missing_ui_warning_dialog_show ();
		g_object_unref (builder);
		return NULL;
	}

	gtk_window_set_title (GTK_WINDOW (dialog), _("802.1X authentication"));
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "dialog-password");
	dialog_set_network_name (connection, GTK_ENTRY (gtk_builder_get_object (builder, "network_name_entry")));

	/* Handle CA cert ignore stuff */
	eap_method_ca_cert_ignore_load (connection);

	security = dialog_set_security (connection, builder, GTK_BOX (gtk_builder_get_object (builder, "security_vbox")));
	wireless_security_set_changed_notify (security, stuff_changed_cb, GTK_WIDGET (gtk_builder_get_object (builder, "ok_button")));
	g_object_set_data_full (G_OBJECT (dialog),
	                        "security", security,
	                        (GDestroyNotify) wireless_security_unref);

	g_object_set_data_full (G_OBJECT (dialog),
	                        "connection", g_object_ref (connection),
	                        (GDestroyNotify) g_object_unref);

	/* Ensure the builder gets destroyed when the dialog goes away */
	g_object_set_data_full (G_OBJECT (dialog),
	                        "builder", builder,
	                        (GDestroyNotify) g_object_unref);

	return dialog;
}
					  
NMConnection *
nma_ethernet_dialog_get_connection (GtkWidget *dialog)
{
	NMConnection *connection, *tmp_connection;
	WirelessSecurity *security;
	NMSetting *s_8021x, *s_con;

	g_return_val_if_fail (dialog != NULL, NULL);

	connection = g_object_get_data (G_OBJECT (dialog), "connection");
	security = g_object_get_data (G_OBJECT (dialog), "security");

	/* Here's a nice hack to work around the fact that ws_802_1x_fill_connection()
	 * needs a wireless setting and a connection setting for various things.
	 */
	tmp_connection = nm_simple_connection_new ();

	/* Add the fake connection setting (mainly for the UUID for cert ignore checking) */
	s_con = nm_connection_get_setting (connection, NM_TYPE_SETTING_CONNECTION);
	g_assert (s_con);
	nm_connection_add_setting (tmp_connection, NM_SETTING (g_object_ref (s_con)));

	/* And the fake wireless setting */
	nm_connection_add_setting (tmp_connection, nm_setting_wireless_new ());

	/* Fill up the 802.1x setting */
	ws_802_1x_fill_connection (security, "wpa_eap_auth_combo", tmp_connection);

	/* Grab it and add it to our original connection */
	s_8021x = nm_connection_get_setting (tmp_connection, NM_TYPE_SETTING_802_1X);
	nm_connection_add_setting (connection, NM_SETTING (g_object_ref (s_8021x)));

	/* Save new CA cert ignore values to GSettings */
	eap_method_ca_cert_ignore_save (tmp_connection);

	/* Remove the 8021x setting to prevent the clearing of secrets when the
	 * simple-connection is destroyed.
	 */
	nm_connection_remove_setting (tmp_connection, NM_TYPE_SETTING_802_1X);
	g_object_unref (tmp_connection);

	return connection;
}
