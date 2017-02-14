/*
 * prefs.c
 *
 * Copyright 2013-2015 AmatCoder
 *
 * This file is part of Mednaffe.
 *
 * Mednaffe is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mednaffe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mednaffe; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"

void save_combo(GKeyFile *key_file, guidata *gui)
{
  GtkTreeModel *combostore;
  GtkTreeIter iter;
  gint a_item, n_items;
  gchar **array;
  gboolean valid;
  gint i=0;

  combostore = gtk_combo_box_get_model(GTK_COMBO_BOX(gui->cbpath));
  a_item = gtk_combo_box_get_active(GTK_COMBO_BOX(gui->cbpath));
  n_items = gtk_tree_model_iter_n_children(combostore, NULL);

  /*const gchar* array[n_items];*/
  array = g_new(gchar *, n_items+1);
  array[n_items]=NULL;

  valid = gtk_tree_model_get_iter_first (combostore, &iter);
  while (valid)
  {
    gtk_tree_model_get (combostore, &iter, 0, &array[i], -1);
    i++;
    valid = gtk_tree_model_iter_next (combostore, &iter);
  }

  g_key_file_set_string_list(key_file, "GUI", "Folders",
                                        (const gchar **)array, n_items);

  g_key_file_set_integer(key_file, "GUI", "Last Folder", a_item);

  g_strfreev(array);
}

void save_systems_showed(GKeyFile *key_file, guidata *gui)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean blist[15];
  gboolean valid;
  gint i=0;

  model = GTK_TREE_MODEL(gtk_builder_get_object(gui->builder, "liststore3"));
  valid = gtk_tree_model_get_iter_first (model, &iter);
  while (valid)
  {
    gtk_tree_model_get(model, &iter, 3, &blist[i], -1);
    i++;
    valid = gtk_tree_model_iter_next (model, &iter);
  }
  g_key_file_set_boolean_list(key_file, "GUI", "Show Systems", blist, 15);
}

void save_emu_options(GKeyFile *key_file, guidata *gui)
{
  GList *list = NULL;
  GList *iterator = NULL;

  list = g_hash_table_get_keys(gui->clist);

  for (iterator = list; iterator; iterator = iterator->next)
  {
    gchar *value;

    iterator->data = ((gchar *)iterator->data)+1;
    value = g_strdup(g_hash_table_lookup(gui->hash, iterator->data));
    iterator->data = ((gchar *)iterator->data)-1;
    g_key_file_set_string(key_file, "Emulator", iterator->data, value);
    g_free(value);
  }
  g_list_free(list);
}

void save_prefs(guidata *gui)
{
  gchar *conf_file;
  gchar *conf;
  FILE *file;
  GKeyFile *key_file;
  GtkWidget *option;

  #ifdef G_OS_WIN32
    conf_file=g_strconcat(g_win32_get_package_installation_directory_of_module(NULL), "\\mednaffe.ini", NULL);
  #else
    conf_file=g_strconcat(g_get_user_config_dir(),"/mednaffe.conf", NULL);
  #endif

  key_file=g_key_file_new();
  /*g_key_file_set_list_separator(key_file,  0x0D);*/

  g_key_file_set_comment(key_file, NULL, NULL, " Version 0.8.5\n \
Do not edit this file!", NULL);

  g_key_file_set_string(key_file, "GUI", "Bin", gui->binpath);

  save_combo(key_file, gui);

  option = GTK_WIDGET(gtk_builder_get_object(gui->settings,"showtooltips"));
  g_key_file_set_boolean(key_file, "GUI", "Tooltips",
                         gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(option)));

  option = GTK_WIDGET(gtk_builder_get_object(gui->settings,"remembersize"));
  g_key_file_set_boolean(key_file, "GUI", "RememberSize",
                         gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(option)));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(option)))
  {
  gint width;
  gint height;

  gtk_window_get_size(GTK_WINDOW(gui->topwindow), &width, &height);
  if (width && height)
  {
      g_key_file_set_integer(key_file, "GUI", "Width", width);
      g_key_file_set_integer(key_file, "GUI", "Height", height);
    }
  }

  g_key_file_set_integer(key_file, "GUI", "Filter", gui->filter);
  g_key_file_set_integer(key_file, "GUI", "View Mode", gui->listmode);
  g_key_file_set_integer(key_file, "GUI", "ActionLaunch", gui->state);

  if (gtk_tree_view_column_get_sort_order(gui->column) == GTK_SORT_DESCENDING)
    g_key_file_set_boolean(key_file, "GUI", "Reverse Sort", TRUE);

  save_systems_showed(key_file, gui);
  save_emu_options(key_file, gui);

  conf = g_key_file_to_data(key_file, NULL, NULL);

  file=fopen(conf_file, "w");
  fputs(conf, file);
  fclose(file);

  g_key_file_free(key_file);
  g_free(conf);
  g_free(conf_file);
}

void load_combo(GKeyFile *key_file, guidata *gui)
{
  GtkTreeModel *combostore;
  GtkTreeIter iter;
  gsize n_items = 0;
  gint a_item;
  gchar **folders = NULL;

  combostore = gtk_combo_box_get_model(GTK_COMBO_BOX(gui->cbpath));

  folders = g_key_file_get_string_list(key_file, "GUI", "Folders",
                                                        &n_items, NULL);

  a_item = g_key_file_get_integer(key_file, "GUI", "Last Folder", NULL);

  if (folders!=NULL)
  {
    while (n_items>0)
    {
      n_items--;
      gtk_list_store_prepend(GTK_LIST_STORE(combostore), &iter);
      gtk_list_store_set(GTK_LIST_STORE(combostore), &iter, 0, folders[n_items], -1);
    }
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(gui->cbpath), a_item);

  g_strfreev(folders);
}

