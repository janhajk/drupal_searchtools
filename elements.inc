<?php

/**
 * Gibt alle Filter HTML-Formatiert zurück
 * @return string HTML-Code der Filter
 */
function _dossiersearch_getFilters($query='') {
  $pre       = 'dossiersearch_filter_';
  $gmap      = '<div id="'.$pre.'location">'.
                 '<label for="'.$pre.'map_active">geographisch mit Karte filtern'.
                   '<input type="checkbox" id="'.$pre.'map_active" />'.
                 '</label>'.
                 '<div style="width:300px;height:150px;display:none;" id="dossiersearch_gmap"></div>'.
                 '<span id="'.$pre.'bounds" style="font-size:0.5em;display:none"></span>'.
               '</div>';
  $fTitle    = '<div class="dossiersearch_filter_terms">'.
                 '<label>Filtern nach Titel:'.
                   '<input type   = "text" '.
                           'id    = "'.$pre.'title" '.
                           'name  = "title" '.
                           'value = "'.($query!=''?$query:'').'" '.
                           'title = "Titel oder beliebiger Teil des Titels eingeben. '.
                                    'Mehrere W&ouml;rter m&ouml;glich. Es werden nur '.
                                    'exakte &Uuml;bereinstimmungen gefunden">'.
                 '</label>'.
               '</div>';
  $group     = '<div class="dossiersearch_filter_terms">'.
                 '<label for="'.$pre.'group_select">nach Autorengruppe:'.
                   '<select id="'.$pre.'group_select">'._dossiersearch_getAuthorGroups().'</select>'.
                 '</label>'.
               '</div>';
  $date      = '<div id="'.$pre.'date" style="clear:both">nach Datum (Dokument, letzte &Auml;nderung):'.
                  '<div id="'.$pre.'date_slider"></div>'.
                  '<div id="'.$pre.'date_value">von: bis:</div>'.
                  '<input id="'.$pre.'date_from" value="0" type="hidden" />'.
                  '<input id="'.$pre.'date_to"   value="0" type="hidden" />'.
               '</div>';
  $taxonomy  = _dossiersearch_getTermDropdown(1).
               _dossiersearch_getTermDropdown(2).
               _dossiersearch_getTermDropdown(3);
  return       '<div id="'.$pre.'box1">'.
                   $fTitle.
                   $taxonomy.
                   $group.
                   $date.
               '</div>'.
               '<div id="'.$pre.'box2">'.
                   $gmap.
               '</div>';
}

/**
 * Gibt ein Dropdown eines Vokabulares aus mit den
 * einzelnen Termen als Optionen
 * @param  integer $vid ID des Vokabulares
 * @return string HTML-Formatiertes Dropdown in einem DIV-Wrapper
 */
function _dossiersearch_getTermDropdown($vid) {
  $results  = db_query('SELECT name FROM vocabulary WHERE vid = %d LIMIT 0,1', $vid);
  $vTitle   = db_fetch_object($results)->name;
  $pre      = 'dossiersearch_filter_';
  return    '<div class="'.$pre.'terms">'.
              '<label>nach&nbsp;<span>'.$vTitle.'</span>:'.
                '<select id="'.$pre.strtolower($vTitle).'_select">'.
                  _dossiersearch_getTermOptions($vid).
                '</select>'.
              '</label>'.
            '</div>';
}



/**
 * Gibt alle OG-Gruppen als HTML-<option>'s zurück
 * @return string '<option value="">...</option><option...'
 */
function _dossiersearch_getAuthorGroups() {
  $allgroups = og_all_groups_options();
  $options = '<option value="0">-- alle --</option>';
  foreach ($allgroups as $gid=>$groupname) {
    $options .= '<option value="'.$gid.'">'.$groupname.'</option>';
  }
  return $options;
}

/**
 * Java-Script Einstellungen des Modules
 *
 * @return array Javascript Settings von Dossiersearch
 */
function _dossiersearch_getJsSettings() {
  $projekt = array();
  $grenzen = explode("\n", variable_get('dossiersearch_project', 0));
  foreach ($grenzen as $latlng) {
    $projekt[] = explode(',', $latlng);
  }
  return array(
      'previews'=>array(
        'images' => array(
            'path'   => url(drupal_get_path('module', 'previews')),
            'filespath' => file_create_url('previews/'),
        ),
      ),
      'dossiersearch'=>array(
        'path'    => url(drupal_get_path('module', 'dossiersearch')),
        'center'  => array(
            'title'     => variable_get('dossiersearch_center_title', 'Basel'),
            'latitude'  => variable_get('dossiersearch_center_latitude', '47.54443828985943'),
            'longitude' => variable_get('dossiersearch_center_longitude', '7.596015930175781'),
        ),
        // Informationen über das Projekt
        'projekt' => array(
            'map' => $projekt,  // Vektorkarte
            'mindate' => _searchtools_oldestDocument(),                     // ältestes Dokument
            'maxdate' => _searchtools_midnight(),      // neustes Dokument
            'range'   => ((_searchtools_midnight()-_searchtools_oldestDocument())/86400), // Anzahl Tage zwischen ältestem und neustem Dokument
        ),
        'gmap'    => array(
            'show'  => variable_get('dossiersearch_showmap'       , false),
            'use'   => variable_get('dossiersearch_filterlocation', false),
        ),
      ),
  );
}


function _dossiersearch_getPreviewsThumbs($node) {
  return _previews_getcontent($node);
}

/**
 * Gibt die Terme als <option>'s eines Vokabulares zurück
 * @param  integer $vid ID des Vokabulares
 * @return string '<option value="">...</option><option...'
 */

function _dossiersearch_getTermOptions($vid) {
  $terms = taxonomy_get_nested_tree($vid);
  $options = array();
  $options[] = '<option value="0">-- alle --</option>';
  foreach ($terms as $t) {
    $options[] = _dossiersearch_getTermOptionsFromParent($t);
    
  }
  return implode('', $options);
}

function _dossiersearch_getTermOptionsFromParent($term) {
  $options = array();
  $options[] = '<option value="'.$term->tid.'" class="dossiersearch_filter_terms_'.$term->depth.'">'.str_repeat('-',3*$term->depth).$term->name.'</option>';
  if (isset($term->children)) {
    foreach ($term->children as $child) {
      $options[] = _dossiersearch_getTermOptionsFromParent($child);
    }
  }
  return implode('',$options);
}


function taxonomy_get_nested_tree($terms = array(), $max_depth = NULL, $parent = 0, $parents_index = array(), $depth = 0) {
  if (is_int($terms)) {
    $terms = taxonomy_get_tree($terms);
  }

  foreach($terms as $term) {
    foreach($term->parents as $term_parent) {
      if ($term_parent == $parent) {
        $return[$term->tid] = $term;
      }
      else {
        $parents_index[$term_parent][$term->tid] = $term;
      }
    }
  }

  foreach($return as &$term) {
    if (isset($parents_index[$term->tid]) && (is_null($max_depth) || $depth < $max_depth)) {
      $term->children = taxonomy_get_nested_tree($parents_index[$term->tid], $max_depth, $term->tid, $parents_index, $depth + 1);
    }
  }

  return $return;
}
