#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>

#include <xcb/xcb.h>

char *btnsize="32"; /* change icon size here. Part of filename so a string*/
/* char *app_name="TabletPC_Applet_Menu"; */ /* so fvwm can apply its rules to it*/
char *app_name="TabletPC_UnitConv";
int display_height=0;
int display_width=0;
GtkLabel *resultLabel;
GtkEntry *inputEntry;
GtkComboBoxText *inLabel;
int lengthMult=0; /* mm, cm, m multiplication factor */

/* allocates and populates a string for using as a label. */
/* this result will need to be freed sometime after each call. */
gchar *parseEntry(const gchar *inputStr)
{
  static const char DECIMAL_FORMAT_STRING[]="%.1f ft (%.2f\")";
  /* convert to meters */
  
  /* meters to feet */
  double resultInch=strtod(inputStr,NULL) * (1.0/lengthMult) * 39.3701;
/*  double resultFoot=(strtod(inputStr,NULL) * (1.0/lengthMult) * 39.3701 )/12.0;*/
  double resultFoot=resultInch / 12.0;
  /* find how much we need to allocate */
  size_t neededChars=snprintf(NULL,0,DECIMAL_FORMAT_STRING,resultFoot,resultInch);
  gchar *output=g_malloc((gsize)neededChars); /* normal malloc() probably is the same here */
  sprintf(output, DECIMAL_FORMAT_STRING,resultFoot,resultInch);
  return output;
}

/* callbacks */
/* allow only integer input */
void input_insert (
  GtkEditable *editable, const gchar *text,
  gint length, gint *position, gpointer data)
{
  int i;
  int decimalUsed=0;
  for (i = 0; i < length; i++) {
    if (!isdigit(text[i])) {
      if(text[i] == '.' && decimalUsed == 0 ) /*allow a single decimal point*/
      {
        decimalUsed=1;
      }
      else
      {
        g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
        return;
      }
    }
  }
}

void
input_insert_after (GtkEditable* edit,
                    gchar* new_text,
                    gint new_length,
                    gpointer position,
                    gpointer data)
{
  /*prevent compiler warnings about unused variables*/
  (void) new_text; (void) new_length; (void) position; (void) data;
  const gchar* content = gtk_entry_get_text( GTK_ENTRY(edit) );
  gchar* output = parseEntry( content);
  gtk_label_set_text(resultLabel,output);
  g_free(output);
}

void
input_delete_after (GtkEditable* edit, gchar *text,
                    gint start_pos,
                    gint end_pos,
                    gint length,
                    gint *position,
                    gpointer data)
{
  fprintf(stderr,"%d\n",(int)length);
  /*no op cast to prevent compiler warnings*/
  (void) start_pos; (void) end_pos; (void) data;

  
  /*get text and modify the entry*/
/*    int cursor_pos = gtk_editable_get_position(edit);*/
  const gchar* content = gtk_entry_get_text( GTK_ENTRY(edit) );
  gchar* output = parseEntry( content);
  gtk_label_set_text(resultLabel,output);
/*    gtk_editable_set_position(edit, cursor_pos);*/
  g_free(output);
}

void input_unit(GtkComboBox *widget, gpointer data)
{
  (void)data;
  char *str=gtk_combo_box_text_get_active_text((GtkComboBoxText*)widget);
  if(g_strcmp0(str,"mm") == 0)
  {
    lengthMult=1000;
  }
  else if(g_strcmp0(str,"cm") == 0)
  {
    lengthMult=100;
  }
  else
  {
    lengthMult=1;
  }
  const gchar* content = gtk_entry_get_text(GTK_ENTRY((GtkEditable*)inputEntry));
  gchar* output = parseEntry( content);
  gtk_label_set_text(resultLabel,output);
/*    gtk_editable_set_position(edit, cursor_pos);*/
  g_free(output);
  
}

