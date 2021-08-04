#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <math.h>


/* Hooray for globals. Making small projects easier for newbies. */
/* If this starts to spiral out of control I will do a re-write. But this
   is basically my first time using C with GTK3. so this should speed up my
   initial time-to-working-demo. */

char *app_name="TabletPC_UnitConv"; /* for FVWM to be happy */
int display_height=0;
int display_width=0;
GtkLabel *resultLabel;
GtkEntry *inputEntry;
GtkComboBoxText *inLabel;
int lengthMult=0; /* mm, cm, m multiplication factor */

/* lots of callbacks and other misc. utility functions here */

/* allocates and populates a string for using as a label. */
/* this result will need to be freed sometime after each call. */
gchar *parseEntry(const gchar *inputStr)
{
/*  static const char DECIMAL_FORMAT_STRING[]="%d' %.1f\" (%.2f\")"; */
  static const char DECIMAL_FORMAT_STRING[]="%d' %.1f\"     ---     (%.2f\")";
  /* convert to meters */
  
  /* meters to feet */
  double resultInch=strtod(inputStr,NULL) * (1.0/lengthMult) * 39.3701;
/*  double resultFoot=(strtod(inputStr,NULL) * (1.0/lengthMult) * 39.3701 )/12.0;*/
  double resultFoot=resultInch / 12.0;
  int resultFootInt=(int)round(resultFoot);
  double resultInchOnly=fmod(resultInch,12.0);
  /* find how much we need to allocate */
  size_t neededChars=snprintf(NULL,0,DECIMAL_FORMAT_STRING,resultFootInt,resultInchOnly,resultInch)+1;
  gchar *output=g_malloc((gsize)neededChars); /* normal malloc() probably is the same here */
  sprintf(output, DECIMAL_FORMAT_STRING,resultFootInt,resultInchOnly,resultInch);
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
  /* parseEntry() malloc's a string. Clear it.*/
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

  const gchar* content = gtk_entry_get_text( GTK_ENTRY(edit) );
  gchar* output = parseEntry( content);
  gtk_label_set_text(resultLabel,output);
  /* parseEntry() malloc's a string. Clear it.*/
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
  /* parseEntry() malloc's a string. Clear it.*/
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
  
  /* inLabel is globally defined. Just assign it here. */
  inLabel=(GtkComboBoxText*)gtk_combo_box_text_new();
  gtk_combo_box_text_append(inLabel, 0, "mm");
  gtk_combo_box_text_append(inLabel, 0, "cm");
  gtk_combo_box_text_append(inLabel, 0, "m");
  /* text variant doesn't have its own 'set_active' func, so we do a cast */
  gtk_combo_box_set_active((GtkComboBox*)inLabel,1);
  /* defaults to cm so lengthMult must be 100 at launch */
  lengthMult=100;

  g_signal_connect(inLabel, "changed", G_CALLBACK(input_unit), NULL );

  gtk_container_add (GTK_CONTAINER (window), vbox);
  resultLabel=(GtkLabel *)gtk_label_new("0' 0.0\"     ---     0.00 in");

  inputEntry=(GtkEntry *)gtk_entry_new();
  gtk_entry_set_width_chars(inputEntry, 11);
  g_signal_connect_after(inputEntry, "insert-text", G_CALLBACK(input_insert_after), NULL );
  g_signal_connect(inputEntry, "insert-text", G_CALLBACK(input_insert), NULL );
  g_signal_connect_after(inputEntry, "delete-text", G_CALLBACK(input_insert_after), NULL );
  gtk_box_pack_start((GtkBox *)hboxInLine, (GtkWidget *)inputEntry, (gboolean)0, (gboolean)1, 0);
  gtk_box_pack_start((GtkBox *)hboxInLine, (GtkWidget *)inLabel, (gboolean)0, (gboolean)1, 4);
  gtk_box_pack_start((GtkBox *)hboxOutLine, (GtkWidget *)resultLabel, (gboolean)0, (gboolean)1, 4);
  gtk_box_pack_start((GtkBox *)vbox, (GtkWidget *)hboxInLine, (gboolean)0, (gboolean)1, 0);
  gtk_box_pack_start((GtkBox *)vbox, (GtkWidget *)hboxOutLine, (gboolean)0, (gboolean)1, 4);
  gtk_widget_show_all(window);          
}


int main (int argc, char **argv)
{
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
