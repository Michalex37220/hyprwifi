#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GtkWidget *wifi_interface_combo;
GtkWidget *wifi_network_list;
GtkWidget *bt_device_list;
GtkWidget *status_label;

char *exec_command(const char *cmd) {
    FILE *fp;
    char buffer[1024];
    char *result = malloc(8192);
    result[0] = '\0';

    fp = popen(cmd, "r");
    if (!fp) return NULL;

    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(result, buffer);
    }

    pclose(fp);
    return result;
}

void refresh_wifi_interfaces() {
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(wifi_interface_combo));
    char *output = exec_command("nmcli device status | grep wifi | awk '{print $1}'");
    char *line = strtok(output, "\n");

    while (line) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(wifi_interface_combo), line);
        line = strtok(NULL, "\n");
    }

    if (gtk_combo_box_get_active(GTK_COMBO_BOX(wifi_interface_combo)) < 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(wifi_interface_combo), 0);
    }
    free(output);
}

void scan_wifi_networks(GtkWidget *widget, gpointer data) {
    gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(wifi_network_list))));
    const char *iface = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wifi_interface_combo));
    if (!iface) return;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "nmcli -f SSID,BSSID,SIGNAL,SECURITY device wifi list ifname %s", iface);
    char *output = exec_command(cmd);

    char *line = strtok(output, "\n");
    int skip = 1;
    while (line) {
        if (skip) { skip = 0; line = strtok(NULL, "\n"); continue; }

        char ssid[256] = "", bssid[256] = "", signal[16] = "", security[64] = "";

        sscanf(line, "%255s %255s %15s %63[^\n]", ssid, bssid, signal, security);

        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(wifi_network_list)));
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, ssid, 1, bssid, 2, signal, 3, security, -1);

        line = strtok(NULL, "\n");
    }

    free(output);
}

void connect_to_wifi(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *ssid;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &ssid, -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Mot de passe", NULL, GTK_DIALOG_MODAL,
                                                      "_Connecter", GTK_RESPONSE_OK, 
                                                      "_Annuler", GTK_RESPONSE_CANCEL, NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
        gtk_container_add(GTK_CONTAINER(content_area), entry);
        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            const char *password = gtk_entry_get_text(GTK_ENTRY(entry));
            const char *iface = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wifi_interface_combo));

            char cmd[512];
            snprintf(cmd, sizeof(cmd), "nmcli dev wifi connect '%s' password '%s' ifname '%s'", ssid, password, iface);
            char *output = exec_command(cmd);
            gtk_label_set_text(GTK_LABEL(status_label), output);
            free(output);
        }

        gtk_widget_destroy(dialog);
        g_free(ssid);
    }
}

void disconnect_wifi(GtkWidget *widget, gpointer data) {
    char *output = exec_command("nmcli connection show --active | grep wifi | awk '{print $1}'");

    if (output && strlen(output) > 0) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "nmcli connection down '%s'", strtok(output, "\n"));
        char *res = exec_command(cmd);
        gtk_label_set_text(GTK_LABEL(status_label), res);
        free(res);
    }

    free(output);
}

void scan_bt_devices(GtkWidget *widget, gpointer data) {
    gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(bt_device_list))));
    
    char *output = exec_command("bluetoothctl devices");
    char *line = strtok(output, "\n");

    while (line) {
        char mac[32] = "", name[256] = "";
        sscanf(line, "Device %31s %255[^\n]", mac, name);

        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(bt_device_list)));
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, mac, 1, name, -1);

        line = strtok(NULL, "\n");
    }

    free(output);
}

void connect_bt_device(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer data) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gchar *mac;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 0, &mac, -1);

        char cmd[256];
        snprintf(cmd, sizeof(cmd), "bluetoothctl connect %s", mac);
        char *output = exec_command(cmd);
        gtk_label_set_text(GTK_LABEL(status_label), output);
        free(output);
        g_free(mac);
    }
}

void disconnect_bt_device(GtkWidget *widget, gpointer data) {
    char *output = exec_command("bluetoothctl info | grep 'Device' | awk '{print $2}'");

    if (output && strlen(output) > 0) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "bluetoothctl disconnect %s", strtok(output, "\n"));
        char *res = exec_command(cmd);
        gtk_label_set_text(GTK_LABEL(status_label), res);
        free(res);
    }

    free(output);
}

