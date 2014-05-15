<?php


/**
 * Nimmt den aktuellen Tag und gibt den den UNIX-Timestamp
 * von 23:59:59 des Tages zurück.
 * @return UNIX-timestamp
 */
function _searchtools_midnight() {
  return mktime(0,0,0,date('n'),date('j')+1,date('Y'));
}

/**
 * Findet das Datum des ältesten Dossiers in der Ablage
 * @return integer timestamp des ältesten Dossiers
 */
function _searchtools_oldestDocument() {
  // Cache oldest Document
  static $oDate = false;
  if ($oDate) return $oDate;

  $sql = "SELECT created FROM {node} WHERE type LIKE 'dossier' ORDER BY created ASC LIMIT 0,1";
  $result = db_query($sql);
  $date1 = db_fetch_object($result)->created;
  $date1 = mktime(0,0,0,date('n',$date1),date('j',$date1),date('Y',$date1));

  $sql = "SELECT field_dokumentendatum_value FROM {content_type_dossier} ORDER BY field_dokumentendatum_value ASC LIMIT 0,1";
  $result = db_query($sql);
  $date2 = strtotime(db_fetch_object($result)->field_dokumentendatum_value);
  $date2 = mktime(0,0,0,date('n',$date2),date('j',$date2),date('Y',$date2));

  $oDate = ($date1<$date2)?$date1:$date2; // write to static cache
  return $oDate;
}


/**
 * Ermittelt das Alter eines Nodes anhand des Timestamps
 * Gibt folgendes zurück: heute, gestern, n Tage, n Wochen, n Monate
 * @param integer $timestamp
 * @return string 
 */
function _searchtools_ago($timestamp) {
  $diff = mktime(23,59,59,date('n'),date('j'),date('Y'))-$timestamp;
  if ($diff <= 60*60*24) {
    return 'heute';
  }
  elseif ($diff <= 60*60*24*2) {
    return 'gestern';
  }
  elseif ($diff <= 60*60*24*7) {
    return floor($diff/60/60/24).' Tage';
  }
  elseif ($diff <= 60*60*24*7*4) {
    $w = floor($diff/60/60/24/7);
    return $w.' Woche'.(($w==1)?'':'n');
  }
  elseif ($diff <= 60*60*24*30*2000) {
    $w = floor($diff/60/60/24/30.3);
    $w = $w<1?1:$w;
    return $w.' Monat'.(($w==1)?'':'e');
  }
}

/**
 * UID -> Username conversion
 * @param integer $uid Eine Drupal User-ID
 * @return string Benutzername
 */
function _searchtools_uid2username($uid) {
  $sql = "SELECT {users}.name FROM ({users}, {node} n) WHERE {users}.uid = '%d'";
  $result = db_query(db_rewrite_sql($sql), $uid);
  return db_fetch_object($result)->name;
}


/**
 * Findet heraus, ob ein Node public ist
 * @param type $nid
 * @return type 
 */
function _searchtools_og_isPublic($nid) {
  $sql = "SELECT og_public FROM {og_access_post} WHERE nid = %d";
  return db_fetch_object(db_query($sql,$nid))->og_public;
}

/**
 * Findet heraus, ob ein Node Koordinaten angehängt hat
 * @param type $nid 
 */
function _searchtools_hasCoordinates($nid) {
  $out = '';
  $sql = "SELECT node.nid FROM node LEFT JOIN location_instance ON (node.vid = location_instance.vid) LEFT JOIN location ON (location_instance.lid = location.lid) WHERE node.nid = ".$nid." AND location.latitude IS NOT NULL";
  if (db_fetch_object(db_query($sql))) {
    $out = '<div class="frontpage-map" title="Dieses Dossier ist mit Koordinaten versehen"></div>';
  }
  return $out;
}


/**
 * Gibt HTML-Code für die Gruppen-Anzeige zurück
 * @param array $groups Array mit Gruppen-Titel zB 'Gruner AG'
 * @return string Kommagetrennte Liste mit Gruppen-Icon und wenn
 * nicht vorhanden mit Gruppenname
 */
function _searchtools_getGroupIcons($groups, $public=false) {
  static $imgs = array();
  $g = array();
  $showLogos = variable_get('frontpage_showOgLogos', true);
  foreach ($groups as $group) {
    $imgpath = path_to_theme().'/custom/icons_og/'.preg_replace('/[^a-z]/msi','',strtolower($group)).'.png';
    if(file_exists($imgpath) && $showLogos){
      if (!isset($imgs[$imgpath])) {
        $imgs[$imgpath] = array('content' => file_get_contents($imgpath),
                                'info'    => getimagesize($imgpath));
      }
      $src = _searchtools_dataUriSupport()?'data:image/jpeg;base64,'.chunk_split(base64_encode($imgs[$imgpath]['content'])):($imgpath);
      $g[] = '<img src="'.$src.'" '.$imgs[$imgpath]['info'][3].' title="'.$group.'" />';
    }
    else { $g[] = $group; } // es existiert kein Bild
  }
  // Für Globale Inhalte noch Globa-Icon voranhängen.
  if ($public) {
    array_unshift($g, _searchtools_getImgPublic());
  }
  return implode(', ', $g);
}



function _searchtools_getImgPublic() {
  static $img = '';
  if ($img != '') return $img;
  $img = theme_image(
          path_to_theme().'/custom/icons_og/global.gif',
          '',
          'Diese Datei ist für alle Plattformbenutzer zugänglich',
          array('width'=>16,'height'=>16),
          false);
  return $img;
}


/**
 * Checks if data URI is supported
 * @return boolean returns true if data-URI is supported
 * false if not supported (IE<8)
 */
function _searchtools_dataUriSupport() {
  return !preg_match('/(?i)msie [1-7]/',$_SERVER['HTTP_USER_AGENT']);
}



function _searchtools_nodetypeIcon($type) {
  // Spezialzeichen löschen; in Kleinbuchstaben umwandeln
  $o = preg_replace('/[^a-z]/','',strtolower($type));

  // Pfad des Bildes
  $imgpath = path_to_theme().'/custom/icons_nodetype/'.$o.'.png';

  // Wenn Bild vorhanden, dann nehmen
  if(file_exists($imgpath)){
  $icon = theme(
          'image',
          $imgpath,
          strtoupper(substr($type,0,1)).substr($type,1,strlen($type)-1),
          strtoupper(substr($type,0,1)).substr($type,1,strlen($type)-1),
          array('width'=>'16'),
          false);
  }

  // anderfalls default-Bild
  else{
  $icon = theme(
          'image',
          path_to_theme().'/custom/icons_nodetype/default.png',
          strtoupper(substr($type,0,1)).substr($type,1,strlen($type)-1),
          strtoupper(substr($type,0,1)).substr($type,1,strlen($type)-1),
          array('width'=>'16'),
          false);
  }

  return $icon;
}