void load_systems_showed(GKeyFile *key_file, guidata *gui)
{
  GtkTreeIter iter, iter2;
  gboolean *showed;
  gsize length = 0;
  gint n_items = 0;

  showed = g_key_file_get_boolean_list(key_file, "GUI", "Show Systems",
                                                        &length, NULL);
  if (showed)
  {
    GtkListStore *store;
    GtkTreeModel *model;

    store = GTK_LIST_STORE(gtk_builder_get_object(gui->builder, "liststore3"));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(gtk_builder_get_object(
                                          gui->settings, "se_treeview")));

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    gtk_tree_model_get_iter_first(model, &iter2);
    while (n_items<15)
    {
      gtk_list_store_set(store, &iter, 3, showed[n_items], -1);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter2, 1, showed[n_items], -1);
      gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
      gtk_tree_model_iter_next(model, &iter2);
      n_items++;
    }
  g_free(showed);
  }
}

void load_emu_options(GKeyFile *key_file, guidata *gui)
{
  gchar **ffekeys;
  gsize length = 0;

  ffekeys = g_key_file_get_keys(key_file, "Emulator", &length, NULL);

  if (length>0)
  {
    gint i=0;

    while (ffekeys[i])
    {
      gchar *ffecopy;

      ffecopy = g_strdup((ffekeys[i])+1);
      g_hash_table_insert(gui->hash, ffecopy,
                          g_key_file_get_string(key_file, "Emulator",
                          ffekeys[i], NULL));

      g_hash_table_replace(gui->clist, ffekeys[i], ffekeys[i]);
      i++;
    }
  }
  g_free(ffekeys);
  g_key_file_free(key_file);
}

GKeyFile* load_prefs(guidata *gui)
{
  gchar *conf_file;
  GKeyFile *key_file;
  gboolean value;
  gint state;
  GError *err = NULL;

  #ifdef G_OS_WIN32
    conf_file=g_strconcat(g_win32_get_package_installation_directory_of_module(NULL), "\\mednaffe.ini", NULL);
  #else
    conf_file=g_strconcat(g_get_user_config_dir(), "/mednaffe.conf", NULL);
  #endif

  key_file = g_key_file_new();
  /*g_key_file_set_list_separator(key_file,  0x0D);*/

  if (g_key_file_load_from_file(key_file, conf_file,
                                G_KEY_FILE_NONE, NULL))
  {
    gui->binpath = g_key_file_get_string(key_file, "GUI", "Bin", NULL);

    load_combo(key_file, gui);

    value = g_key_file_get_boolean(key_file, "GUI", "Reverse Sort", &err);

     if (err==NULL)
     {
      if (value)
        gtk_tree_view_column_clicked(gui->column);
     }
    else
    {
      g_error_free (err);
      err=NULL;
    }

    GtkWidget *option;

    option = GTK_WIDGET(gtk_builder_get_object(gui->settings,
                                               "showtooltips"));
    value = g_key_file_get_boolean(key_file, "GUI", "Tooltips", &err);
    if (err==NULL)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option),value);
    else
    {
      g_error_free (err);
      err=NULL;
    }

    option = GTK_WIDGET(gtk_builder_get_object(gui->settings,
                                               "remembersize"));
    value = g_key_file_get_boolean(key_file, "GUI", "RememberSize", &err);
    if (err==NULL)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option),value);
    else
    {
      g_error_free (err);
      err=NULL;
    }
    if (value)
    {
    gint width;
    gint height;

    width = g_key_file_get_integer(key_file, "GUI", "Width", NULL);
    height = g_key_file_get_integer(key_file, "GUI", "Height", NULL);

    if (width && height)
      gtk_window_resize(GTK_WINDOW(gui->topwindow), width, height);
  }

    state=g_key_file_get_integer(key_file, "GUI", "Filter", NULL);

    switch (state)
    {
      case 1:
        option = GTK_WIDGET(gtk_builder_get_object(gui->builder, "radiomenuzip"));
        gtk_menu_item_activate (GTK_MENU_ITEM(option));
      break;

      case 2:
        option = GTK_WIDGET(gtk_builder_get_object(gui->builder, "radiomenucue"));
        gtk_menu_item_activate (GTK_MENU_ITEM(option));
      break;

      default:
      break;
    }

    state=g_key_file_get_integer(key_file, "GUI", "ActionLaunch", NULL);

    switch (state)
    {
      case 2:
        option = GTK_WIDGET(gtk_builder_get_object(gui->settings, "rbhide"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option), TRUE);
      break;

      case 1:
        option = GTK_WIDGET(gtk_builder_get_object(gui->settings, "rbminimize"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(option), TRUE);
      break;

      default:
      break;
    }

    state=g_key_file_get_integer(key_file, "GUI", "View Mode", NULL);

    switch (state)
    {
      case 1:
        option = GTK_WIDGET(gtk_builder_get_object(gui->builder, "recursivemenu"));
        gtk_menu_item_activate (GTK_MENU_ITEM(option));
      break;

      default:
      break;
    }

    load_systems_showed(key_file, gui);
  }
  else
  {
    g_key_file_free(key_file);
    key_file = NULL;
  }
  g_free(conf_file);

  return key_file;
}