void toggle_bt_power(GtkWidget *widget, gpointer data) {
    char *output = exec_command("bluetoothctl show | grep 'Powered: yes'");
    if (output) {
        if (strlen(output) > 0) {
            // BT is on, turn it off
            system("bluetoothctl power off");
            gtk_label_set_text(GTK_LABEL(status_label), "Bluetooth désactivé");
        } else {
            // BT is off, turn it on
            system("bluetoothctl power on");
            gtk_label_set_text(GTK_LABEL(status_label), "Bluetooth activé");
        }
        free(output);
    }
}

GtkWidget* create_wifi_tab() {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    wifi_interface_combo = gtk_combo_box_text_new();
    gtk_widget_set_halign(wifi_interface_combo, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Interface Wi-Fi :"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), wifi_interface_combo, 1, 0, 2, 1);

    GtkWidget *scan_button = gtk_button_new_with_label("Scanner les réseaux");
    gtk_widget_set_halign(scan_button, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), scan_button, 0, 1, 3, 1);

    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    wifi_network_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(wifi_network_list),
        gtk_tree_view_column_new_with_attributes("SSID", renderer, "text", 0, NULL));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(wifi_network_list),
        gtk_tree_view_column_new_with_attributes("BSSID", renderer, "text", 1, NULL));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(wifi_network_list),
        gtk_tree_view_column_new_with_attributes("Signal (%)", renderer, "text", 2, NULL));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(wifi_network_list),
        gtk_tree_view_column_new_with_attributes("Sécurité", renderer, "text", 3, NULL));

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), wifi_network_list);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0, 2, 3, 1);

    g_signal_connect(scan_button, "clicked", G_CALLBACK(scan_wifi_networks), NULL);
    g_signal_connect(wifi_network_list, "row-activated", G_CALLBACK(connect_to_wifi), NULL);

    GtkWidget *disconnect_button = gtk_button_new_with_label("Déconnecter Wi-Fi");
    gtk_widget_set_halign(disconnect_button, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), disconnect_button, 0, 3, 3, 1);
    g_signal_connect(disconnect_button, "clicked", G_CALLBACK(disconnect_wifi), NULL);

    return grid;
}

GtkWidget* create_bluetooth_tab() {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    GtkWidget *power_button = gtk_button_new_with_label("Activer/Désactiver Bluetooth");
    gtk_widget_set_halign(power_button, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), power_button, 0, 0, 3, 1);
    g_signal_connect(power_button, "clicked", G_CALLBACK(toggle_bt_power), NULL);

    GtkWidget *scan_button = gtk_button_new_with_label("Scanner les appareils");
    gtk_widget_set_halign(scan_button, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), scan_button, 0, 1, 3, 1);

    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    bt_device_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(bt_device_list),
        gtk_tree_view_column_new_with_attributes("Adresse MAC", renderer, "text", 0, NULL));

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(bt_device_list),
        gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL));

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), bt_device_list);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0, 2, 3, 1);

    g_signal_connect(scan_button, "clicked", G_CALLBACK(scan_bt_devices), NULL);
    g_signal_connect(bt_device_list, "row-activated", G_CALLBACK(connect_bt_device), NULL);

    GtkWidget *disconnect_button = gtk_button_new_with_label("Déconnecter l'appareil");
    gtk_widget_set_halign(disconnect_button, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), disconnect_button, 0, 3, 3, 1);
    g_signal_connect(disconnect_button, "clicked", G_CALLBACK(disconnect_bt_device), NULL);

    return grid;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gestionnaire de Connexions");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 0);

    // Onglet Wi-Fi
    GtkWidget *wifi_tab = create_wifi_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), wifi_tab, gtk_label_new("Wi-Fi"));

    // Onglet Bluetooth
    GtkWidget *bt_tab = create_bluetooth_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bt_tab, gtk_label_new("Bluetooth"));

    // Barre de statut
    status_label = gtk_label_new("Statut : En attente");
    gtk_widget_set_halign(status_label, GTK_ALIGN_CENTER);
    gtk_box_pack_end(GTK_BOX(main_box), status_label, FALSE, FALSE, 5);

    refresh_wifi_interfaces();
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}