static void activate(GtkApplication* applet, gpointer user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new (applet);
  gtk_window_set_title (GTK_WINDOW (window), "Units");
  /* gtk_window_set_wmclass() is deprecated, replacing with g_set_prgname()
     near the bottom of the program. */
  /*  gtk_window_set_wmclass(GTK_WINDOW (window), app_name, app_name); */
  GtkWidget *vbox=gtk_box_new (GTK_ORIENTATION_VERTICAL, 0); /* 0 spacing */
  GtkWidget *hboxInLine=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *hboxOutLine=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
/*  GtkLabel *inLabel=(GtkLabel *)gtk_label_new("cm"); */

  /* inLabel is global */
  inLabel=(GtkComboBoxText*)gtk_combo_box_text_new();
  gtk_combo_box_text_append(inLabel, 0, "mm");
  gtk_combo_box_text_append(inLabel, 0, "cm");
  gtk_combo_box_text_append(inLabel, 0, "m");
  /* text variant doesn't have its own 'set_active' func, so we do a cast */
  gtk_combo_box_set_active((GtkComboBox*)inLabel,1);
  /* defaults to cm so lengthMult must be 100 */
  lengthMult=100;

  g_signal_connect(inLabel, "changed", G_CALLBACK(input_unit), NULL );

/*  GtkLabel *outLabelInch=(GtkLabel *)gtk_label_new("in");*/
/*  GtkLabel *outLabelFoot=(GtkLabel *)gtk_label_new("ft)");*/
  gtk_container_add (GTK_CONTAINER (window), vbox);
  resultLabel=(GtkLabel *)gtk_label_new("0.0 ft (0.00\")");

/*  GtkEntryBuffer *inputEntryBuffer=gtk_entry_buffer_new("", 0);*/
  inputEntry=(GtkEntry *)gtk_entry_new();
  gtk_entry_set_width_chars(inputEntry, 11);
  g_signal_connect_after(inputEntry, "insert-text", G_CALLBACK(input_insert_after), NULL );
  g_signal_connect(inputEntry, "insert-text", G_CALLBACK(input_insert), NULL );
  g_signal_connect_after(inputEntry, "delete-text", G_CALLBACK(input_insert_after), NULL );
/*  g_signal_connect(inputEntry, "delete-text", G_CALLBACK(input_delete), NULL );*/
  gtk_box_pack_start((GtkBox *)hboxInLine, (GtkWidget *)inputEntry, (gboolean)0, (gboolean)1, 0);
  gtk_box_pack_start((GtkBox *)hboxInLine, (GtkWidget *)inLabel, (gboolean)0, (gboolean)1, 4);
  gtk_box_pack_start((GtkBox *)hboxOutLine, (GtkWidget *)resultLabel, (gboolean)0, (gboolean)1, 4);
/*  gtk_box_pack_start((GtkBox *)hboxOutLine, (GtkWidget *)outLabelInch, (gboolean)0, (gboolean)1, 0);*/
//  gtk_container_add((GtkContainer *)vbox, (GtkWidget *)inputEntry);
  gtk_box_pack_start((GtkBox *)vbox, (GtkWidget *)hboxInLine, (gboolean)0, (gboolean)1, 0);
  gtk_box_pack_start((GtkBox *)vbox, (GtkWidget *)hboxOutLine, (gboolean)0, (gboolean)1, 4);
//  gtk_container_add((GtkContainer *)vbox, (GtkWidget *)resultLabel);
  gtk_widget_show_all(window);
              
}


int main (int argc, char **argv)
{
  /* we need to use XCB (or could have used Xlib) to get display geometry.*/
  xcb_screen_t *screen;
  xcb_screen_iterator_t iter;
  int screen_num; /* gets set by xcb_connect */

  xcb_connection_t *dispcon = xcb_connect (NULL, &screen_num);


/*  const xcb_setup_t *setup = xcb_get_setup (dispcon);*/
  /* in the python version I looked at root window dimensions. Here I am
     looking at actual screen dimensions. */
  /* Get the screen number */
  iter = xcb_setup_roots_iterator (xcb_get_setup (dispcon));
  for (; iter.rem; --screen_num, xcb_screen_next (&iter))
  {
    if (screen_num == 0)
    {
      screen = iter.data;
      break;
    }
  }
  xcb_flush (dispcon);

  /*  screen = xcb_setup_roots_iterator(setup).data; */
/*  printf("Width:\t%d\nHeight:\t%d\n",screen->width_in_pixels,screen->height_in_pixels);*/
  display_width=screen->width_in_pixels;
  display_height=screen->height_in_pixels;
  /* we're all done using XCB now. We got what we needed.*/
  xcb_disconnect (dispcon);

  /* GTK applet initialization stuff */
  GtkApplication *applet;
  int status;

  applet = gtk_application_new ("org.wyatt8740.tabletpc_unitconv", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (applet, "activate", G_CALLBACK (activate), NULL);
/* use this instead of `gtk_window_set_wmclass()`, which is deprecated: */
  g_set_prgname(app_name);
  status = g_application_run (G_APPLICATION (applet), argc, argv);
  g_object_unref (applet);

  return status;
}
