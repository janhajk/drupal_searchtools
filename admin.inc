<?php 

// $Id:

/**
 * Declares administrative settings for the module.
 */
function searchtools_admin_settings() {
  $form['gmap'] = array(
    '#type' => 'fieldset',
    '#title' => t('Google Maps'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Google Maps Einstellungen')
  );
  $form['gmap']['searchtools_filterlocation'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Möglichkeit geben, nach Location zu Filtern'),
    '#default_value' => variable_get('searchtools_filterlocation', false),
    '#description'   => t('Das Locations Modul muss dazu installiert sein'),
  );
  $form['gmap']['searchtools_center_title'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Google Maps Zentrum Title'),
    '#default_value' => variable_get('searchtools_center_title', 'Basel'),
    '#size'          => 20,
    '#description'   => t(''),
  );
  $form['gmap']['searchtools_center_latitude'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Google Maps Zentrum Latitude'),
    '#default_value' => variable_get('searchtools_center_latitude', '47.54443828985943'),
    '#size'          => 20,
    '#description'   => t(''),
  );
  $form['gmap']['searchtools_center_longitude'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Google Maps Zentrum Longitude'),
    '#default_value' => variable_get('searchtools_center_longitude', '7.596015930175781'),
    '#size'          => 20,
    '#description'   => t(''),
  );
  $form['gmap']['searchtools_project'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Projektgebiet'),
    '#default_value' => variable_get('searchtools_project', 0),
    '#cols'          => 60,
    '#rows'          => 5,
    '#description'   => t('Pro Zeile eine Koordinate (Lat/Lng), Kommagetrennt'),
  );
  $form['gmap']['searchtools_showmap'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Google Maps anzeigen'),
    '#default_value' => variable_get('searchtools_showmap', false),
    '#description'   => t('In Dossiersuche Google Maps per default einblenden'),
  );
  return system_settings_form($form);
}


/**
 * Declares administrative settings for the module.
 */
function searchtools_frontpage_admin_settings() {
  $form['frontpage_titel'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Titel der Startseite'),
    '#default_value' => variable_get('frontpage_titel', 'Neustes'),
    '#description'   => t('Wenn leer, dann wird kein Titel angezeigt'),
  );
  $form['frontpage_titel_hide'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Titel verbergen'),
    '#default_value' => variable_get('frontpage_titel_hide', true),
    '#description'   => t(''),
  );
  $form['frontpage_usecache'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Use DB-Cache'),
    '#default_value' => variable_get('frontpage_usecache', true),
    '#description'   => t('If checked, the frontpage is cached'),
  );
  $form['frontpage_showOgLogos'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Show Organic Groups Logos'),
    '#default_value' => variable_get('frontpage_showOgLogos', true),
    '#description'   => t('If checked, Logos of Organic Groups are Displayed. Else only Name in Textform is showed'),
  );
  $form['frontpage_searchpath'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Pfad für Suche'),
    '#default_value' => variable_get('frontpage_searchpath', 'dossier/filter'),
    '#description'   => t('Pfad auf den Suche weitergeleitet werden soll ohne vorangehende und abschliessendes "/"'),
  );
  $form['frontpage_sql'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Abfrage der Daten SQL'),
    '#default_value' => variable_get('frontpage_sql', ''),
    '#description'   => t(''),
  );
  $form['frontpage_dossiertype'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Node Typ des Dossiers'),
    '#default_value' => variable_get('frontpage_dossiertype', 'dossier'),
    '#description'   => t(''),
  );
  return system_settings_form($form);
}


function frontpage_clearCache() {
  db_query("DELETE FROM {cache} WHERE cid LIKE 'frontpage_%'");
  drupal_set_message(t('Frontpage-Cache cleared!'));
  drupal_goto('<front>');
}
